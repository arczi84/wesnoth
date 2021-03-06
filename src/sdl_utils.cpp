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

#include "config.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wassert.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>

extern "C"{
#include <sys/time.h>

struct timeval tval_before, tval_after, tval_result;
}

#define TAKETIME

#define nullptr NULL

#define ERR_DP LOG_STREAM(err, display)

SDL_Color int_to_color(const Uint32 rgb)
{
	SDL_Color result;
	result.r = (0x00FF0000 & rgb )>> 16;
	result.g = (0x0000FF00 & rgb) >> 8;
	result.b = (0x000000FF & rgb);
	result.unused = 0;
	return result;
}

SDLKey sdl_keysym_from_name(std::string const &keyname)
{
	static bool initialized = false;
	typedef std::map<std::string const, SDLKey> keysym_map_t;
	static keysym_map_t keysym_map;

	if (!initialized) {
		for(SDLKey i = SDLK_FIRST; i < SDLK_LAST; i = SDLKey(int(i) + 1)) {
			std::string name = SDL_GetKeyName(i);
			if (!name.empty())
				keysym_map[name] = i;
		}
		initialized = true;
	}

	keysym_map_t::const_iterator it = keysym_map.find(keyname);
	if (it != keysym_map.end())
		return it->second;
	else
		return SDLK_UNKNOWN;
}

bool point_in_rect(int x, int y, const SDL_Rect& rect)
{
	return x >= rect.x && y >= rect.y && x < rect.x + rect.w && y < rect.y + rect.h;
}

bool rects_overlap(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
	return (rect1.x < rect2.x+rect2.w && rect2.x < rect1.x+rect1.w &&
			rect1.y < rect2.y+rect2.h && rect2.y < rect1.y+rect1.h);
}

SDL_Rect intersect_rects(SDL_Rect const &rect1, SDL_Rect const &rect2)
{
	SDL_Rect res;
	res.x = maximum<int>(rect1.x, rect2.x);
	res.y = maximum<int>(rect1.y, rect2.y);
	res.w = maximum<int>(minimum<int>(rect1.x + rect1.w, rect2.x + rect2.w) - res.x, 0);
	res.h = maximum<int>(minimum<int>(rect1.y + rect1.h, rect2.y + rect2.h) - res.y, 0);
	return res;
}

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}

namespace {
	SDL_PixelFormat& get_neutral_pixel_format()
	{
		static bool first_time = true;
		static SDL_PixelFormat format;

		if(first_time) {
			first_time = false;
			surface surf(SDL_CreateRGBSurface(SDL_SWSURFACE,1,1,32,0xFF0000,0xFF00,0xFF,0xFF000000));
			format = *surf->format;
			format.palette = NULL;
		}

		return format;
	}

}

surface make_neutral_surface(surface const &surf)
{
	if(surf == NULL) {
		std::cerr << "null neutral surface...\n";
		return NULL;
	}

	surface const result = SDL_ConvertSurface(surf,&get_neutral_pixel_format(),SDL_SWSURFACE);
	if(result != NULL) {
		SDL_SetAlpha(result,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
	}

	return result;
}


surface create_optimized_surface(surface const &surf)
{
	if(surf == NULL)
		return NULL;

	surface const result = display_format_alpha(surf);
	if(result == surf) {
		std::cerr << "resulting surface is the same as the source!!!\n";
	} else if(result == NULL) {
		return surf;
	}

	SDL_SetAlpha(result,SDL_SRCALPHA|SDL_RLEACCEL,SDL_ALPHA_OPAQUE);

	return result;
}

surface scale_surface(surface const &surf, int w, int h)
{
	if(surf == NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}

	surface dst(SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));
	surface src(make_neutral_surface(surf));

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const fixed_t xratio = fxpdiv(surf->w,w);
	const fixed_t yratio = fxpdiv(surf->h,h);

	{
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* const dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		fixed_t ysrc = ftofxp(0.0);
		for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
			fixed_t xsrc = ftofxp(0.0);
			for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
				const int xsrcint = fxptoi(xsrc);
				const int ysrcint = fxptoi(ysrc);

				dst_pixels[ydst*dst->w + xdst] = src_pixels[ysrcint*src->w + xsrcint];
			}
		}
	}

	return create_optimized_surface(dst);
}

