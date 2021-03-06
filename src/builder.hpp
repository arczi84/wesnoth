/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef BUILDER_H_INCLUDED
#define BUILDER_H_INCLUDED

#include "animated.hpp"
#include "image.hpp"
#include "map.hpp"
#include "SDL.h"

#include <map>
#include <set>
#include <string>

class config;

/**
 * The class terrain_builder is constructed from a config object, and a gamemap
 * object. On construction, it parses the configuration and extracts the list
 * of [terrain_graphics] rules. Each terrain_graphics rule attachs one or more
 * images to a specific terrain pattern.
 * It then applies the rules loaded from the configuration to the current map,
 * and calculates the list of images that must be associated to each hex of the
 * map.
 *
 * The get_terrain_at method can then be used to obtain the list of images
 * necessary to draw the terrain on a given tile.
 */
class terrain_builder
{
public:
	/** Used as a parameter for the get_terrain_at function.
	 * ADJACENT_BACKGROUND represent terrains which are to be drawn behind
	 * unit sprites, ADJACENT_FOREGROUND terrains which are to be drawn in
	 * front of them.
	 */
	enum ADJACENT_TERRAIN_TYPE { ADJACENT_BACKGROUND, ADJACENT_FOREGROUND };
	/** A shorthand typedef for a list of animated image locators, the base
	 * data type returned by the get_terrain_at method.
	 */
	typedef std::vector<animated<image::locator> > imagelist;

	/** Constructor for the terrain_builder class.
	 *
	 * @param cfg   The main grame configuration object, where the
	 *              [terrain_graphics] rule reside.
	 * @param level A level (scenario)-specific configuration file,
	 *              containing scenario-specific [terrain_graphics] rules.
	 * @param gmap  A properly-initialized gamemap object representing the
	 *              current terrain map.
	 */
	terrain_builder(const config& cfg, const config &level, const gamemap& gmap);

	/** Returns a vector of strings representing the images to load & blit
	 * together to get the built content for this tile.
	 *
	 * @param loc   The location relative the the terrain map, where we ask
	 *              for the image list
	 * @param tod   The string representing the current time-of day. Will
	 *              be used if some images specify several time-of-day-
	 *              related variants.
	 * @param terrain_type ADJACENT_BACKGROUND or ADJACENT_FOREGROUND,
	 *              depending on wheter we ask for the terrain which is
	 *              before, or after the unit sprite.
	 *
	 * @return      Returns a pointer list of animated images corresponding
	 *              to the parameters, or NULL if there is none.
	 */
	const imagelist *get_terrain_at(const gamemap::location &loc,
			const std::string &tod, ADJACENT_TERRAIN_TYPE terrain_type) const;

	/** Updates the animation at a given tile. returns true if something has
	 * changed, and must be redrawn.
	 *
	 * @param loc   the location to update
	 *
	 * @return      true: this tile must be redrawn.
	 */
	bool update_animation(const gamemap::location &loc);

	/** Performs a "quick-rebuild" of the terrain in a given location. The
	 * "quick-rebuild" is no proper rebuild: it only clears the terrain
	 * cache for a given location, and replaces it with a signle, default
	 * image for this terrain.
	 *
	 * @param loc   the location where to rebuild terrains
	 */
	void rebuild_terrain(const gamemap::location &loc);

	/** Performs a complete rebuild of the list of terrain graphics
	 * attached to a map. Should be called when a terrain is changed in the
	 * map.
	 */
	void rebuild_all();

	/**
	 * An image variant. The in-memory representation of the [variant] WML
	 * tag of the [image] WML tag. When an image only has one variant, the
	 * [variant] tag may be omitted.
	 */
	struct rule_image_variant {
		/** Shorthand constructor for this structure */
		rule_image_variant(const std::string &image_string, const std::string &tod) :
			image_string(image_string), tod(tod) {};

