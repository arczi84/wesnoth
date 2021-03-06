/* $Id$ */
/*
   Copyright (C) 2004 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCROLLAREA_HPP_INCLUDED
#define SCROLLAREA_HPP_INCLUDED

#include "SDL.h"
#include "../sdl_utils.hpp"
#include "scrollbar.hpp"
#include "widget.hpp"

namespace gui {

class scrollarea : public widget
{
public:
	/// Create a zone with automatic handling of scrollbar.
	/// \param d the display object
	/// \param pane the widget where wheel events take place
	scrollarea(CVideo &video);

	virtual void hide(bool value = true);

protected:
	virtual void update_location(SDL_Rect const &rect);
	virtual void handle_event(const SDL_Event& event);
	virtual void process_event();
	virtual void scroll(int pos) = 0;
	virtual void set_inner_location(SDL_Rect const &rect) = 0;

	SDL_Rect inner_location() const;
	unsigned scrollbar_width() const;

	unsigned get_position() const;
	unsigned get_max_position() const;
	void set_position(unsigned pos);
	void adjust_position(unsigned pos);
	void move_position(int dep);
	void set_shown_size(unsigned h);
	void set_full_size(unsigned h);
	void set_scroll_rate(unsigned r);
	bool has_scrollbar() const;

private:
	scrollbar scrollbar_;
	int old_position_;
	bool recursive_, shown_scrollbar_;
	unsigned shown_size_;
	unsigned full_size_;

	void test_scrollbar();
};

}

#endif