surface scale_surface_blended(surface const &surf, int w, int h)
{
	if(surf== NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}

	surface dst(SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));
	surface src(make_neutral_surface(surf));

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const double xratio = static_cast<double>(surf->w)/
			              static_cast<double>(w);
	const double yratio = static_cast<double>(surf->h)/
			              static_cast<double>(h);

	{
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* const dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		double ysrc = 0.0;
		for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
			double xsrc = 0.0;
			for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
				double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;

				double summation = 0.0;

				//we now have a rectangle, (xsrc,ysrc,xratio,yratio) which we
				//want to derive the pixel from
				for(double xloc = xsrc; xloc < xsrc+xratio; xloc += 1.0) {
					const double xsize = minimum<double>(std::floor(xloc+1.0)-xloc,xsrc+xratio-xloc);
					for(double yloc = ysrc; yloc < ysrc+yratio; yloc += 1.0) {
						const int xsrcint = maximum<int>(0,minimum<int>(src->w-1,static_cast<int>(xsrc)));
						const int ysrcint = maximum<int>(0,minimum<int>(src->h-1,static_cast<int>(ysrc)));

						const double ysize = minimum<double>(std::floor(yloc+1.0)-yloc,ysrc+yratio-yloc);

						Uint8 r,g,b,a;

						SDL_GetRGBA(src_pixels[ysrcint*src->w + xsrcint],src->format,&r,&g,&b,&a);
						const double value = xsize*ysize*double(a)/255.0;
						summation += value;

						red += r*value;
						green += g*value;
						blue += b*value;
						alpha += a*value;
					}
				}

				if(summation == 0.0)
					summation = 1.0;

				red /= summation;
				green /= summation;
				blue /= summation;
				alpha /= summation;

				dst_pixels[ydst*dst->w + xdst] = SDL_MapRGBA(dst->format,Uint8(red),Uint8(green),Uint8(blue),Uint8(alpha));
			}
		}
	}

	return create_optimized_surface(dst);
}

