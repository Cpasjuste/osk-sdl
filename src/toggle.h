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

#ifndef TOGGLE_H
#define TOGGLE_H
#include "config.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <cstdint>
#include <list>
#include <string>
#include <vector>

class Toggle {
public:
	/**
	  Constructor
	  @param width Width of the toggle
	  @param height Height of the toggle
	  @param config Config object
	  */
	Toggle(int width, int height, Config *config);
	~Toggle();
	/**
	  Initialize toggle
	  @param renderer Initialized SDL renderer object
	  @param image_path Path to image file to use in the toggle
	  @return Non-zero int on failure
	  */
	int init(SDL_Renderer *renderer, const std::string &text);
	/**
	  Draw toggle
	  @param renderer Initialized SDL renderer object
	  @param x X-axis coordinate
	  @param y Y-axis coordinate
	  */
	void draw(SDL_Renderer *renderer, int x, int y);

	bool isVisible();
	void setVisible(bool val);
	bool isTapped(int x, int y);

private:
	SDL_Texture *texture = nullptr;
	SDL_Rect target;
	Config *config;
	int width;
	int height;
	bool visible;
};
#endif
