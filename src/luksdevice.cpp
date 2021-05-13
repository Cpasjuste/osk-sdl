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

#include "luksdevice.h"

int LuksDevice::unlock()
{
	SDL_CreateThread(unlock, "lukscryptdevice_unlock", this);
	return 0;
}

int LuksDevice::unlock(void *luksDev)
{
	struct crypt_device *cd;
	int ret = 0;
	const auto lcd = static_cast<LuksDevice *>(luksDev);
	SDL_Event event = {
		.type = lcd->eventType
	};

	// Note: no mutex here, since this function makes a blocking call later on.
	// Careful!
	lcd->running = true;

	usleep(std::chrono::microseconds { MIN_UNLOCK_TIME }.count());

	// Initialize crypt device
	ret = crypt_init(&cd, lcd->devicePath.c_str());
	if (ret < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "crypt_init() failed for %s.", lcd->devicePath.c_str());
		goto DONE;
	}

	// Load header
	ret = crypt_load(cd, nullptr, nullptr);
	if (ret < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "crypt_load() failed on device %s.", crypt_get_device_name(cd));
		crypt_free(cd);
		goto DONE;
	}

	ret = crypt_activate_by_passphrase(
		cd, lcd->deviceName.c_str(),
		CRYPT_ANY_SLOT,
		lcd->passphrase.c_str(),
		lcd->passphrase.size(),
		// Enable TRIM support, disable read/write workqueues for performance on SSDs
		CRYPT_ACTIVATE_ALLOW_DISCARDS | CRYPT_ACTIVATE_NO_READ_WORKQUEUE | CRYPT_ACTIVATE_NO_WRITE_WORKQUEUE);
	if (ret < 0) {
		SDL_Log("crypt_activate_by_passphrase failed on device. Errno %i", ret);
		crypt_free(cd);
		goto DONE;
	}
	SDL_Log("Successfully unlocked device %s", lcd->devicePath.c_str());
	crypt_free(cd);
	lcd->locked = false;

DONE:
	SDL_PushEvent(&event);
	lcd->running = false;
	return ret;
}
