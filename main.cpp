#include "SDL/SDL.h"
#include <iostream>
#include <libcryptsetup.h>
#include <unistd.h>

using namespace std;

#define TICK_INTERVAL 30

static Uint32 next_time;

static int unlock_crypt_dev(const char *path, const char *device_name,
                            const char *passphrase) {
  struct crypt_device *cd;
  struct crypt_active_device cad;
  int ret;

  /* Initialize crypt device */
  ret = crypt_init(&cd, path);
  if (ret < 0) {
    printf("crypt_init() failed for %s.\n", path);
    return ret;
  }

  /* Load header */
  ret = crypt_load(cd, CRYPT_LUKS1, NULL);
  if (ret < 0) {
    printf("crypt_load() failed on device %s.\n", crypt_get_device_name(cd));
    crypt_free(cd);
    return ret;
  }

  ret = crypt_activate_by_passphrase(
      cd, device_name, CRYPT_ANY_SLOT, passphrase, sizeof(passphrase),
      CRYPT_ACTIVATE_ALLOW_DISCARDS); /* Enable TRIM support */
  printf("Successfully unlocked device %s\n", path);
  crypt_free(cd);
  return 0;
}

Uint32 time_left(void) {
  Uint32 now;

  now = SDL_GetTicks();
  if (next_time <= now)
    return 0;
  else
    return next_time - now;
}

int main(int argc, char **args) {
  /*
   * These strings are for development only
   * TODO: Make these mandatory after calling getopts
   */
  char *path = "/home/user/disk";
  char *dev_name = "TEST-DISK";
  char *passphrase = "p";

  bool unlocked = false;
  SDL_Event event;
  float keyboardPosition = 0;
  float keyboardTargetPosition = 1;
  SDL_Surface *screen = NULL;
  int WIDTH = 480;
  int HEIGHT = 800;
  int opt;

  while ((opt = getopt(argc, args, "d:n:")) != -1)
    switch (opt) {
    case 'd':
      path = optarg;
      break;
    case 'n':
      dev_name = optarg;
      break;
    default:
      fprintf(stdout, "Usage: osk_mouse [-d /dev/sda] [-n device_name]\n");
      return -1;
    }

  SDL_Init(SDL_INIT_EVERYTHING);

  // Set up screen
  screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE);
  /*
   * If this fails, try to swap height/width in case device is in
   * 'landscape' mode
   */
  if (screen == NULL) {
    screen = SDL_SetVideoMode(HEIGHT, WIDTH, 32, SDL_SWSURFACE);
    int t = HEIGHT;
    HEIGHT = WIDTH;
    WIDTH = t;
  }
  if (screen == NULL) {
    printf("Unable to set up video mode!\n");
    return -1;
  }
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 128, 0));
  SDL_Flip(screen);

  auto keyboardColor = SDL_MapRGB(screen->format, 30, 30, 30);
  next_time = SDL_GetTicks() + TICK_INTERVAL;

  while (unlocked == false) {
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 128, 0));
    if (SDL_PollEvent(&event)) {
      /* an event was found */
      switch (event.type) {
      /* handle the keyboard */
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_RETURN:
          /* One day this will be sufficient */
          unlocked = !unlock_crypt_dev(path, dev_name, passphrase);
          printf("unlocked: %i\n", unlocked);
          /* for development usage */
          unlocked = true;
          break;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        unsigned int xMouse, yMouse;
        xMouse = event.button.x;
        yMouse = event.button.y;
        /* Debug output only */
        printf("xMouse: %i\tyMouse: %i\n", xMouse, yMouse);
        break;
      }
    }
    if (keyboardPosition != keyboardTargetPosition) {
      if (keyboardPosition > keyboardTargetPosition) {
        keyboardPosition -= (keyboardPosition - keyboardTargetPosition) / 3;
      } else {
        keyboardPosition += (keyboardTargetPosition - keyboardPosition) / 5;
      }
      SDL_Rect keyboardRect;
      keyboardRect.x = 0;
      keyboardRect.y = (int)(HEIGHT - ((HEIGHT / 2) * keyboardPosition));
      keyboardRect.w = WIDTH;
      keyboardRect.h = (int)(HEIGHT / 2 * keyboardPosition);
      SDL_FillRect(screen, &keyboardRect, keyboardColor);
    }
    SDL_Delay(time_left());
    next_time += TICK_INTERVAL;
    SDL_UpdateRect(screen, 0, 0, 0, 0);
  }
  SDL_Quit();
  return 0;
}