		/** A string representing either the filename for an image, or
		 * a list of images, with an optional timing for each image.
		 * Corresponds to the "name" parameter of the [variant] (or of
		 * the [image]) WML tag.
		 *
		 * The timing string is in the following format (expressed in EBNF)
		 *
		 * <timing_string> ::= <timed_image> ( "," <timed_image> ) +
		 *
		 * <timed_image> ::= <image_name> [ ":" <timing> ]
		 *
		 * Where <image_name> represents the actual filename of an
		 * image, and <timing> the number of milliseconds this image
		 * will last in the animation.
		 */
		std::string image_string;

		/** An animated image locator built according to the image
		 * string. This will be the image locator which will actually
		 * be returned to the user.
		 */
		animated<image::locator> image;
		/**
		 * The time-of-day to which this variant applies. Set to the
		 * empty string, this variant applies to all TODs.
		 */
		std::string tod;
	};

	/**
	 * A map associating a rule_image_variant to a string representing the
	 * time of day.
	 */
	typedef std::map<std::string, rule_image_variant> rule_image_variantlist;

	/**
	 * Each terrain_graphics rule is associated a set of images, which are
	 * applied on the terrain if the rule matches. An image is more than
	 * graphics: it is graphics (with several possible tod-alternatives,)
	 * and a position for these graphics.
	 * The rule_image structure represents one such image.
	 */
	struct rule_image {
		rule_image(int layer, bool global_image=false);
		rule_image(int x, int y, bool global_image=false);

		/** There are 2 kinds of images:
		 * * Flat, horizontal, stacked images, who have a layer
		 * * Vertical images who have an x/y position
		 */
		enum { HORIZONTAL, VERTICAL } position;

		/** The layer of the image, if position is set to HORIZONTAL */
		int layer;
		/** The position of the image base (that is, the point where
		 * the image reaches the floor) if position is set to VERTICAL
		 */
		int basex, basey;

		/** The tile width used when using basex and basey. This is not,
		 * necessarily, the tile width in pixels, this is totally
		 * arbitrary. However, it will be set to 72 for convenience.
		 */
		static const int TILEWIDTH;
		/** The position of unit graphics in a tile. Graphics whose y
		 * position is below this value are considered background for
		 * this tile; graphics whose y position is above this value are
		 * considered foreground. */
		static const int UNITPOS;

		/** Set to true if the image was defined as a child of the
		 * [terrain_graphics] tag, set to false if it was defined as a
		 * child of a [tile] tag */
		bool global_image;

		/** A list of Time-Of-Day-related variants for this image
		 */
		rule_image_variantlist variants;
	};

	/**
	 * A shorthand notation for a vector of rule_images
	 */
	typedef std::vector<rule_image> rule_imagelist;

	/**
	 * The in-memory representation of a [tile] WML rule inside of a
	 * [terrain_graphics] WML rule.
	 */
	struct terrain_constraint
	{
		terrain_constraint() : loc() {};

		terrain_constraint(gamemap::location loc) : loc(loc) {};

		gamemap::location loc;
		std::string terrain_types;
		std::vector<std::string> set_flag;
		std::vector<std::string> no_flag;
		std::vector<std::string> has_flag;

		rule_imagelist images;
	};

	/**
	 * Represents a tile of the game map, with all associated
	 * builder-specific parameters: flags, images attached to this tile,
	 * etc. An array of those tiles is built when terrains are built either
	 * during construction, or upon calling the rebuild_all() method.
	 */
	struct tile
	{
		/** An ordered rule_image list */
		typedef std::multimap<int, const rule_image*> ordered_ri_list;

		/** Contructor for the tile() structure */
		tile();

		/** Adds an image, extracted from an ordered rule_image list,
		 * to the background or foreground image cache.
		 *
		 * @param tod    The current time-of-day, to select between
		 *               images presenting several variants.
		 * @param itor   An iterator pointing to the rule_image where
		 *               to extract the image we wish to add to the
		 *               cache.
		 */
		void add_image_to_cache(const std::string &tod, ordered_ri_list::const_iterator itor) const;

