Lightweight On-Screen-Keyboard based on SDL2
---

This keyboard is used to unlock the encrypted root partition in
[postmarketOS](https://postmarketos.org) and other distributions.

[![](https://wiki.postmarketos.org/images/thumb/c/ca/Osk-sdl-3x.jpeg/320px-Osk-sdl-3x.jpeg)](https://wiki.postmarketos.org/wiki/File:Osk-sdl-3x.jpeg)

### Building:

This project uses meson, and can be built with:

```
$ meson _build
$ meson compile -C _build
```

### Tests:

Functional tests require `xvfb-run`. Mesa w/ swrast is needed if running on a headless system (e.g., a CI).

```
$ meson test -C _build
```

Note: Tests which require elevated privileges will be skipped.
