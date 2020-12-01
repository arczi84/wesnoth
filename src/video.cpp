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

#include <stdio.h>
#include <iostream>
#include <vector>

#include "font.hpp"
#include "image.hpp"
#include "log.hpp"
#include "video.hpp"

#define LOG_DP LOG_STREAM(info, display)
#define ERR_DP LOG_STREAM(err, display)

#define TEST_VIDEO_ON 0

#include <clib/exec_protos.h>

#define GAME_SCREEN_WIDTH         (960)
#define GAME_SCREEN_HEIGHT        (540)
#define GAME_SCREEN_BITSPERPIXEL  (16)
#define GAME_SCREEN_BYTESPERPIXEL (2)
#define GAME_memsize              (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT * GAME_SCREEN_BYTESPERPIXEL)

#define SAGA_VIDEO_PLANEPTR       0xDFF1EC
#define SAGA_VIDEO_MODE           0xDFF1F4
#define SAGA_VIDEO_YBEAM_POS      0xDFF204


ULONG  x_MemSize = ((GAME_memsize *  4) + 32);
ULONG *x_MemAddr = NULL;
ULONG *x_FBAddr1 = NULL;
ULONG *x_FBAddr2 = NULL;
ULONG *x_FBAddr3 = NULL;
ULONG *x_Pixels  = NULL;


void InitBuffers(void)
{
	/* Allocate Drawing Buffers (Triple-Buffer) */
	x_MemAddr = (ULONG*)AllocMem(x_MemSize,	 MEMF_LOCAL| MEMF_FAST|MEMF_CLEAR);

	// Fatal Exit on error
	if(x_MemAddr == NULL) {
		printf("Failed to allocmem for screens\n");
		exit(1);
	}

	// Aligned Drawing Buffers 1,2,3 */
	x_FBAddr1 = (ULONG*)((((ULONG)x_MemAddr) + 31)& ~31);
	x_FBAddr2 = x_FBAddr1 + GAME_memsize;
	x_FBAddr3 = x_FBAddr2 + GAME_memsize;
}

short freed = 0;

void FreeBuffers(void)
{
	if(x_MemAddr != NULL)
	{
		if (freed == 0)
			FreeMem(x_MemAddr, x_MemSize);
	freed = 1;
	}
}

extern "C"
{

#include <dos/dos.h>
#include <exec/exec.h>

extern struct ExecBase *SysBase;
short ac68080 = 1;
bool checked = false;

void check_sys()
{
	if (!checked) {
		if(SysBase->AttnFlags & (1<<10)) {
				ac68080 = 1;
				printf("Vampire acceleretion detected.\n");
			}
		checked = true;
	}
}

}

#if (TEST_VIDEO_ON==1)

#include <stdlib.h>

//test program takes three args - x-res y-res colour-depth
int main( int argc, char** argv )
{
	if( argc != 4 ) {
		printf( "usage: %s x-res y-res bitperpixel\n", argv[0] );
		return 1;
	}
	SDL_Init(SDL_INIT_VIDEO);
	CVideo video;

	printf( "args: %s, %s, %s\n", argv[1], argv[2], argv[3] );

	printf( "(%d,%d,%d)\n", strtoul(argv[1],0,10), strtoul(argv[2],0,10),
	                        strtoul(argv[3],0,10) );

	if( video.setMode( strtoul(argv[1],0,10), strtoul(argv[2],0,10),
	                        strtoul(argv[3],0,10), FULL_SCREEN ) ) {
		printf( "video mode possible\n" );
	} else  printf( "video mode NOT possible\n" );
	printf( "%d, %d, %d\n", video.getx(), video.gety(),
	                        video.getBitsPerPixel() );

	for( int s = 0; s < 50; s++ ) {
		video.lock();
		for( int i = 0; i < video.getx(); i++ )
				video.setPixel( i, 90, 255, 0, 0 );
		if( s%10==0)
			printf( "frame %d\n", s );
		video.unlock();
		video.update( 0, 90, video.getx(), 1 );
	}
	return 0;
}

#endif

namespace {
bool fullScreen = false;

unsigned int get_flags(unsigned int flags)
{
	//SDL under Windows doesn't seem to like hardware surfaces for
	//some reason.
#if !(defined(_WIN32) || defined(__APPLE__))
		flags |= SDL_HWSURFACE/*|SDL_DOUBLEBUF*/;
#endif
	if((flags&SDL_FULLSCREEN) == 0)
		flags |= SDL_RESIZABLE;

	return flags;
}

std::vector<SDL_Rect> update_rects;
bool update_all = false;

bool rect_contains(const SDL_Rect& a, const SDL_Rect& b) {
	return a.x <= b.x && a.y <= b.y && a.x+a.w >= b.x+b.w && a.y+a.h >= b.y+b.h;
}

void clear_updates()
{
	update_all = false;
	update_rects.clear();
}

surface frameBuffer = NULL;

}

