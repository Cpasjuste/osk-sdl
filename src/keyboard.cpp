/*
Copyright (C) 2017-2020
Martijn Braam, Clayton Craft <clayton@craftyguy.net>, et al.

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

#include "keyboard.h"
#include "draw_helpers.h"

Keyboard::Keyboard(int pos, int targetPos, int width, int height, Config *config)
{

	this->position = pos;
	this->targetPosition = targetPos;
	this->keyboardWidth = width;
	this->keyboardHeight = height;
	this->config = config;
	this->lastAnimTicks = SDL_GetTicks();
}

Keyboard::~Keyboard()
{
	std::list<KeyboardLayer>::iterator layer;
	for (layer = keyboard.begin(); layer != keyboard.end(); ++layer) {
		delete (*layer).surface;
	}
}

int Keyboard::init(SDL_Renderer *renderer)
{
	std::list<KeyboardLayer>::iterator layer;

	loadKeymap();
	long keyLong = strtol(config->keyRadius.c_str(), nullptr, 10);
	if (keyLong >= BEZIER_RESOLUTION || keyLong > (keyboardHeight / 5) / 1.5) {
		fprintf(stderr, "key-radius must be below %f and %f, it is %ld\n",
			BEZIER_RESOLUTION, (keyboardHeight / 5) / 1.5, keyLong);
		keyRadius = 0;
	} else {
		keyRadius = keyLong;
	}
	for (layer = this->keyboard.begin(); layer != this->keyboard.end(); ++layer) {
		(*layer).surface = makeKeyboard(&(*layer));
		if (!(*layer).surface) {
			fprintf(stderr, "ERROR: Unable to generate keyboard surface\n");
			return 1;
		}
		(*layer).texture = SDL_CreateTextureFromSurface(renderer, layer->surface);
		if (!(*layer).texture) {
			fprintf(stderr, "ERROR: Unable to generate keyboard texture\n");
			return 1;
		}
	}
	this->lastAnimTicks = SDL_GetTicks();
	return 0;
}

float Keyboard::getPosition()
{
	return this->position;
}

float Keyboard::getTargetPosition()
{
	return this->targetPosition;
}

void Keyboard::setTargetPosition(float p)
{
	if (this->targetPosition - p > 0.1) {
		// Make sure we restart the animation from a smooth
		// starting point:
		this->lastAnimTicks = SDL_GetTicks();
	}
	this->targetPosition = p;
	return;
}

void Keyboard::setKeyboardColor(int a, int r, int g, int b)
{
	this->keyboardColor.a = a;
	this->keyboardColor.r = r;
	this->keyboardColor.g = g;
	this->keyboardColor.b = b;
}

float Keyboard::getHeight()
{
	return this->keyboardHeight;
}

void Keyboard::updateAnimations()
{
	const int animStep = 20; // 20ms -> 50 FPS
	const int maxFallBehindSteps = 20;
	int now = SDL_GetTicks();

	// First, make sure we didn't fall too far behind:
	if (this->lastAnimTicks + animStep * maxFallBehindSteps < now) {
		this->lastAnimTicks = now - animStep; // keep up faster
	}

	// Do gradual animation steps:
	while (this->lastAnimTicks < now) {
		float position = this->position;
		float targetPosition = this->targetPosition;

		// Vertical keyboard movement:
		if (fabs(position - targetPosition) > 0.01) {
			if (!(config->animations)) {
				// If animations are disabled, just jump to target:
				this->position = targetPosition;
			} else {
				// Gradually update the position:
				if (position > targetPosition) {
					this->position -= fmax(0.1, position - targetPosition) / 8;
					if (this->position < targetPosition)
						this->position = targetPosition;
				} else if (position < targetPosition) {
					this->position += fmax(0.1, targetPosition - position) / 8;
					if (this->position > targetPosition)
						this->position = targetPosition;
				}
			}
		} else {
			this->position = targetPosition;
		}

		// Advance animation tick:
		this->lastAnimTicks += animStep;
	}
}

void Keyboard::draw(SDL_Renderer *renderer, Config *config, int screenHeight)
{
	this->updateAnimations();

	std::list<KeyboardLayer>::iterator layer;
	SDL_Rect keyboardRect, srcRect;

	keyboardRect.x = 0;
	keyboardRect.y = (int)(screenHeight - (this->keyboardHeight * this->position));
	keyboardRect.w = this->keyboardWidth;
	keyboardRect.h = (int)(this->keyboardHeight * this->position);
	// Correct for any issues from rounding
	keyboardRect.y += screenHeight - (keyboardRect.h + keyboardRect.y);

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = this->keyboardWidth;
	srcRect.h = keyboardRect.h;

	SDL_SetRenderDrawColor(renderer, this->keyboardColor.a, this->keyboardColor.r,
		this->keyboardColor.g, this->keyboardColor.b);

	for (layer = keyboard.begin(); layer != keyboard.end(); ++layer) {
		if ((*layer).layerNum == this->activeLayer) {
			SDL_RenderCopy(renderer, (*layer).texture,
				&srcRect, &keyboardRect);
		}
	}
	return;
}

bool Keyboard::isInSlideAnimation()
{
	return (fabs(getTargetPosition() - getPosition()) > 0.001);
}

void Keyboard::drawRow(SDL_Surface *surface, std::vector<touchArea> *keyList, int x, int y, int width, int height,
	std::list<std::string> *keys, int padding, TTF_Font *font)
{

	auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
	SDL_Color textColor = { 255, 255, 255, 0 };

	auto background = SDL_MapRGB(surface->format, keyboardColor.r, keyboardColor.g, keyboardColor.b);
	int i = 0;
	std::list<std::string>::const_iterator keyCap;
	for (keyCap = keys->begin(); keyCap != keys->end(); ++keyCap) {
		SDL_Rect keyRect;
		keyRect.x = x + (i * width) + padding;
		keyRect.y = y + padding;
		keyRect.w = width - (2 * padding);
		keyRect.h = height - (2 * padding);
		SDL_FillRect(surface, &keyRect, keyBackground);
		if (keyRadius > 0) {
			smooth_corners_surface(surface, background, &keyRect, keyRadius);
		}
		SDL_Surface *textSurface;
		keyList->push_back({ *keyCap, x + (i * width), x + (i * width) + width, y, y + height });

		textSurface = TTF_RenderUTF8_Blended(font, keyCap->c_str(), textColor);

		SDL_Rect keyCapRect;
		keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
		keyCapRect.y = keyRect.y + ((keyRect.h / 2) - (textSurface->h / 2));
		keyCapRect.w = keyRect.w;
		keyCapRect.h = keyRect.h;
		SDL_BlitSurface(textSurface, nullptr, surface, &keyCapRect);

		i++;
	}
}

void Keyboard::drawKey(SDL_Surface *surface, std::vector<touchArea> *keyList, int x, int y, int width, int height, char *cap,
	const std::string *key, int padding, TTF_Font *font)
{
	auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
	SDL_Color textColor = { 255, 255, 255, 0 };

	SDL_Rect keyRect;
	keyRect.x = x + padding;
	keyRect.y = y + padding;
	keyRect.w = width - (2 * padding);
	keyRect.h = height - (2 * padding);
	SDL_FillRect(surface, &keyRect, keyBackground);
	SDL_Surface *textSurface;

	keyList->push_back({ *key, x, x + width, y, y + height });

	textSurface = TTF_RenderUTF8_Blended(font, cap, textColor);

	SDL_Rect keyCapRect;
	keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
	keyCapRect.y = keyRect.y + ((keyRect.h / 2) - (textSurface->h / 2));
	keyCapRect.w = keyRect.w;
	keyCapRect.h = keyRect.h;
	SDL_BlitSurface(textSurface, nullptr, surface, &keyCapRect);
}

SDL_Surface *Keyboard::makeKeyboard(KeyboardLayer *layer)
{
	SDL_Surface *surface;
	Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
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

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, this->keyboardWidth,
		this->keyboardHeight, 32, rmask, gmask,
		bmask, amask);

	if (surface == nullptr) {
		fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		return nullptr;
	}

	SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, this->keyboardColor.r, this->keyboardColor.g, this->keyboardColor.b));

	int rowCount = sizeof(layer->rows) / sizeof(*layer->rows);
	int rowHeight = this->keyboardHeight / (rowCount + 1);

	if (TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		return nullptr;
	}

	TTF_Font *font = TTF_OpenFont(config->keyboardFont.c_str(), 24);
	if (!font) {
		printf("TTF_OpenFont: %s\n", TTF_GetError());
		return nullptr;
	}

	// Divide the bottom row in 20 columns and use that for calculations
	int colw = this->keyboardWidth / 20;

	int sidebuttonsWidth = this->keyboardWidth / 20 + colw * 2;
	int y = 0;
	int i = 0;
	while (i < rowCount) {
		int rowElementCount = layer->rows[i].size();
		int x = 0;
		if (i < 2 && rowElementCount < 10)
			x = this->keyboardWidth / 20;
		if (i == 2) /* leave room for shift, "123" or "=\<" key */
			x = this->keyboardWidth / 20 + colw * 2;
		drawRow(surface, &layer->keyList, x, y, this->keyboardWidth / 10,
			rowHeight, &layer->rows[i], this->keyboardWidth / 100, font);
		y += rowHeight;
		i++;
	}

	/* Bottom-left key, 123 or ABC key based on which layer we're on: */
	if (layer->layerNum < 2) {
		char nums[] = "123";
		drawKey(surface, &layer->keyList, colw, y, colw * 3, rowHeight,
			nums, &KEYCAP_NUMBERS, this->keyboardWidth / 100, font);
	} else {
		char abc[] = "abc";
		drawKey(surface, &layer->keyList, colw, y, colw * 3, rowHeight,
			abc, &KEYCAP_ABC, this->keyboardWidth / 100, font);
	}
	/* Shift-key that transforms into "123" or "=\<" depending on layer: */
	if (layer->layerNum == 2) {
		char symb[] = "=\\<";
		drawKey(surface, &layer->keyList, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			symb, &KEYCAP_SYMBOLS, this->keyboardWidth / 100, font);
	} else if (layer->layerNum == 3) {
		char nums[] = "123";
		drawKey(surface, &layer->keyList, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			nums, &KEYCAP_NUMBERS, this->keyboardWidth / 100, font);
	} else {
		char shift[64] = "";
		memcpy(shift, KEYCAP_SHIFT.c_str(), strlen(KEYCAP_SHIFT.c_str()) + 1);
		drawKey(surface, &layer->keyList, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			shift, &KEYCAP_SHIFT, this->keyboardWidth / 100, font);
	}
	/* Backspace key that is larger-sized (hence also drawn separately) */
	{
		char bcksp[64];
		memcpy(bcksp, KEYCAP_BACKSPACE.c_str(),
			strlen(KEYCAP_BACKSPACE.c_str()) + 1);
		drawKey(surface, &layer->keyList, this->keyboardWidth / 20 + colw * 16,
			y - rowHeight, sidebuttonsWidth, rowHeight,
			bcksp, &KEYCAP_BACKSPACE, this->keyboardWidth / 100, font);
	}

	char space[] = " ";
	drawKey(surface, &layer->keyList, colw * 5, y, colw * 8, rowHeight,
		space, &KEYCAP_SPACE, this->keyboardWidth / 100, font);

	char period[] = ".";
	drawKey(surface, &layer->keyList, colw * 13, y, colw * 2, rowHeight,
		period, &KEYCAP_PERIOD, this->keyboardWidth / 100, font);

	char enter[] = "OK";
	drawKey(surface, &layer->keyList, colw * 15, y, colw * 5, rowHeight,
		enter, &KEYCAP_RETURN, this->keyboardWidth / 100, font);

	return surface;
}

