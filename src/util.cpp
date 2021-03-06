/*
Copyright (C) 2017-2021 Martijn Braam & Clayton Craft <clayton@craftyguy.net>

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

#include "util.h"
#include "draw_helpers.h"
#include <errno.h>
#include <getopt.h>
#include <numeric>

int fetchOpts(int argc, char **args, Opts *opts)
{
	int opt, optIndex = 0;

	static struct option longOpts[] = {
		{ "testmode", no_argument, 0, 't' },
		{ "keyscript", no_argument, 0, 'k' },
		{ "verbose", no_argument, 0, 'v' },
		{ "no-gles", no_argument, 0, 'G' },
		{ "version", no_argument, 0, 'V' },
		{ "no-keyboard", no_argument, 0, 'x' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, args, "td:n:c:o:kvGVx", longOpts, &optIndex)) != -1)
		switch (opt) {
		case 't':
			opts->luksDevPath = DEFAULT_LUKSDEVPATH;
			opts->luksDevName = DEFAULT_LUKSDEVNAME;
			opts->testMode = true;
			break;
		case 'd':
			opts->luksDevPath = optarg;
			break;
		case 'n':
			opts->luksDevName = optarg;
			break;
		case 'c':
			opts->confPath = optarg;
			break;
		case 'o':
			opts->confOverridePath = optarg;
			break;
		case 'k':
			opts->keyscript = true;
			break;
		case 'v':
			opts->verbose = true;
			break;
		case 'x':
			opts->noKeyboard= true;
			break;
		case 'G':
			opts->noGLES = true;
			break;
		case 'V':
			SDL_Log("osk-sdl v%s", VERSION);
			exit(0);
		default:
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Usage: osk-sdl [-t|--testmode] [-k|--keyscript] [-d /dev/sda] [-n device_name] "
												 "[-c /etc/osk.conf] [-o /boot/osk.conf] "
												 "[-v|--verbose] [-G|--no-gles] [-x|--no-keyboard");
			return 1;
		}
	if (opts->luksDevPath.empty()) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "No device path specified, use -d [path] or -t");
		return 1;
	}
	if (opts->luksDevName.empty()) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "No device name specified, use -n [name] or -t");
		return 1;
	}
	if (opts->confPath.empty()) {
		opts->confPath = DEFAULT_CONFPATH;
	}
	return 0;
}

std::string strVector2str(const std::vector<std::string> &strVector)
{
	const auto strLength = std::accumulate(strVector.begin(), strVector.end(), size_t {},
		[](auto acc, const auto &s) { return acc + s.size(); });
	std::string result;
	result.reserve(strLength);
	for (const auto &str : strVector) {
		result.append(str);
	}
	return result;
}

int find_gles_driver_index()
{
	int render_driver_count = SDL_GetNumRenderDrivers();
	if (render_driver_count < 1) {
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Couldn't find any renderers, will fall back to default: %s", SDL_GetError());
		return -1;
	}

	SDL_RendererInfo renderer_info;
	for (int i = 0; i < render_driver_count; ++i) {
		if (SDL_GetRenderDriverInfo(i, &renderer_info) != 0) {
			SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Failed to get info for driver at index %i: %s", i, SDL_GetError());
			continue;
		}
		if (strncmp(renderer_info.name, "opengles", strlen("opengles")) == 0) {
			SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Located OpenGL ES driver at index %i", i);
			return i;
		}
	}

	SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Couldn't find OpenGL ES driver, will fall back to default");
	return -1;
}

SDL_Texture *dotGlyph = nullptr;
int dotGlyphSize = 0;

void draw_dot_glyph(SDL_Renderer *renderer, SDL_Point center, int size, Config *config)
{
	if (config->inputBoxDotGlyph.compare("") == 0)
		return;

	// Destroy cached texture if radius doesn't match
	if (dotGlyph && dotGlyphSize != size) {
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Destroying previously cached dot glyph texture with radius %i",
				dotGlyphSize);
		SDL_DestroyTexture(dotGlyph);
		dotGlyph = nullptr;
		dotGlyphSize = 0;
	}

	// Cache a new texture if needed
	if (!dotGlyph) {
		SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Caching a new glyph texture with size %i", size);
		TTF_Font *font = TTF_OpenFont(config->keyboardFont.c_str(), size);
		SDL_Color textColor = { config->inputBoxForeground.r, config->inputBoxForeground.g, config->inputBoxForeground.b, config->inputBoxForeground.a };
		SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, config->inputBoxDotGlyph.c_str(), textColor);

		dotGlyph = SDL_CreateTextureFromSurface(renderer, textSurface);
		SDL_FreeSurface(textSurface);
		dotGlyphSize = size;
	}

	// Copy cached texture into display
	int w, h;
	SDL_QueryTexture(dotGlyph, NULL, NULL, &w, &h);
	SDL_Rect rect = { center.x - size / 2, center.y - size / 2, w, h};
	SDL_RenderCopy(renderer, dotGlyph, nullptr, &rect);
}

void draw_password_box_dots(SDL_Renderer *renderer, Config *config, const SDL_Rect &inputRect, int numDots, bool busy)
{
	int deflection = inputRect.h / 4;
	int ypos = inputRect.y + inputRect.h / 2;
	int xmax = inputRect.x + inputRect.w;
	float tick = static_cast<float>(SDL_GetTicks());
	int dotSize = static_cast<int>(inputRect.h / 2);
	int padding = static_cast<int>(inputRect.h / 2);
	int offset = 0;
	/*
	 * NOTE: Clipping is not used with DirectFB since SetClip() seems to be broken(??) on DirectFB
	 */
	if (!isDirectFB())
		SDL_RenderSetClipRect(renderer, &inputRect); // Prevent drawing outside the input bounds
	for (int i = numDots - 1; i >= 0; i--) {
		SDL_Point dotPos;
		dotPos.x = inputRect.x + padding + (i * dotSize) - offset;
		// Offset dot center so that for long passwords the last dot aligns with right edge (minus padding)
		if (dotPos.x + padding > xmax) {
			offset = dotPos.x + padding - xmax;
			dotPos.x -= offset;
		}
		// Stop once we reach a dot that is entirely outside the input rect
		if (dotPos.x + dotSize < inputRect.x) {
			break;
		}
		if (busy && config->animations) {
			dotPos.y = static_cast<int>(ypos + std::sin(tick / 100.0f + i) * deflection);
		} else {
			dotPos.y = ypos;
		}
		draw_dot_glyph(renderer, dotPos, dotSize, config);
	}
	if (!isDirectFB())
		SDL_RenderSetClipRect(renderer, nullptr); // Reset clip rect
}