bool non_interactive()
{
	return SDL_GetVideoSurface() == NULL;
}

surface display_format_alpha(surface surf)
{
	if(SDL_GetVideoSurface() != NULL)
		return SDL_DisplayFormatAlpha(surf);
	else if(frameBuffer != NULL)
		return SDL_ConvertSurface(surf,frameBuffer->format,0);
	else
		return NULL;
}

surface get_video_surface()
{
	return frameBuffer;
}

SDL_Rect screen_area()
{
	const SDL_Rect res = {0,0,frameBuffer->w,frameBuffer->h};
	return res;
}

void update_rect(size_t x, size_t y, size_t w, size_t h)
{
	const SDL_Rect rect = {x,y,w,h};
	update_rect(rect);
}

void update_rect(const SDL_Rect& rect_value)
{
	if(update_all)
		return;

	SDL_Rect rect = rect_value;

	surface const fb = SDL_GetVideoSurface();
	if(fb != NULL) {
		if(rect.x < 0) {
			if(rect.x*-1 >= int(rect.w))
				return;

			rect.w += rect.x;
			rect.x = 0;
		}

		if(rect.y < 0) {
			if(rect.y*-1 >= int(rect.h))
				return;

			rect.h += rect.y;
			rect.y = 0;
		}

		if(rect.x + rect.w > fb->w) {
			rect.w = fb->w - rect.x;
		}

		if(rect.y + rect.h > fb->h) {
			rect.h = fb->h - rect.y;
		}

		if(rect.x >= fb->w) {
			return;
		}

		if(rect.y >= fb->h) {
			return;
		}
	}

	for(std::vector<SDL_Rect>::iterator i = update_rects.begin();
	    i != update_rects.end(); ++i) {
		if(rect_contains(*i,rect)) {
			return;
		}

		if(rect_contains(rect,*i)) {
			*i = rect;
			for(++i; i != update_rects.end(); ++i) {
				if(rect_contains(rect,*i)) {
					i->w = 0;
				}
			}

			return;
		}
	}

	update_rects.push_back(rect);
}

void update_whole_screen()
{
	update_all = true;
}
CVideo::CVideo() : bpp(0), fake_screen(false), help_string_(0), updatesLocked_(0)
{
	const int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

	if(res < 0) {
		ERR_DP << "Could not initialize SDL: " << SDL_GetError() << "\n";
		throw CVideo::error();
	}
}

CVideo::CVideo( int x, int y, int bits_per_pixel, int flags)
		 : bpp(0), fake_screen(false), help_string_(0), updatesLocked_(0)
{
	const int res = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	if(res < 0) {
		ERR_DP << "Could not initialize SDL: " << SDL_GetError() << "\n";
		throw CVideo::error();
	}

	const int mode_res = setMode( x, y, bits_per_pixel, flags );
	if (mode_res == 0) {
		ERR_DP << "Could not set Video Mode\n";
		throw CVideo::error();
	}

	check_sys();

	if (ac68080)
		InitBuffers();

}

CVideo::~CVideo()
{
	LOG_DP << "calling SDL_Quit()\n";
	if (ac68080)
		FreeBuffers();

	SDL_Quit();
	LOG_DP << "called SDL_Quit()\n";
}

void CVideo::blit_surface(int x, int y, surface surf, SDL_Rect* srcrect, SDL_Rect* clip_rect)
{
	const surface target(getSurface());
	SDL_Rect dst = {x,y,0,0};

	if(clip_rect != NULL) {
		const clip_rect_setter clip_setter(target,*clip_rect);
		SDL_BlitSurface(surf,srcrect,target,&dst);
	} else {
		SDL_BlitSurface(surf,srcrect,target,&dst);
	}
}

void CVideo::make_fake()
{
	fake_screen = true;
	frameBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,1,1,24,0xFF0000,0xFF00,0xFF,0);
	image::set_pixel_format(frameBuffer->format);
}

int CVideo::modePossible( int x, int y, int bits_per_pixel, int flags )
{
	return SDL_VideoModeOK( x, y, bits_per_pixel, get_flags(flags) );
}

int CVideo::setMode( int x, int y, int bits_per_pixel, int flags )
{
	update_rects.clear();
	mode_changed_ = true;

	flags = get_flags(flags);
	const int res = SDL_VideoModeOK( x, y, bits_per_pixel, flags );

	if( res == 0 )
		return 0;

	fullScreen = (flags & FULL_SCREEN) != 0;
	frameBuffer = SDL_SetVideoMode( x, y, bits_per_pixel, flags );

	if( frameBuffer != NULL ) {
		image::set_pixel_format(frameBuffer->format);

		return bits_per_pixel;
	} else	return 0;
}

