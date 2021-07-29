/*
Copyright (C) 2021
Clayton Craft <clayton@craftyguy.net>

This file is part of osk-sdl.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "toggle.h"
#include "draw_helpers.h"


Toggle::Toggle(int width, int height, Config *config)
	: config(config)
	, width(width)
	, height(height)
	, visible(false)
{
}

Toggle::~Toggle()
{
	if (texture) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}

int Toggle::init(SDL_Renderer *renderer, const std::string &text)
{
	SDL_Surface *surface;
	Uint32 rmask, gmask, bmask, amask;
	argb foregroundColor, backgroundColor;
	// SDL interprets each pixel as a 32-bit number, so our masks must depend
	//   on the endianness (byte order) of the machine
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask,
		bmask, amask);
	if (surface == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "CreateRGBSurface failed: %s", SDL_GetError());
		return -1;
	}

	foregroundColor = config->inputBoxForeground;
	backgroundColor = config->inputBoxBackground;

	Uint32 background = SDL_MapRGB(surface->format, backgroundColor.r, backgroundColor.g, backgroundColor.b);
	SDL_FillRect(surface, nullptr, background);

	TTF_Font *font = TTF_OpenFont(config->keyboardFont.c_str(), config->keyboardFontSize);
	SDL_Surface *textSurface;
	SDL_Color textColor = { foregroundColor.r, foregroundColor.g, foregroundColor.b, foregroundColor.a };
	textSurface = TTF_RenderText_Blended(font, text.c_str(), textColor);

	SDL_Rect textRect;
	textRect.x = (width / 2) - (textSurface->w / 2);
	textRect.y = (height / 2) - (textSurface->h / 2);
	textRect.w = textSurface->w;
	textRect.h = textSurface->h;
	SDL_BlitSurface(textSurface, nullptr, surface, &textRect);

	texture = SDL_CreateTextureFromSurface(renderer, surface);

	TTF_CloseFont(font);
	SDL_FreeSurface(textSurface);
	SDL_FreeSurface(surface);

	return 0;
}

void Toggle::draw(SDL_Renderer *renderer, int x, int y)
{
	target.x = x;
	target.y = y;
	target.w = width;
	target.h = height;
	SDL_RenderCopy(renderer, texture, nullptr, &target);
}

void Toggle::setVisible(bool val)
{
	visible = val;
	SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "keyboard toggle status: %i", val);
}

bool Toggle::isTapped(int x, int y)
{

	if (x >= target.x && x <= (target.x + target.w) &&
			y >= target.y && y <= (target.y + target.h)) {
		SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "keyboard toggle hit x: %i, y: %i", x, y);
		return true;
	}
	return false;
}

bool Toggle::isVisible()
{
	return visible;
}