void Keyboard::setActiveLayer(int layerNum)
{
	if (layerNum >= 0) {
		if ((std::string::size_type)layerNum <= keyboard.size() - 1) {
			this->activeLayer = layerNum;
			return;
		}
	}
	fprintf(stderr, "Unknown layer number: %i\n", layerNum);
	return;
}

int Keyboard::getActiveLayer()
{
	return this->activeLayer;
}

/*
 * This function is not actually parsing any external keymaps, it's currently
 * filling in the keyboardKeymap object with a US/QWERTY layout until parsing
 * from a file is implemented
 *
 * Be careful when changing the layout, you could lock somebody out who is
 * using these symbols in their password!
 *
 * If the symbols are changed, then the check for allowed characters in
 * postmarketos-ondev#14 needs to be adjusted too. This has the same layout as
 * squeekboard now:
 * https://source.puri.sm/Librem5/squeekboard/-/blob/master/data/keyboards/us.yaml
 */
void Keyboard::loadKeymap()
{
	KeyboardLayer layer0, layer1, layer2, layer3;
	this->keyboard.clear();

	layer0.rows[0] = { "q", "w", "e", "r", "t", "y", "u", "i", "o", "p" };
	layer0.rows[1] = { "a", "s", "d", "f", "g", "h", "j", "k", "l" };
	layer0.rows[2] = { "z", "x", "c", "v", "b", "n", "m" };
	layer0.layerNum = 0;

	layer1.rows[0] = { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P" };
	layer1.rows[1] = { "A", "S", "D", "F", "G", "H", "J", "K", "L" };
	layer1.rows[2] = { "Z", "X", "C", "V", "B", "N", "M" };
	layer1.layerNum = 1;

	layer2.rows[0] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };
	layer2.rows[1] = { "@", "#", "$", "%", "&", "-", "_", "+", "(", ")" };
	layer2.rows[2] = { ",", "\"", "'", ":", ";", "!", "?" };
	layer2.layerNum = 2;

	layer3.rows[0] = { "~", "`", "|", "·", "√", "π", "τ", "÷", "×", "¶" };
	layer3.rows[1] = { "©", "®", "£", "€", "¥", "^", "°", "*", "{", "}" };
	layer3.rows[2] = { "\\", "/", "<", ">", "=", "[", "]" };
	layer3.layerNum = 3;

	this->keyboard.push_back(layer0);
	this->keyboard.push_back(layer1);
	this->keyboard.push_back(layer2);
	this->keyboard.push_back(layer3);

	return;
}

std::string Keyboard::getCharForCoordinates(int x, int y)
{
	std::list<KeyboardLayer>::iterator layer;
	std::vector<touchArea>::iterator it;

	for (layer = this->keyboard.begin(); layer != this->keyboard.end(); ++layer) {
		if ((*layer).layerNum == this->activeLayer) {
			for (it = (*layer).keyList.begin(); it != (*layer).keyList.end(); ++it) {
				if (x > it->x1 && x < it->x2 && y > it->y1 && y < it->y2) {
					return it->keyChar;
				}
			}
		}
	}
	return "\0";
}