bool CVideo::modeChanged()
{
	bool ret = mode_changed_;
	mode_changed_ = false;
	return ret;
}

int CVideo::setGamma(float gamma)
{
	SDL_SetGamma(gamma, gamma, gamma);

	return 0;
}

int CVideo::getx() const
{
	return frameBuffer->w;
}

int CVideo::gety() const
{
	return frameBuffer->h;
}

int CVideo::getBitsPerPixel()
{
	return frameBuffer->format->BitsPerPixel;
}

int CVideo::getBytesPerPixel()
{
	return frameBuffer->format->BytesPerPixel;
}

int CVideo::getRedMask()
{
	return frameBuffer->format->Rmask;
}

int CVideo::getGreenMask()
{
	return frameBuffer->format->Gmask;
}

int CVideo::getBlueMask()
{
	return frameBuffer->format->Bmask;
}


void flipSAGA1()
{
	x_Pixels  = (ULONG *)frameBuffer->pixels;
	/* Swap Drawing Buffers */
	x_FBAddr3 = x_FBAddr2;
	x_FBAddr2 = x_FBAddr1;
	x_FBAddr1 = x_Pixels ;

	/* Make Drawing Buffer visible */
	*(volatile ULONG*)SAGA_VIDEO_PLANEPTR = (ULONG)x_Pixels;

	 frameBuffer->pixels = (void *)x_FBAddr3;
}

#include <clib/graphics_protos.h> //WaitTOF

void flipSAGA()
{
    /* Get current buffer */
    x_Pixels = (ULONG *)frameBuffer->pixels;

    /* Make Drawing Buffer visible */
    *(volatile ULONG*)SAGA_VIDEO_PLANEPTR = (ULONG)x_Pixels;

    /* Rotate the Buffers */
    ULONG *temp = x_FBAddr1;
    x_FBAddr1 = x_FBAddr2;
    x_FBAddr2 = x_FBAddr3;
    x_FBAddr3 = temp;

    frameBuffer->pixels = (void *)x_FBAddr1;
    //WaitTOF();
}

void CVideo::flip()
{
	if(fake_screen)
		return;

	if(update_all) {
		if (ac68080)
			flipSAGA();
		else
			::SDL_Flip(frameBuffer);

	} else if(update_rects.empty() == false) {
		size_t sum = 0;
		for(size_t n = 0; n != update_rects.size(); ++n) {
			sum += update_rects[n].w*update_rects[n].h;
		}

		const size_t redraw_whole_screen_threshold = 80;
		if(sum > ((getx()*gety())*redraw_whole_screen_threshold)/100) {
			if (ac68080)
				flipSAGA();
			else
				::SDL_Flip(frameBuffer);

		} else {
			//if (ac68080)
			//	flipSAGA();

			SDL_UpdateRects(frameBuffer,update_rects.size(),&update_rects[0]);
		}
	}

	clear_updates();
}

void CVideo::lock_updates(bool value)
{
	if(value == true)
		++updatesLocked_;
	else
		--updatesLocked_;
}

bool CVideo::update_locked() const
{
	return updatesLocked_ > 0;
}

void CVideo::lock()
{
	if( SDL_MUSTLOCK(frameBuffer) )
		SDL_LockSurface( frameBuffer );
}

void CVideo::unlock()
{
	if( SDL_MUSTLOCK(frameBuffer) )
		SDL_UnlockSurface( frameBuffer );
}

int CVideo::mustLock()
{
	return SDL_MUSTLOCK(frameBuffer);
}

surface CVideo::getSurface( void )
{
	return frameBuffer;
}

bool CVideo::isFullScreen() const { return fullScreen; }

void CVideo::setBpp( int bpp )
{
	this->bpp = bpp;
}

int CVideo::getBpp( void )
{
	return bpp;
}

int CVideo::set_help_string(const std::string& str)
{
	font::remove_floating_label(help_string_);

	const SDL_Color colour = {0x0,0x00,0x00,0x77};

	int size = font::SIZE_LARGE;

	while(size > 0) {
		if(font::line_width(str, size) > getx()) {
			size--;
		} else {
			break;
		}
	}

	help_string_ = font::add_floating_label(str,size,font::NORMAL_COLOUR,getx()/2,gety(),0.0,0.0,-1,screen_area(),font::CENTER_ALIGN,&colour,5);
	const SDL_Rect& rect = font::get_floating_label_rect(help_string_);
	font::move_floating_label(help_string_,0.0,-double(rect.h));
	return help_string_;
}

void CVideo::clear_help_string(int handle)
{
	if(handle == help_string_) {
		font::remove_floating_label(handle);
		help_string_ = 0;
	}
}

void CVideo::clear_all_help_strings()
{
	clear_help_string(help_string_);
}