		/** Rebuilds the whole image cache, for a given time-of-day.
		 * Must be called when the time-of-day has changed, to select
		 * the correct images.
		 *
		 * @param tod    The current time-of-day
		 */
		void rebuild_cache(const std::string &tod) const;

		/** Clears all data in this tile, and resets the cache */
		void clear();

		/** The list of flags present in this tile */
		std::set<std::string> flags;

		/** The list of images associated to this tile which have the
		 * "position=horizontal" parameter set, ordered by their layer.
		 */
		ordered_ri_list horizontal_images;
		/** The list of images associated to this tile which have the
		 * "position=vertical" parameter set, ordered by their position
		 * in the y-axis
		 */
		ordered_ri_list vertical_images;

		/** The list of images which are in front of the unit sprites,
		 * attached to this tile. This member is considered a cache: it
		 * is built once, and on-demand.
		 */
		mutable imagelist images_foreground;
		/** The list of images which are behind the unit sprites,
		 * attached to this tile. This member is considered a cache: it
		 * is built once, and on-demand.
		 */
		mutable imagelist images_background;
		/**
		 * The time-of-day to which the image caches correspond.
		 */
		mutable std::string last_tod;

		/**
		 * The 6 adjacent terrains of this tile, and the terrain on
		 * this tile.
		 */
		gamemap::TERRAIN adjacents[7];

	};

private:
	/**
	 * The list of constraints attached to a terrain_graphics WML rule.
	 */
	typedef std::map<gamemap::location, terrain_constraint> constraint_set;

	/**
	 * The in-memory representation of a [terrain_graphics] WML rule.
	 */
	struct building_rule
	{
		/**
		 * The set of [tile] constraints of this rule.
		 */
		constraint_set constraints;

		/**
		 * The location on which this map may match. Set to a valid
		 * gamemap::location if the "x" and "y" parameters of the
		 * [terrain_graphics] rule are set.
		 */
		gamemap::location location_constraints;

		/**
		 * The probability of this rule to match, when all conditions
		 * are met. Defined if the "probability" parameter of the
		 * [terrain_graphics] element is set.
		 */
		int probability;

		/**
		 * The precedence of this rule. Used to order rules differently
		 * that the order in which they appear. Defined if the
		 * "precedence" parameter of the [terrain_graphics] element is
		 * set.
		 */
		int precedence;
	};

	/**
	 * The map of "tile" structures corresponding to the level map.
	 */
	class tilemap
	{
	public:
		/**
		 * Constructs a tilemap of dimensions x * y
		 */
		tilemap(int x, int y) : map_((x+2)*(y+2)), x_(x), y_(y) {}

		/**
		 * Returns a reference to the tile which is at the position
		 * pointed by loc. The location MUST be on the map!
		 *
		 * @param loc    The location of the tile
		 *
		 * @return A reference to the tile at this location.
		 */
		tile &operator[](const gamemap::location &loc);
		/**
		 * a const variant of operator[]
		 */
		const tile &operator[] (const gamemap::location &loc) const;

		/**
		 * Tests if a location is on the map
		 *
		 * @param loc   The location to test
		 *
		 * @return true if loc is on the map, false otherwise.
		 */
		bool on_map(const gamemap::location &loc) const;

		/**
		 * Resets the whole tile map
		 */
		void reset();
	private:
		/** The map */
		std::vector<tile> map_;
		/** The x dimension of the map */
		int x_;
		/** The y dimension of the map */
		int y_;
	};

	/**
	 * A set of building rules. In-memory representation of the whole set
	 * of [terrain_graphics] rules.
	 */
	typedef std::multimap<int, building_rule> building_ruleset;

	/**
	 * Tests for validity of a rule. A rule is considered valid if all its
	 * images are present. This method is used, when building the ruleset,
	 * to only add rules which are valid to the ruleset.
	 *
	 * @param rule   The rule to test for validity
	 *
	 * @return true if the rule is valid, false if it is not.
	 */
	bool rule_valid(const building_rule &rule);

