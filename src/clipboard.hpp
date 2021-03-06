/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CLIPBOARD_HPP_INCLUDED
#define CLIPBOARD_HPP_INCLUDED

#include <string>
#include "SDL.h"
#include "serialization/string_utils.hpp"
void copy_ucs2_to_clipboard(const ucs2_string& text);
ucs2_string copy_ucs2_from_clipboard();
void copy_to_clipboard(const std::string& text);
std::string copy_from_clipboard();

#if defined(_X11) && !defined(__APPLE__)
void handle_system_event(const SDL_Event& ev);
#endif

#endif