bool handleVirtualKeyPress(const std::string &tapped, Keyboard &kbd, LuksDevice &lkd,
	std::vector<std::string> &passphrase, bool keyscript)
{
	// return pressed
	if (tapped == "\n") {
		std::string pass = strVector2str(passphrase);
		lkd.setPassphrase(pass);
		if (keyscript) {
			return true;
		}
		lkd.unlock();
	}
	// Backspace pressed
	else if (tapped == KEYCAP_BACKSPACE) {
		if (!passphrase.empty()) {
			passphrase.pop_back();
		}
	}
	// Shift pressed
	else if (tapped == KEYCAP_SHIFT) {
		if (kbd.getActiveLayer() > 1) {
			kbd.setActiveLayer(0);
		} else {
			kbd.setActiveLayer(!kbd.getActiveLayer());
		}
	}
	// Numbers key pressed:
	else if (tapped == KEYCAP_NUMBERS) {
		kbd.setActiveLayer(2);
	}
	// Symbols key pressed
	else if (tapped == KEYCAP_SYMBOLS) {
		kbd.setActiveLayer(3);
	}
	// ABC key was pressed
	else if (tapped == KEYCAP_ABC) {
		kbd.setActiveLayer(0);
	}
	// handle other key presses
	else if (!tapped.empty()) {
		passphrase.push_back(tapped);
	}
	return false;
}

