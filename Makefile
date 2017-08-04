osk-sdl: main.cpp keyboard.cpp keyboard.h Makefile
	g++ -o osk-sdl main.cpp keyboard.cpp -std=c++14 -Werror -g -lcryptsetup `sdl2-config --cflags --libs`
