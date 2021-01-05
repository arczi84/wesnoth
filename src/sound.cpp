/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "sound.hpp"
#include "wesconfig.h"


#include <string>
#include <iostream>
#include <map>

extern "C"
{
#ifdef __AMIGA__
#include <hardware/custom.h>
extern struct Custom custom;

extern struct WaveData *loadMusic(const char *filename);
extern void playMusic(struct WaveData *waveData);
extern void playSound(struct WaveData *waveData);
extern void FreeMusic(struct WaveData *waveData);
extern short ac68080;
struct WaveData *music;
struct WaveData *sounds;

#endif
}

#define LOG_AUDIO std::cout //LOG_STREAM(info, audio)
#define ERR_AUDIO std::cout //LOG_STREAM(err, audio)

namespace {

std::string current_music = "";
}

namespace sound {
manager::manager()
{
}
manager::~manager()
{
	close_sound();
}

bool init_sound() {
//sounds don't sound good on Windows unless the buffer size is 4k,
//but this seems to cause crashes on other systems...
if (ac68080)
  {
	LOG_AUDIO << "Audio initialized.\n";

	music = loadMusic(current_music.c_str());
  }
	return true;

}

void close_sound() {
	if (ac68080) {
		FreeMusic(music);
		FreeMusic(sounds);
	}

}

void stop_music() {
	if (ac68080) {
		custom.dmacon = 0x0001;
	}
}

extern "C"
{
extern int channel;
}

void stop_sound() {
	if (ac68080) {
		if (channel == 1) {
			custom.dmacon = 0x0002;
			channel = 2;
		}
		else if (channel == 2) {
			custom.dmacon = 0x0004;
			channel = 3; }
		else {
			custom.dmacon = 0x0008;
			channel = 1;
		}
	}

}

void play_music(std::string file)
{
	if (ac68080) {

		if(preferences::music_on()) {

				const std::string& filename = get_binary_file_location("music",file);

				if(filename.empty()) {
					return;
				}
				ERR_AUDIO << "Loading music file '" << filename << "\n";

				if(music)
					stop_music();

				music = loadMusic(filename.c_str());

				if(music == NULL) {
					ERR_AUDIO << "Could not load music file '" << filename << "\n";
					return;
				}

				playMusic(music);

		}
	}

}

void play_sound(const std::string& file)
{
	if (ac68080)
	{
		if(preferences::sound_on()) {

				const std::string& filename = get_binary_file_location("sounds", file);
				if (!filename.empty()) {

					//ERR_AUDIO << "Loading sound file '" << filename << "\n";
					sounds = loadMusic(filename.c_str());
				}

				if (sounds == NULL) {
					ERR_AUDIO << "Could not load sound file '" << filename << "\n";
					return;
				}

			//play on the first available channel
			playSound(sounds);
		}
	}
}

void set_music_volume(int vol)
{
	if (ac68080) {
		/*To Do*/
	}

}

void set_sound_volume(int vol)
{
	if (ac68080) {
		/*To Do*/
	}

}

}