void handleTapBegin(unsigned xTapped, unsigned yTapped, int screenHeight, Keyboard &kbd)
{
	int offsetYTapped = yTapped - static_cast<int>(screenHeight - (kbd.getHeight() * kbd.getPosition()));
	touchArea key = kbd.getKeyForCoordinates(xTapped, offsetYTapped);
	kbd.setHighlightedKey(key);
	// only rumble if an actual key was tapped
	if (!key.keyChar.empty())
		kbd.hapticRumble();
}

void handleTapEnd(unsigned xTapped, unsigned yTapped, int screenHeight, Keyboard &kbd, Toggle &kbdToggle, LuksDevice &lkd, std::vector<std::string> &passphrase, bool keyscript, bool &showPasswordError, bool &done)
{
	showPasswordError = false;
	int offsetYTapped = yTapped - static_cast<int>(screenHeight - (kbd.getHeight() * kbd.getPosition()));

	if (!kbdToggle.isVisible()) {
		/* handle tap on osk */
		touchArea key = kbd.getKeyForCoordinates(xTapped, offsetYTapped);
		touchArea highlightedKey = kbd.getHighlightedKey();

		kbd.unsetHighlightedKey();
		if (key.x1 != highlightedKey.x1 || key.y1 != highlightedKey.y1) {
			return;
		}
		std::string tapped = key.keyChar;
		if (!lkd.unlockRunning()) {
			done = handleVirtualKeyPress(tapped, kbd, lkd, passphrase, keyscript);
		}
	} else if (kbdToggle.isTapped(xTapped, yTapped)) {
		/* disable toggle so osk shows up */
		kbdToggle.setVisible(false);
	}
}

bool isDirectFB()
{
	char *sdlVideoEnv = getenv("SDL_VIDEODRIVER");
	if (sdlVideoEnv && strncmp(sdlVideoEnv, "directfb", strlen("directfb")) == 0) {
		return true;
	}

	const char *sdlCurDriver = SDL_GetCurrentVideoDriver();
	if (sdlCurDriver && strncmp(sdlCurDriver, "directfb", strlen("directfb")) == 0) {
		return true;
	}
	return false;
}

bool hasPhysKeyboard()
{
	if (!std::filesystem::exists("/dev/input")) {
		SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "/dev/input does not exist, skipping physical keyboard check");
		return false;
	}

	for (const auto &file: std::filesystem::directory_iterator("/dev/input")) {

		std::string dev_name = file.path().string();
		if (dev_name.rfind("/dev/input/event", 0) != 0){
			SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Skipping non-event file: %s", dev_name.c_str());
			continue;
		}
		int fd = open(dev_name.c_str(), O_RDONLY);
		if (fd < 0) {
			if (errno == 13) {
				SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Insufficient permissions to perform physical keyboard detection");
				return false;
			}
			SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Unable to open device: %s", dev_name.c_str());
			continue;
		}

		unsigned long keyMask[KEY_MAX / 8 + 1];
		memset(keyMask, 0, sizeof(keyMask));
		ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyMask)), &keyMask);
		close(fd);

		/*
		 * This mask is the first 32-bits advertised by an N900 keyboard. It's obviously a physical keyboard, so
		 * matching at *least* what it shows is a good baseline. Other physical keyboards should have no trouble
		 * matching this.
		 */
		unsigned long mask = 0xF3FF4000;
		if ((keyMask[0] & mask) == mask) {
			SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Probably a physical keyboard: %s", dev_name.c_str());
			return true;
		} else {
			SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Not a physical keyboard: %s", dev_name.c_str());
		}
	}

	return false;
}
