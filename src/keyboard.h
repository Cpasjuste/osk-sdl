/*
Copyright (C) 2017 Martijn Braam & Clayton Craft <clayton@craftyguy.net>

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

#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "config.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <cstdint>
#include <list>
#include <string>
#include <vector>

constexpr char KEYCAP_BACKSPACE[] = "\u2190";
constexpr char KEYCAP_SHIFT[] = "\u2191";
constexpr char KEYCAP_NUMBERS[] = "123";
constexpr char KEYCAP_SYMBOLS[] = "SYM";
constexpr char KEYCAP_ABC[] = "abc";
constexpr char KEYCAP_SPACE[] = " ";
constexpr char KEYCAP_RETURN[] = "\n";
constexpr char KEYCAP_PERIOD[] = ".";

struct touchArea {
	std::string keyChar;
	int x1;
	int x2;
	int y1;
	int y2;
};

struct rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct argb {
	unsigned char a;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct KeyboardLayer {
	SDL_Surface *surface = nullptr;
	SDL_Texture *texture = nullptr;
	std::array<std::vector<std::string>, 3> rows;
	std::vector<touchArea> keyVector;
	int layerNum;
};

class Keyboard {

public:
	/**
	  Constructor for Keyboard
	  @param pos Starting position (e.g. 0 for hidden)
	  @param targetPos Final position (e.g. 1 for max height)
	  @param width Width to draw keyboard
	  @param height Height to draw keyboard
	  @param config Pointer to Config
	  */
	Keyboard(int pos, int targetPos, int width, int height, Config *config);
	/**
	  Get the character/key at the given coordinates
	  @param x X-axis coordinate
	  @param y Y-axis coordinate
	  @return String with value of key at the given coordinates
	  */
	std::string getCharForCoordinates(int x, int y);
	/**
	  Set keyboard color
	  @param a Alpha value
	  @param r Red value
	  @param g Green value
	  @param b Blue value
	  */
	void setKeyboardColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
	/**
	  Get position of keyboard
	  @return Position as a value between 0 and 1 (0% and 100%)
	  */
	float getPosition() const { return position; };
	/**
	  Get keyboard target position
	  @return Target position of keyboard, between 0 (0%) and 1 (100%)
	  */
	float getTargetPosition() const { return targetPosition; };
	/**
	  Set keyboard target position
	  @param p Position between 0 (0%) and 1 (100%)
	  */
	void setTargetPosition(float p);
	/**
	  Get keyboard height
	  @return configured height of keyboard
	  */
	int getHeight() const { return keyboardHeight; };
	/**
	  Draw/update keyboard on the screen
	  @param renderer An initialized SDL_Renderer object
	  @param screenHeight Height of screen
	  */
	void draw(SDL_Renderer *renderer, int screenHeight);
	/**
	  Get the active keyboard layer
	  @return Index of active keyboard layer
	  */
	int getActiveLayer() const { return activeLayer; };
	/**
	  Set the active keyboard layer
	  @param layerNum Index of layer to activate
	  */
	void setActiveLayer(int layerNum);
	/**
	  Initialize keyboard object
	  @param renderer Initialized SDL_Renderer object
	  @return 0 on success, non-zero on error
	  */
	int init(SDL_Renderer *renderer);
	/**
	  Query whether keyboard is currently sliding up/down.
	  */
	bool isInSlideAnimation() const;

private:
	argb keyboardColor = { 0, 0, 0, 0 };
	int keyRadius = 0;
	float position;
	float targetPosition;
	int lastAnimTicks = 0;
	int keyboardWidth;
	int keyboardHeight;
	int activeLayer = 0;
	std::vector<KeyboardLayer> keyboard;
	Config *config;

	/**
	  Draw keyboard row
	  @param surface Surface to draw on
	  @param keyList List of keys for keyboard layout
	  @param x X-axis coord. for start of row
	  @param y Y-axis coord. for start of row
	  @param width Width of row
	  @param height Height of row
	  @param cap Key cap
	  @param key Key text
	  @param padding Spacing to reserve around the key
	  @param font Font to use for key character
	  */
	void drawRow(SDL_Surface *surface, std::vector<touchArea> &keyVector, int x, int y,
		int width, int height, const std::vector<std::string> &keys, int padding,
		TTF_Font *font) const;

	/**
	  Internal function to gradually update the animations.
	  Will be implicitly called by the draw function.
	  */
	void updateAnimations();

	/**
	  Draw key for keyboard
	  @param surface Surface to draw on
	  @param keyList List of keys for keyboard layout
	  @param x X-axis coord. for start of row
	  @param y Y-axis coord. for start of row
	  @param width Width of row
	  @param height Height of row
	  @param cap Key cap
	  @param key Key text
	  @param padding Spacing to reserve around the key
	  @param font Font to use for key character
	  */
	void drawKey(SDL_Surface *surface, std::vector<touchArea> &keyVector, int x, int y,
		int width, int height, char *cap, const char *key, int padding, TTF_Font *font) const;
	/**
	  Prepare new keyboard
	  @param layer Keyboard layer to use
	  @return New SDL_Surface, or nullptr on error
	  */
	SDL_Surface *makeKeyboard(KeyboardLayer *layer) const;
	/**
	  Load a keymap into the keyboard
	  */
	void loadKeymap();
};
#endif
