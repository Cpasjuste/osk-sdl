Lightweight On-Screen-Keyboard based on SDL2
---

This keyboard is used to unlock the encrypted root partition in
[postmarketOS](https://postmarketos.org).

Photos/Videos:
* [Running on Nokia N900](https://user-images.githubusercontent.com/1474209/29724945-5035d652-897f-11e7-88ea-148265c799a1.jpg)
* [Running on Nexus 4 and Samsung Galaxy SII](https://wiki.postmarketos.org/wiki/File:Osk-sdl-mako-i9100.jpg)
* [Unlocking animation](https://postmarketos.org/static/img/2017-09-03/osk-wave.gif)


### Tests:
Functional tests can be run with `make check`, which will use `xvfb-run`. Mesa w/ swrast is needed if running on a headless system (e.g., a CI).