	/**
	 * Starts the animation on a rule.
	 *
	 * @param rule    The rule on which ot start animations
	 *
	 * @return true
	 */
	bool start_animation(building_rule &rule);

	/**
	 * "Rotates" a constraint from a rule. Takes a template constraint from
	 * a template rule, and creates a constraint from this template,
	 * rotated to the given angle.
	 *
	 * On a constraint, the relative position of each rule, and the "base"
	 * of each vertical images, are rotated according to the given angle.
	 *
	 * Template terrain constraints are defined like normal terrain
	 * constraints, except that, flags, and image filenames, may contain
	 * template strings of the form <code>@Rn</code>, n being a number from
	 * 0 to 5. See the rotate_rule method for more info.
	 *
	 * @param constraint A template constraint to rotate
	 * @param angle      An int, from 0 to 5, representing the rotation
	 *                   angle.
	 */
	terrain_constraint rotate(const terrain_constraint &constraint, int angle);

	/**
	 * Replaces, in a given string, a token with its value.
	 *
	 * @param s      The string in which to do the replacement
	 * @param token  The token to substitute
	 * @param replacement The replacement string
	 */
	void replace_token(std::string &s, const std::string &token,
			const std::string& replacement);

	/**
	 * Replaces, in a given rule_variant_image, a token with its value. The
	 * actual substitution is done in the "image_string" parameter of this
	 * rule_variant_image.
	 *
	 * @param variant The rule_variant_image in which to do the replacement
	 * @param token  The token to substitute
	 * @param replacement The replacement string
	 */
	void replace_token(rule_image_variant &variant, const std::string &token,
			const std::string& replacement);

	/**
	 * Replaces, in a given rule_image, a token with its value. The actual
	 * substitution is done in all variants of the given image.
	 *
	 * @param image The rule_image in which to do the replacement
	 * @param token  The token to substitute
	 * @param replacement The replacement string
	 */
	void replace_token(rule_image &image, const std::string &token,
			const std::string& replacement);

	/**
	 * Replaces, in a given rule_imagelist, a token with its value. The
	 * actual substitution is done in all rule_images contained in the
	 * rule_imagelist.
	 *
	 * @param image The rule_imagelist in which to do the replacement
	 * @param token  The token to substitute
	 * @param replacement The replacement string
	 */
	void replace_token(rule_imagelist &, const std::string &token,
			const std::string& replacement);

	/**
	 * Replaces, in a given building_rule, a token with its value. The
	 * actual substitution is done in the rule_imagelists contained in all
	 * constraints of the building_rule, and in the flags (has_flag,
	 * set_flag and no_flag) contained in all constraints of the
	 * building_rule.
	 *
	 * @param image The building_rule  in which to do the replacement
	 * @param token  The token to substitute
	 * @param replacement The replacement string
	 */
	void replace_token(building_rule &s, const std::string &token,
			const std::string& replacement);

	/**
	 * Rotates a template rule to a given angle, and returns the rotated
	 * rule.
	 *
	 * Template rules are defined like normal rules, except that:
	 * * Flags and image filenames may contain template strings of the form
	 * <code>@Rn</code>, n being a number from 0 to 5.
	 * * The rule contains the rotations=r0,r1,r2,r3,r4,r5, with r0 to r5
	 * being strings describing the 6 different positions, typically, n,
	 * ne, se, s, sw, and nw (buy may be anything else.)
	 *
	 * A template rule will generate 6 rules, which are similar to the
	 * template, except that:
	 *
	 * * The map of constraints ( [tile]s ) of this rule will be rotated by
	 * an angle, of 0 to 5 pi / 6
	 *
	 * * On the rule which is rotated to 0rad, the template strings @R0,
	 * @R1, @R2, @R3, @R4, @R5, will be replaced by the corresponding r0,
	 * r1, r2, r3, r4, r5 variables given in the rotations= element.
	 *
	 * * On the rule which is rotated to pi/3 rad, the template strings
	 * @R0, @R1, @R2 etc. will be replaced by the corresponding <strong>r1,
	 * r2, r3, r4, r5, r0</strong> (note the shift in indices).
	 *
	 * * On the rule rotated 2pi/3, those will be replaced by r2, r3, r4,
	 * r5, r0, r1 and so on.
	 *
	 */
	building_rule rotate_rule(const building_rule &rule, int angle, const
			std::vector<std::string>& angle_name);

