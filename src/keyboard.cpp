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
	: position(static_cast<float>(pos))
	, targetPosition(static_cast<float>(targetPos))
	, keyboardWidth(width)
	, keyboardHeight(height)
	, config(config)
{
	lastAnimTicks = SDL_GetTicks();
}

int Keyboard::init(SDL_Renderer *renderer)
{
	loadKeymap();
	int keyLong = std::strtol(config->keyRadius.c_str(), nullptr, 10);
	if (keyLong >= BEZIER_RESOLUTION || static_cast<double>(keyLong) > (keyboardHeight / 5.0) / 1.5) {
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "key-radius must be below %d and %f, it is %d",
			BEZIER_RESOLUTION, (keyboardHeight / 5.0) / 1.5, keyLong);
		keyRadius = 0;
	} else {
		keyRadius = keyLong;
	}
	for (auto &layer : keyboard) {
		layer.surface = makeKeyboard(&layer);
		if (!layer.surface) {
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Unable to generate keyboard surface");
			return 1;
		}
		layer.texture = SDL_CreateTextureFromSurface(renderer, layer.surface);
		if (!layer.texture) {
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Unable to generate keyboard texture");
			return 1;
		}
	}
	lastAnimTicks = SDL_GetTicks();
	return 0;
}

void Keyboard::setTargetPosition(float p)
{
	if (targetPosition - p > 0.1) {
		// Make sure we restart the animation from a smooth
		// starting point:
		lastAnimTicks = SDL_GetTicks();
	}
	targetPosition = p;
}

void Keyboard::setKeyboardColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	keyboardColor.a = a;
	keyboardColor.r = r;
	keyboardColor.g = g;
	keyboardColor.b = b;
}

void Keyboard::updateAnimations()
{
	const int animStep = 20; // 20ms -> 50 FPS
	const int maxFallBehindSteps = 20;
	int now = SDL_GetTicks();

	// First, make sure we didn't fall too far behind:
	if (lastAnimTicks + animStep * maxFallBehindSteps < now) {
		lastAnimTicks = now - animStep; // keep up faster
	}

	// Do gradual animation steps:
	while (lastAnimTicks < now) {
		// Vertical keyboard movement:
		if (fabs(position - targetPosition) > 0.01) {
			if (!(config->animations)) {
				// If animations are disabled, just jump to target:
				position = targetPosition;
			} else {
				// Gradually update the position:
				if (position > targetPosition) {
					position -= fmax(0.1, position - targetPosition) / 8;
					if (position < targetPosition)
						position = targetPosition;
				} else if (position < targetPosition) {
					position += fmax(0.1, targetPosition - position) / 8;
					if (position > targetPosition)
						position = targetPosition;
				}
			}
		} else {
			position = targetPosition;
		}

		// Advance animation tick:
		lastAnimTicks += animStep;
	}
}

void Keyboard::draw(SDL_Renderer *renderer, int screenHeight)
{
	updateAnimations();

	SDL_Rect keyboardRect, srcRect;

	keyboardRect.x = 0;
	keyboardRect.y = static_cast<int>(screenHeight - (keyboardHeight * position));
	keyboardRect.w = keyboardWidth;
	keyboardRect.h = static_cast<int>(keyboardHeight * position);
	// Correct for any issues from rounding
	keyboardRect.y += screenHeight - (keyboardRect.h + keyboardRect.y);

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = keyboardWidth;
	srcRect.h = keyboardRect.h;

	SDL_SetRenderDrawColor(renderer, keyboardColor.r, keyboardColor.g, keyboardColor.b, keyboardColor.a);

	for (const auto &layer : keyboard) {
		if (layer.layerNum == activeLayer) {
			SDL_RenderCopy(renderer, layer.texture, &srcRect, &keyboardRect);
		}
	}
}

bool Keyboard::isInSlideAnimation() const
{
	return (fabs(getTargetPosition() - getPosition()) > 0.001);
}

void Keyboard::drawRow(SDL_Surface *surface, std::vector<touchArea> &keyVector, int x, int y, int width, int height,
	const std::vector<std::string> &keys, int padding, TTF_Font *font) const
{
	auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
	SDL_Color textColor = { 255, 255, 255, 0 };

	auto background = SDL_MapRGB(surface->format, keyboardColor.r, keyboardColor.g, keyboardColor.b);
	int i = 0;
	for (const auto &keyCap : keys) {
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
		keyVector.push_back({ keyCap, x + (i * width), x + (i * width) + width, y, y + height });

		textSurface = TTF_RenderUTF8_Blended(font, keyCap.c_str(), textColor);

		SDL_Rect keyCapRect;
		keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
		keyCapRect.y = keyRect.y + ((keyRect.h / 2) - (textSurface->h / 2));
		keyCapRect.w = keyRect.w;
		keyCapRect.h = keyRect.h;
		SDL_BlitSurface(textSurface, nullptr, surface, &keyCapRect);

		i++;
	}
}

