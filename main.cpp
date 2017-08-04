#include "SDL2/SDL.h"
#include <iostream>
#include <libcryptsetup.h>
#include <string>
#include <unistd.h>

using namespace std;

#define TICK_INTERVAL 16

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

void draw_circle(SDL_Renderer *renderer, SDL_Point center, int radius) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  for (int w = 0; w < radius * 2; w++) {
    for (int h = 0; h < radius * 2; h++) {
      int dx = radius - w; // horizontal offset
      int dy = radius - h; // vertical offset
      if ((dx * dx + dy * dy) <= (radius * radius)) {
        SDL_RenderDrawPoint(renderer, center.x + dx, center.y + dy);
      }
    }
  }
}

int main(int argc, char **args) {
  /*
   * These strings are for development only
   * TODO: Make these mandatory after calling getopts
   */
  char path_default[] = "/home/user/disk";
  char dev_name_default[] = "TEST-DISK";
  char *path = NULL;
  char *dev_name = NULL;
  string passphrase;

  bool testmode = false;

  bool unlocked = false;
  SDL_Event event;
  float keyboardPosition = 0;
  float keyboardTargetPosition = 1;
  SDL_Window *display = NULL;
  SDL_Surface *screen = NULL;
  SDL_Renderer *renderer = NULL;
  int WIDTH = 480;
  int HEIGHT = 800;
  int opt;

  while ((opt = getopt(argc, args, "td:n:")) != -1)
    switch (opt) {
    case 't':
      path = path_default;
      dev_name = dev_name_default;
      testmode = true;
      break;
    case 'd':
      path = optarg;
      break;
    case 'n':
      dev_name = optarg;
      break;
    default:
      fprintf(stdout, "Usage: osk_mouse [-t] [-d /dev/sda] [-n device_name]\n");
      return 1;
    }

  if (!path) {
    fprintf(stderr, "No device path specified, use -d [path] or -t\n");
    return 1;
  }

  if (!dev_name) {
    fprintf(stderr, "No device name specified, use -n [name] or -n\n");
    return 1;
  }

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
  if (!SDL_Init(SDL_INIT_EVERYTHING)) {
    printf("*****************\n");
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    SDL_Quit();
    printf("*****************\n");
    return -1;
  }

  for (int a=0; a<SDL_GetNumVideoDrivers(); ++a)
    printf("SDL Vid driver: %s\n", SDL_GetVideoDriver(a));

  if (SDL_VideoInit(NULL) != 0) {
    printf("Error initializing SDL video:  %s\n", SDL_GetError());
    return -1;
  }

  if (!testmode) {
    // Switch to the resolution of the framebuffer if not running
    // in test mode.
    SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};
    if(SDL_GetDisplayMode(0, 0, &mode) != 0){
      printf("Unable to get display resolution!\n");
      SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
      return -1;
    }
    WIDTH = mode.w;
    HEIGHT = mode.h;
  }

  /*
   * Set up display, renderer, & screen
   * Use windowed mode in test mode and device resolution otherwise
   */
  int windowFlags = 0;
  if (testmode) {
    windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
  } else {
    windowFlags = SDL_WINDOW_FULLSCREEN;
  }

  display =
      SDL_CreateWindow("OSK SDL", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, windowFlags);
  if (display == NULL) {
    fprintf(stderr, "Could not create window/display: %s\n", SDL_GetError());
    return 1;
  }

  renderer = SDL_CreateRenderer(display, -1, SDL_RENDERER_SOFTWARE);

  if (renderer == NULL) {
    fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
    return 1;
  }

  screen = SDL_GetWindowSurface(display);

  if (screen == NULL) {
    fprintf(stderr, "Could not get window surface: %s\n", SDL_GetError());
    return 1;
  }

  int keyboardHeight = HEIGHT / 3 * 2;
  if (HEIGHT > WIDTH) {
    // Keyboard height is screen width / max number of keys per row * rows
    keyboardHeight = WIDTH / 2;
  }

  int inputHeight = WIDTH / 10;
  auto backgroundColor = SDL_MapRGB(screen->format, 255, 128, 0);

  if (SDL_FillRect(screen, NULL, backgroundColor) != 0) {
    fprintf(stderr, "Could not fill background color: %s\n", SDL_GetError());
    return 1;
  }

  auto keyboardColor = SDL_MapRGB(screen->format, 30, 30, 30);
  auto inputColor = SDL_MapRGB(screen->format, 255, 255, 255);
  auto dotColor = SDL_MapRGB(screen->format, 0, 0, 0);

  next_time = SDL_GetTicks() + TICK_INTERVAL;

  /* Disable mouse cursor if not in testmode */
  if (SDL_ShowCursor(testmode) < 0) {
    fprintf(stderr, "Setting cursor visibility failed: %s\n", SDL_GetError());
    // Not stopping here, this is a pretty recoverable error.
  }

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
          unlocked = !unlock_crypt_dev(path, dev_name, passphrase.c_str());
          printf("unlocked: %i\n", unlocked);
          /* for development usage */
          unlocked = true;
          break;
        case SDLK_ESCAPE:
          exit(1);
          break;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        unsigned int xMouse, yMouse;
        xMouse = event.button.x;
        yMouse = event.button.y;
        /* Debug output only */
        printf("xMouse: %i\tyMouse: %i\n", xMouse, yMouse);
        passphrase.append("*");
        break;
      }
    }
    if (keyboardPosition != keyboardTargetPosition) {
      if (keyboardPosition > keyboardTargetPosition) {
        keyboardPosition -= (keyboardPosition - keyboardTargetPosition) / 10;
      } else {
        keyboardPosition += (keyboardTargetPosition - keyboardPosition) / 10;
      }

      SDL_Rect keyboardRect;
      keyboardRect.x = 0;
      keyboardRect.y = (int)(HEIGHT - (keyboardHeight * keyboardPosition));
      keyboardRect.w = WIDTH;
      keyboardRect.h = (int)(keyboardHeight * keyboardPosition) + 1;
      SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
      SDL_RenderFillRect(renderer, &keyboardRect);
    }

    // Draw empty password box
    int topHalf = HEIGHT - (keyboardHeight * keyboardPosition);
    SDL_Rect inputRect;
    inputRect.x = WIDTH / 20;
    inputRect.y = (topHalf / 2) - (inputHeight / 2);
    inputRect.w = WIDTH * 0.9;
    inputRect.h = inputHeight;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &inputRect);

    // Draw password dots
    int dotSize = WIDTH * 0.02;
    for (int i = 0; i < passphrase.length(); i++) {
      SDL_Point dotPos;
      dotPos.x = (WIDTH / 10) + (i * dotSize * 3);
      dotPos.y = (topHalf / 2);
      draw_circle(renderer, dotPos, dotSize);
    }

    SDL_Delay(time_left());
    next_time += TICK_INTERVAL;
    /* Update */
    SDL_RenderPresent(renderer);
  }
  SDL_Quit();
  return 0;
}