	/**
	 * Parses a "config" object, which should contains [image] children, and
	 * adds the corresponding parsed rule_images to a rule_imagelist.
	 *
	 * @param images   The rule_imagelist into which to add the parsed images.
	 * @param cfg      The WML configuration object to parse
	 * @param global   Whether those [image]s elements belong to a
	 *                 [terrain_graphics] element, or to a [tile] child.
	 *                 Set to true if those belong to a [terrain_graphics]
	 *                 element.
	 * @param dx       The X coordinate of the constraint those images
	 *                 apply to, relative to the start of the rule. Only
	 *                 meaningful if global is set to false.
	 * @param dy       The Y coordinate of the constraint those images
	 *                 apply to.
	 */
	void add_images_from_config(rule_imagelist &images, const config &cfg, bool global,
			int dx=0, int dy=0);


	/**
	 * Creates a rule constraint object which matches a given list of
	 * terrains, and adds it to the list of constraints of a rule.
	 *
	 * @param constraints The constraint set to which to add the constraint.
	 * @param loc         The location of the constraint
	 * @param type        The list of terrains this constraint will match
	 * @param global_images A configuration object containing [image] tags
	 *                    describing rule-global images.
	 */
	void add_constraints(constraint_set& constraints,
			const gamemap::location &loc, const std::string& type,
			const config& global_images);

	/**
	 * Creates a rule constraint object from a config object and adds it to
	 * the list of constraints of a rule.
	 *
	 * @param constraints The constraint set to which to add the constraint.
	 * @param loc         The location of the constraint
	 * @param cfg         The config object describing this constraint.
	 *                    Usually, a [tile] child of a [terrain_graphics]
	 *                    rule.
	 * @param global_images A configuration object containing [image] tags
	 *                    describing rule-global images.
	 */
	void add_constraints(constraint_set& constraints,
			const gamemap::location &loc, const config &cfg,
			const config& global_images);

	typedef std::multimap<int, gamemap::location> anchormap;

	/**
	 * Parses a map string (the map= element of a [terrain_graphics] rule,
	 * and adds constraints from this map to a building_rule.
	 *
	 * @param mapstring The map string to parse
	 * @param br        The building rule into which to add the extracted
	 *                  constraints
	 * @param anchors   A map where to put "anchors" extracted from the map.
	 * @param global_images A config object representing the images defined
	 *                  as direct children of the [terrain_graphics] rule.
	 */
	void parse_mapstring(const std::string &mapstring, struct building_rule &br,
			     anchormap& anchors, const config& global_images);

	/**
	 * Adds a rule to a ruleset. Checks for validity before adding the rule.
	 *
	 * @param rules   The ruleset into which to add the rules.
	 * @param rule    The rule to add.
	 */
	void add_rule(building_ruleset& rules, building_rule &rule);

	/**
	 * Adds a set of rules to a ruleset, from a template rule which spans 6
	 * rotations (or less if some of the rotated rules are invalid).
	 *
	 * @param rules     The ruleset into which to add the rules.
	 * @param tpl       The template rule
	 * @param rotations A coma-separated string containing the 6 values for
	 *                  replacing rotation template strings (@Rn)
	 */
	void add_rotated_rules(building_ruleset& rules, building_rule& tpl, const std::string &rotations);

	/**
	 * Parses a configuration object containing [terrain_graphics] rules,
	 * and fills the building_rules_ member of the current class according
	 * to those.
	 *
	 * @param config   The configuration object to parse.
	 */
	void parse_config(const config &cfg);