void Keyboard::drawKey(SDL_Surface *surface, std::vector<touchArea> &keyVector, int x, int y, int width, int height,
	char *cap, const char *key, int padding, TTF_Font *font) const
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

	keyVector.push_back({ key, x, x + width, y, y + height });

	textSurface = TTF_RenderUTF8_Blended(font, cap, textColor);

	SDL_Rect keyCapRect;
	keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
	keyCapRect.y = keyRect.y + ((keyRect.h / 2) - (textSurface->h / 2));
	keyCapRect.w = keyRect.w;
	keyCapRect.h = keyRect.h;
	SDL_BlitSurface(textSurface, nullptr, surface, &keyCapRect);
}

SDL_Surface *Keyboard::makeKeyboard(KeyboardLayer *layer) const
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

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, keyboardWidth, keyboardHeight, 32, rmask, gmask,
		bmask, amask);

	if (surface == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "CreateRGBSurface failed: %s", SDL_GetError());
		return nullptr;
	}

	SDL_FillRect(surface, nullptr,
		SDL_MapRGB(surface->format, keyboardColor.r, keyboardColor.g, keyboardColor.b));

	int rowCount = layer->rows.size();
	int rowHeight = keyboardHeight / (rowCount + 1);

	if (TTF_Init() == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "TTF_Init: %s", TTF_GetError());
		return nullptr;
	}

	TTF_Font *font = TTF_OpenFont(config->keyboardFont.c_str(), 24);
	if (!font) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "TTF_OpenFont: %s", TTF_GetError());
		return nullptr;
	}

	// Divide the bottom row in 20 columns and use that for calculations
	int colw = keyboardWidth / 20;

	int sidebuttonsWidth = keyboardWidth / 20 + colw * 2;
	int y = 0;
	int i = 0;
	while (i < rowCount) {
		int rowElementCount = layer->rows[i].size();
		int x = 0;
		if (i < 2 && rowElementCount < 10)
			x = keyboardWidth / 20;
		if (i == 2) /* leave room for shift, "123" or "=\<" key */
			x = keyboardWidth / 20 + colw * 2;
		drawRow(surface, layer->keyVector, x, y, keyboardWidth / 10,
			rowHeight, layer->rows[i], keyboardWidth / 100, font);
		y += rowHeight;
		i++;
	}

	/* Bottom-left key, 123 or ABC key based on which layer we're on: */
	if (layer->layerNum < 2) {
		char nums[] = "123";
		drawKey(surface, layer->keyVector, colw, y, colw * 3, rowHeight,
			nums, KEYCAP_NUMBERS, keyboardWidth / 100, font);
	} else {
		char abc[] = "abc";
		drawKey(surface, layer->keyVector, colw, y, colw * 3, rowHeight,
			abc, KEYCAP_ABC, keyboardWidth / 100, font);
	}
	/* Shift-key that transforms into "123" or "=\<" depending on layer: */
	if (layer->layerNum == 2) {
		char symb[] = "=\\<";
		drawKey(surface, layer->keyVector, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			symb, KEYCAP_SYMBOLS, keyboardWidth / 100, font);
	} else if (layer->layerNum == 3) {
		char nums[] = "123";
		drawKey(surface, layer->keyVector, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			nums, KEYCAP_NUMBERS, keyboardWidth / 100, font);
	} else {
		char shift[64] = "";
		memcpy(shift, KEYCAP_SHIFT, strlen(KEYCAP_SHIFT) + 1);
		drawKey(surface, layer->keyVector, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			shift, KEYCAP_SHIFT, keyboardWidth / 100, font);
	}
	/* Backspace key that is larger-sized (hence also drawn separately) */
	{
		char bcksp[64];
		memcpy(bcksp, KEYCAP_BACKSPACE,
			strlen(KEYCAP_BACKSPACE) + 1);
		drawKey(surface, layer->keyVector, keyboardWidth / 20 + colw * 16,
			y - rowHeight, sidebuttonsWidth, rowHeight,
			bcksp, KEYCAP_BACKSPACE, keyboardWidth / 100, font);
	}

	char space[] = " ";
	drawKey(surface, layer->keyVector, colw * 5, y, colw * 8, rowHeight,
		space, KEYCAP_SPACE, keyboardWidth / 100, font);

	char period[] = ".";
	drawKey(surface, layer->keyVector, colw * 13, y, colw * 2, rowHeight,
		period, KEYCAP_PERIOD, keyboardWidth / 100, font);

	char enter[] = "OK";
	drawKey(surface, layer->keyVector, colw * 15, y, colw * 5, rowHeight,
		enter, KEYCAP_RETURN, keyboardWidth / 100, font);

	return surface;
}

void Keyboard::setActiveLayer(int layerNum)
{
	if (layerNum >= 0) {
		if (static_cast<size_t>(layerNum) <= keyboard.size() - 1) {
			activeLayer = layerNum;
			return;
		}
	}
	SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "Unknown layer number: %i", layerNum);
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
	keyboard.clear();

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

	keyboard.push_back(layer0);
	keyboard.push_back(layer1);
	keyboard.push_back(layer2);
	keyboard.push_back(layer3);
}

std::string Keyboard::getCharForCoordinates(int x, int y)
{
	for (const auto &layer : keyboard) {
		if (layer.layerNum == activeLayer) {
			for (const auto &it : layer.keyVector) {
				if (x > it.x1 && x < it.x2 && y > it.y1 && y < it.y2) {
					return it.keyChar;
				}
			}
		}
	}
	return "";
}