surface adjust_surface_colour(const surface &surf, int red, int green, int blue)
{
	if(surf == nullptr)
		return nullptr;

	if((red == 0 && green == 0 && blue == 0)) {
		surface temp = surf; // TODO: remove temp surface
		return temp;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
	}

	{
		surface_lock lock(nsurf);
		uint32_t* beg = lock.pixels();
		uint32_t* end = beg + nsurf->w*surf->h;

		while(beg != end) {
			uint8_t alpha = (*beg) >> 24;

			if(alpha) {
				uint8_t r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg) >> 0;

				r = std::max<int>(0,std::min<int>(255,static_cast<int>(r)+red));
				g = std::max<int>(0,std::min<int>(255,static_cast<int>(g)+green));
				b = std::max<int>(0,std::min<int>(255,static_cast<int>(b)+blue));

				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface greyscale_image(const surface &surf)
{
	if(surf == nullptr)
		return nullptr;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
	}

	{
		surface_lock lock(nsurf);
		uint32_t* beg = lock.pixels();
		uint32_t* end = beg + nsurf->w*surf->h;

		while(beg != end) {
			uint8_t alpha = (*beg) >> 24;

			if(alpha) {
				uint8_t r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);
			//const Uint8 avg = (red+green+blue)/3;

			//use the correct formula for RGB to grayscale
			//conversion. ok, this is no big deal :)
			//the correct formula being:
			//gray=0.299red+0.587green+0.114blue
				const uint8_t avg = static_cast<uint8_t>((
					77  * static_cast<uint16_t>(r) +
					150 * static_cast<uint16_t>(g) +
					29  * static_cast<uint16_t>(b)  ) / 256);

				*beg = (alpha << 24) | (avg << 16) | (avg << 8) | avg;
			}

			++beg;
		}
	}

	//return nsurf;
	return create_optimized_surface(nsurf);
}

surface brighten_image(const surface &surf, fixed_t amount)
{
	if(surf == nullptr) {
		return nullptr;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
	}

	{
		surface_lock lock(nsurf);
		uint32_t* beg = lock.pixels();
		uint32_t* end = beg + nsurf->w*surf->h;

		if (amount < 0) amount = 0;
		while(beg != end) {
			uint8_t alpha = (*beg) >> 24;

			if(alpha) {
				uint8_t r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				r = std::min<unsigned>(static_cast<unsigned>(fxpmult(r, amount)),255);
				g = std::min<unsigned>(static_cast<unsigned>(fxpmult(g, amount)),255);
				b = std::min<unsigned>(static_cast<unsigned>(fxpmult(b, amount)),255);

				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}
	}

	//return surf;
	return create_optimized_surface(nsurf);
}

surface adjust_surface_alpha(const surface &surf, int amount)
{
	if(surf== nullptr) {
		return nullptr;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
	}

	{
		surface_lock lock(nsurf);
		uint32_t* beg = lock.pixels();
		uint32_t* end = beg + nsurf->w*surf->h;

		while(beg != end) {
			uint8_t alpha = (*beg) >> 24;

			if(alpha) {
				uint8_t r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				alpha = uint8_t(std::max<int>(0,std::min<int>(255,static_cast<int>(alpha) + amount)));
				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}
	}


	return create_optimized_surface(nsurf);
	//return nsurf;

}

surface adjust_surface_alpha_add(surface const &surf, int amount)
{
	if(surf== nullptr) {
		return nullptr;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
	}

	{
		surface_lock lock(nsurf);
		uint32_t* beg = lock.pixels();
		uint32_t* end = beg + nsurf->w*surf->h;

		while(beg != end) {
			uint8_t alpha = (*beg) >> 24;

			if(alpha) {
				uint8_t r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				alpha = uint8_t(std::max<int>(0,std::min<int>(255,static_cast<int>(alpha) + amount)));
				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

int inc;

// Applies a mask on a surface
surface mask_surface(surface const &surf, surface const &mask)
{
	if(surf == NULL) {
		return NULL;
	}
//printf("mask_surface  n*%d",inc++);
	//return surf;

	surface nsurf = make_neutral_surface(surf);
	surface nmask(make_neutral_surface(mask));

	if(nsurf == NULL || nmask == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		surface_lock mlock(nmask);

		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;
		Uint32* mbeg = mlock.pixels();
		Uint32* mend = mbeg + nmask->w*nmask->h;

		while(beg != end && mbeg != mend) {
			Uint8 red, green, blue, alpha;
			Uint8 malpha;

			alpha = (*beg) >> 24;
			red   = (*beg) >> 16;
			green = (*beg) >> 8;
			blue  = (*beg) >> 0;

			malpha = (*mbeg) >> 24;

			alpha = Uint8(minimum<int>(malpha, alpha));

			*beg = (alpha << 24) + (red << 16) + (green << 8) + blue;

			++beg;
			++mbeg;
		}
	}

	//return nsurf;
	return create_optimized_surface(nsurf);
}

// Cross-fades a surface
surface blur_surface(surface const &surf, int depth)
{
	if(surf == NULL) {
		return NULL;
	}
	//return surf;

	surface nsurf = make_neutral_surface(surf);
	surface res = create_compatible_surface(nsurf, surf->w, surf->h);

	if(nsurf == NULL || res == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	for(int count = 0; count < depth; ++count) {
		surface_lock lock(nsurf);
		surface_lock rlock(res);

		{
			Uint32* p = lock.pixels();
			Uint32* left = p - 1;
			Uint32* right = p + 1;
			Uint32* top = p - res->w;
			Uint32* bottom = p + res->w;
			Uint32* rp = rlock.pixels();

			for(int y = 0; y < res->h; ++y) {
				for(int x = 0; x < res->w; ++x) {
					Uint8 red, green, blue, alpha;
					Uint16 rred=0, rgreen=0, rblue=0, ralpha=0;

					if (x != 0) {
						SDL_GetRGBA(*left, nsurf->format, &red, &green, &blue, &alpha);
						rred += red;
						rgreen += green;
						rblue += blue;
						ralpha += alpha;
					}

					if (x != res->w - 1) {
						SDL_GetRGBA(*right, nsurf->format, &red, &green, &blue, &alpha);
						rred += red;
						rgreen += green;
						rblue += blue;
						ralpha += alpha;
					}

					if (y != 0) {
						SDL_GetRGBA(*top, nsurf->format, &red, &green, &blue, &alpha);
						rred += red;
						rgreen += green;
						rblue += blue;
						ralpha += alpha;
					}

					if (y != res->h - 1) {
						SDL_GetRGBA(*bottom, nsurf->format, &red, &green, &blue, &alpha);
						rred += red;
						rgreen += green;
						rblue += blue;
						ralpha += alpha;
					}

					rred /= 4;
					rgreen /= 4;
					rblue /= 4;
					ralpha /= 4;

					*rp = SDL_MapRGBA(res->format, (Uint8)rred, (Uint8)rgreen, (Uint8)rblue, (Uint8)ralpha);

					++left; ++right; ++top; ++bottom; ++rp;
				}
			}
		}

		if (count < depth - 1) {
			nsurf = res;
		}
	}

	return create_optimized_surface(res);
}

// Cuts a rectangle from a surface.
surface cut_surface(surface const &surf, SDL_Rect const &r)
{
	surface res = create_compatible_surface(surf, r.w, r.h);

	size_t sbpp = surf->format->BytesPerPixel;
	size_t spitch = surf->pitch;
	size_t rbpp = res->format->BytesPerPixel;
	size_t rpitch = res->pitch;

	surface_lock slock(surf);
	surface_lock rlock(res);

	Uint8* src = reinterpret_cast<Uint8 *>(slock.pixels());
	Uint8* dest = reinterpret_cast<Uint8 *>(rlock.pixels());

	if(r.x >= surf->w)
		return res;

	for(int y = 0; y < r.h && (r.y + y) < surf->h; ++y) {
		Uint8* line_src = src + (r.y + y) * spitch + r.x * sbpp;
		Uint8* line_dest = dest + y * rpitch;
		size_t size = r.w + r.x <= surf->w ? r.w : surf->w - r.x;

		wassert(rpitch >= r.w * rbpp);
		memcpy(line_dest, line_src, size * rbpp);
	}

	return res;
}

surface blend_surface(surface const &surf, double amount, Uint32 colour)
{
	if(surf== NULL) {
		return NULL;
	}

	SDL_Color color = int_to_color(colour);

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		uint16_t ratio = amount * 256;
		const uint16_t red   = ratio * color.r;
		const uint16_t green = ratio * color.g;
		const uint16_t blue  = ratio * color.b;
		ratio = 256 - ratio;

		while(beg != end) {
			uint8_t a = static_cast<uint8_t>(*beg >> 24);
			uint8_t r = (ratio * static_cast<uint8_t>(*beg >> 16) + red)   >> 8;
			uint8_t g = (ratio * static_cast<uint8_t>(*beg >> 8)  + green) >> 8;
			uint8_t b = (ratio * static_cast<uint8_t>(*beg)       + blue)  >> 8;

			*beg = (a << 24) | (r << 16) | (g << 8) | b;

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface flip_surface(const surface &surf)
{
	if(surf == NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* const pixels = lock.pixels();

		for(int y = 0; y != nsurf->h; ++y) {
			for(int x = 0; x != nsurf->w/2; ++x) {
				const int index1 = y*nsurf->w + x;
				const int index2 = (y+1)*nsurf->w - x - 1;
				std::swap(pixels[index1],pixels[index2]);
			}
		}
	}

	return create_optimized_surface(nsurf);
}

surface flop_surface(const surface &surf)
{
	if(surf == NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		uint32_t* const pixels = lock.pixels();

		for(int x = 0; x != nsurf->w; ++x) {
			for(int y = 0; y != nsurf->h/2; ++y) {
				const int index1 = y*nsurf->w + x;
				const int index2 = (nsurf->h-y-1)*surf->w + x;
				std::swap(pixels[index1],pixels[index2]);
			}
		}
	}

	return create_optimized_surface(nsurf);
}


surface create_compatible_surface(const surface &surf, int width, int height)
{
	if(surf == NULL)
		return NULL;

	if(width == -1)
		width = surf->w;

	if(height == -1)
		height = surf->h;

	return SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,surf->format->BitsPerPixel,
		                        surf->format->Rmask,surf->format->Gmask,surf->format->Bmask,surf->format->Amask);
}

void fill_rect_alpha(SDL_Rect &rect, Uint32 colour, Uint8 alpha, surface const &target)
{
	if(alpha == SDL_ALPHA_OPAQUE) {
		SDL_FillRect(target,&rect,colour);
		return;
	} else if(alpha == SDL_ALPHA_TRANSPARENT) {
		return;
	}

	surface tmp(create_compatible_surface(target,rect.w,rect.h));
	if(tmp == NULL) {
		return;
	}

	SDL_Rect r = {0,0,rect.w,rect.h};
	SDL_FillRect(tmp,&r,colour);
	SDL_SetAlpha(tmp,SDL_SRCALPHA,alpha);
	SDL_BlitSurface(tmp,NULL,target,&rect);
}

surface get_surface_portion(const surface &src, SDL_Rect &area)
{
	if (src == NULL) {
		return NULL;
	}
	if(area.x >= src->w || area.y >= src->h || area.x + area.w < 0 || area.y + area.h < 0) {
		return NULL;
	}

	if(area.x + area.w > src->w) {
		area.w = src->w - area.x;
	}

	if(area.y + area.h > src->h) {
		area.h = src->h - area.y;
	}

	surface dst = create_compatible_surface(src, area.w, area.h);
	if(dst == NULL) {
		std::cerr << "Could not create a new surface in get_surface_portion()\n";
		return NULL;
	}

	//SDL_Rect dstarea = {0,0,0,0};

	SDL_BlitSurface(src,&area,dst,NULL);

	return dst;
}

namespace {

struct not_alpha
{
	not_alpha() {}


	bool operator()(uint32_t pixel) const {
		uint8_t alpha = pixel >> 24;
		return alpha != 0x00;
	}

};

}

SDL_Rect get_non_transperant_portion(const surface &surf)
{
	SDL_Rect res = {0,0,0,0};
	surface nsurf(make_neutral_surface(surf));
	if(nsurf == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return res;
	}

	const not_alpha calc;

	surface_lock lock(nsurf);
	const uint32_t* const pixels = lock.pixels();

	int n;
	for(n = 0; n != nsurf->h; ++n) {
		const uint32_t* const start_row = pixels + n*nsurf->w;
		const uint32_t* const end_row = start_row + nsurf->w;

		if(std::find_if(start_row,end_row,calc) != end_row)
			break;
	}

	res.y = n;

	for(n = 0; n != nsurf->h-res.y; ++n) {
		const uint32_t* const start_row = pixels + (nsurf->h-n-1)*surf->w;
		const uint32_t* const end_row = start_row + nsurf->w;

		if(std::find_if(start_row,end_row,calc) != end_row)
			break;
	}

	// The height is the height of the surface,
	// minus the distance from the top and
	// the distance from the bottom.
	res.h = nsurf->h - res.y - n;

	for(n = 0; n != nsurf->w; ++n) {
		int y;
		for(y = 0; y != nsurf->h; ++y) {
			const uint32_t pixel = pixels[y*nsurf->w + n];
			if(calc(pixel))
				break;
		}

		if(y != nsurf->h)
			break;
	}

	res.x = n;

	for(n = 0; n != nsurf->w-res.x; ++n) {
		int y;
		for(y = 0; y != nsurf->h; ++y) {
			const uint32_t pixel = pixels[y*nsurf->w + surf->w - n - 1];
			if(calc(pixel))
				break;
		}

		if(y != nsurf->h)
			break;
	}

	res.w = nsurf->w - res.x - n;

	return res;
}

bool operator==(const SDL_Rect& a, const SDL_Rect& b)
{
	return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

bool operator!=(const SDL_Rect& a, const SDL_Rect& b)
{
	return !operator==(a,b);
}

bool operator==(const SDL_Color& a, const SDL_Color& b) {
	return a.r == b.r && a.g == b.g && a.b == b.b;
}

bool operator!=(const SDL_Color& a, const SDL_Color& b) {
	return !operator==(a,b);
}

SDL_Color inverse(const SDL_Color& colour) {
	SDL_Color inverse;
	inverse.r = 255 - colour.r;
	inverse.g = 255 - colour.g;
	inverse.b = 255 - colour.b;
	inverse.unused = 0;
	return inverse;
}

void pixel_data::read(const config& cfg) {
	const std::string& red = cfg["red"];
	const std::string& green = cfg["green"];
	const std::string& blue = cfg["blue"];

	if (red.empty())
		r = 0;
	else
		r = atoi(red.c_str());

	if (green.empty())
		g = 0;
	else
		g = atoi(green.c_str());

	if (blue.empty())
		b = 0;
	else
		b = atoi(blue.c_str());
}

namespace {
	const SDL_Rect empty_rect = {0,0,0,0};
}

surface_restorer::surface_restorer() : target_(NULL), rect_(empty_rect), surface_(NULL)
{
}

surface_restorer::surface_restorer(CVideo* target, const SDL_Rect& rect)
: target_(target), rect_(rect), surface_(NULL)
{
	update();
}

surface_restorer::~surface_restorer()
{
	restore();
}

void surface_restorer::restore(SDL_Rect const &dst) const
{
	if (surface_.null())
		return;
	SDL_Rect dst2 = intersect_rects(dst, rect_);
	if (dst2.w == 0 || dst2.h == 0)
		return;
	SDL_Rect src = dst2;
	src.x -= rect_.x;
	src.y -= rect_.y;
	SDL_BlitSurface(surface_, &src, target_->getSurface(), &dst2);
	update_rect(dst2);
}

void surface_restorer::restore() const
{
	if (surface_.null())
		return;
	SDL_Rect dst = rect_;
	SDL_BlitSurface(surface_, NULL, target_->getSurface(), &dst);
	update_rect(rect_);
}

void surface_restorer::update()
{
	if(rect_.w == 0 || rect_.h == 0)
		surface_.assign(NULL);
	else
		surface_.assign(::get_surface_portion(target_->getSurface(),rect_));
}

void surface_restorer::cancel()
{
	surface_.assign(NULL);
}

void draw_rectangle(int x, int y, int w, int h, Uint32 colour,surface target)
{

	SDL_Rect top = {x,y,w,1};
	SDL_Rect bot = {x,y+h-1,w,1};
	SDL_Rect left = {x,y,1,h};
	SDL_Rect right = {x+w-1,y,1,h};

	SDL_FillRect(target,&top,colour);
	SDL_FillRect(target,&bot,colour);
	SDL_FillRect(target,&left,colour);
	SDL_FillRect(target,&right,colour);
}

void draw_solid_tinted_rectangle(int x, int y, int w, int h,
                                 int r, int g, int b,
                                 double alpha, surface target)
{

	SDL_Rect rect = {x,y,w,h};
	fill_rect_alpha(rect,SDL_MapRGB(target->format,r,g,b),Uint8(alpha*255),target);
}