	/**
	 * Checks whether a terrain letter matches a given list of terrain letters
	 *
	 * @param letter   The letter to check
	 * @param terrains The terrain list agains which to check the terrain
	 *                 letter. May contain the metacharacter '*' meaning
	 *                 "all terrains" or the character "!" meaning "all
	 *                 terrains except those present in the list."
	 *
	 * @return returns true if "letter" matches the list, false if it does not.
	 */
	bool terrain_matches(gamemap::TERRAIN letter, const std::string &terrains);

	/**
	 * Checks whether a rule matches a given location in the map.
	 *
	 * @param rule      The rule to check.
	 * @param loc       The location in the map where we want to check
	 *                  whether the rule matches.
	 * @param rule_index The index of the rule, relative to the start of
	 *                  the rule list. Rule indices are used for seeding
	 *                  the pseudo-random-number generator used for
	 *                  probability calculations.
	 * @param check_loc If this parameter is set to false, the "location"
	 *                  of the rule (ie, wheter all constraints's terrain
	 *                  types do actually match) will be assumed to be
	 *                  already checked, only flags and probability will be
	 *                  checked.
	 */
	bool rule_matches(const building_rule &rule, const gamemap::location &loc,
			int rule_index, bool check_loc);

	/**
	 * Applies a rule at a given location: applies the result of a matching
	 * rule at a given location: attachs the images corresponding to the
	 * rule, and sets the flags corresponding to the rule.
	 *
	 * @param rule      The rule to apply
	 * @param loc       The location to which to apply the rule.
	 */
	void apply_rule(const building_rule &rule, const gamemap::location &loc);

	/**
	 * Returns the number of constraints adjacent to a given constraint in
	 * a rule. For example, of the rule map is something like that
	 *  1   .
	 *    2
	 *  .   3
	 * The constraint corresponding to the "1", and the one corresponding
	 * to the "3", will each have 1 adjacent contraint, whereas the
	 * constraint corresponding to the "2" will have 2.
	 *
	 * @param rule     The rule this applies to
	 * @param loc      The location of the constraint to check for adjacent
	 *                 constraints
	 *
	 * @return the number of constraints adjacent to the location loc
	 */
	int get_constraint_adjacents(const building_rule& rule, const gamemap::location& loc);

	/**
	 * Returns the "size" of a constraint, that is, the number of tiles, in
	 * the current map, on which this constraint may possibly match. This
	 * is used to exclude positions where the rule has no change to match
	 * from being checked for.
	 *
	 * The terrain_by_type_border_ and terrain_by_type_ arrays must be
	 * initialized before this method is called.
	 *
	 * @param rule       The rule to check for
	 * @param constraint The constraint to check for
	 * @param border     (out) Set to true if this constraint applies to a non-isolated tile
	 *
	 * @return The number of tiles in the current map, on which the current
	 *         constraint may match. INT_MAX means "all of them" or "unable
	 *         to determine"
	 */
	int get_constraint_size(const building_rule& rule, const terrain_constraint& constraint,
			bool& border);

	/**
	 * Calculates the list of terrains, and fills the tile_map_ member,
	 * from the gamemap and the building_rules_.
	 */
	void build_terrains();

	/**
	 * A reference to the gamemap class used in the current level.
	 */
	const gamemap& map_;
	/**
	 * The tile_map_ for the current level, which is filled by the
	 * build_terrains_ method to contain "tiles" representing images
	 * attached to each tile.
	 */
	tilemap tile_map_;

	/**
	 * Shorthand typedef for a map associating a list of locations to a terrain type.
	 */
	typedef std::map<unsigned char, std::vector<gamemap::location> > terrain_by_type_map;

	/**
	 * A map representing all locations whose terrain is of a given type.
	 */
	terrain_by_type_map terrain_by_type_;
	/**
	 * A map representing all locations whose terrain is adjacent to a
	 * terrain of a given type.
	 */
	terrain_by_type_map terrain_by_type_border_;

	building_ruleset building_rules_;

};

#endif
