/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "log.hpp"
#include "serialization/binary_wml.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#define ERR_CF LOG_STREAM(err, config)

//data compression. Compression is designed for network traffic.
//assumptions compression is based on:
// - most space is taken up by element names and attribute names
// - there are relatively few element names and attribute names that are repeated many times
//
//how it works: there are some control characters:
// 'compress_open_element': signals that the next word found is an element.
// any words found that are not after this are assumed to be attributes
// 'compress_close_element': signals to close the current element
// 'compress_schema_item': signals that following is a nul-delimited string, which should
//                         be added as a word in the schema
// 'compress_literal_word': signals that following is a word stored as a nul-delimited string
//    (an attribute name, unless it was preceeded by 'compress_open_element')
//
// all other characters are mapped to words. When an item is inserted into the schema,
// it is mapped to the first available character. Any attribute found is always followed
// by a nul-delimited string which is the value for the attribute.
//
// the schema objects are designed to be persisted. That is, in a network game, both peers
// can store their schema objects, and so rather than sending schema data each time, the peers
// use and build their schemas as the game progresses, adding a new word to the schema anytime
// it is required.

static const unsigned int
	compress_open_element = 0, compress_close_element = 1,
	compress_schema_item = 2, compress_literal_word = 3,
	compress_first_word = 4, compress_end_words = 256;
static const size_t compress_max_words = compress_end_words - compress_first_word;
static const size_t max_schema_item_length = 20;
static const int max_recursion_levels = 1000;

static void compress_output_literal_word(std::ostream &out, std::string const &word)
{
	out.write(word.c_str(), word.length() + 1);
}

static compression_schema::word_char_map::const_iterator
add_word_to_schema(std::string const &word, compression_schema &schema)
{
	if (word.size() > max_schema_item_length)
		throw config::error("Schema item is too long");

	unsigned int c = compress_first_word + schema.word_to_char.size();

	schema.char_to_word.insert(std::make_pair(c, word));
	return schema.word_to_char.insert(std::make_pair(word, c)).first;
}

static compression_schema::word_char_map::const_iterator
get_word_in_schema(std::string const &word, compression_schema &schema, std::ostream &out)
{
	if (word.size() > max_schema_item_length)
		return schema.word_to_char.end();

	//see if this word is already in the schema
	const compression_schema::word_char_map::const_iterator w = schema.word_to_char.find(word);
	if (w != schema.word_to_char.end()) {
		//in the schema. Return it
		return w;
	} else if (schema.word_to_char.size() < compress_max_words) {
		//we can add the word to the schema

		//we insert the code to add a schema item, followed by the zero-delimited word
		out.put(compress_schema_item);
		compress_output_literal_word(out, word);

		return add_word_to_schema(word, schema);
	} else {
		//it's not there, and there's no room to add it
		return schema.word_to_char.end();
	}
}

static void compress_emit_word(std::ostream &out, std::string const &word, compression_schema &schema)
{
	//get the word in the schema
	const compression_schema::word_char_map::const_iterator w = get_word_in_schema(word, schema, out);
	if (w != schema.word_to_char.end()) {
		//the word is in the schema, all we have to do is output the compression code for it.
		out.put(w->second);
	} else {
		//the word is not in the schema. Output it as a literal word
		out.put(compress_literal_word);
		compress_output_literal_word(out, word);
	}
}

static std::string compress_read_literal_word(std::istream &in)
{
	std::string buffer;
	std::getline(in, buffer, '\0');
	if (!in.good())
		throw config::error("Unexpected end of data in compressed config read");
	return buffer;
}

static void write_compressed_internal(std::ostream &out, config const &cfg, compression_schema &schema, int level)
{
	if (level > max_recursion_levels)
		throw config::error("Too many recursion levels in compressed config write");

	for (string_map::const_iterator i = cfg.values.begin(), i_end = cfg.values.end(); i != i_end; ++i) {
		if (i->second.empty() == false) {
			//output the name, using compression
			compress_emit_word(out, i->first, schema);

			//output the value, with no compression
			compress_output_literal_word(out, i->second.to_serialized());
		}
	}

	for (config::all_children_iterator j = cfg.ordered_begin(), j_end = cfg.ordered_end(); j != j_end; ++j) {
		std::pair< std::string const *, config const * > const &item = *j;
		std::string const &name = *item.first;
		config const &cfg2 = *item.second;

		out.put(compress_open_element);
		compress_emit_word(out, name, schema);
		write_compressed_internal(out, cfg2, schema, level + 1);
		out.put(compress_close_element);
	}
}

void write_compressed(std::ostream &out, config const &cfg, compression_schema &schema)
{
	write_compressed_internal(out, cfg, schema, 0);
}

static void read_compressed_internal(config &cfg, std::istream &in, compression_schema &schema, int level)
{
	if (level >= max_recursion_levels)
		throw config::error("Too many recursion levels in compressed config read");

	bool in_open_element = false;
	for(;;) {
		unsigned char const c = in.get();
		if (!in.good())
			return;
		switch (c) {
		case compress_open_element:
			in_open_element = true;
			break;
		case compress_close_element:
			return;
		case compress_schema_item:
			add_word_to_schema(compress_read_literal_word(in), schema);
			break;

		default: {
			std::string word;
			if (c == compress_literal_word) {
				word = compress_read_literal_word(in);
			} else {
				unsigned int code = c;

				const compression_schema::char_word_map::const_iterator itor
					= schema.char_to_word.find(code);
				if (itor == schema.char_to_word.end()) {
					ERR_CF << "illegal word code: " << code << "\n";
					throw config::error("Illegal character in compression input");
				}

				word = itor->second;
			}

			if (in_open_element) {
				in_open_element = false;
				config &cfg2 = cfg.add_child(word);
				read_compressed_internal(cfg2, in, schema, level + 1);
			} else {
				//we have a name/value pair, the value is always a literal string
				std::string value = compress_read_literal_word(in);
				t_string t_value = t_string::from_serialized(value);
				cfg.values.insert(std::make_pair(word, t_value));
			}
		}

		} //end switch
	}
}

void read_compressed(config &cfg, std::istream &in, compression_schema &schema)
{
	cfg.clear();
	read_compressed_internal(cfg, in, schema, 0);
}

void write_compressed(std::ostream &out, config const &cfg) {
	compression_schema schema;
	write_compressed(out, cfg, schema);
}

void read_compressed(config &cfg, std::istream &in) {
	compression_schema schema;
	read_compressed(cfg, in, schema);
}
