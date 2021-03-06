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
#ifndef SERIALIZATION_PREPROCESSOR_HPP_INCLUDED
#define SERIALIZATION_PREPROCESSOR_HPP_INCLUDED

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

struct preproc_define
{
	preproc_define() {}
	explicit preproc_define(std::string const &val) : value(val) {}
	preproc_define(std::string const &val, std::vector< std::string > const &args,
	               std::string const &domain, int line, std::string const &loc)
		: value(val), arguments(args), textdomain(domain), linenum(line), location(loc) {}
	std::string value;
	std::vector< std::string > arguments;
	std::string textdomain;
	int linenum;
	std::string location;
	bool operator==(preproc_define const &) const;
	bool operator!=(preproc_define const &v) const { return !operator==(v); }
};

typedef std::map< std::string, preproc_define > preproc_map;

//function to use the WML preprocessor on a file, and returns the resulting
//preprocessed file data. defines is a map of symbols defined.
std::istream *preprocess_file(std::string const &fname,
                              preproc_map *defines = NULL);

#endif
