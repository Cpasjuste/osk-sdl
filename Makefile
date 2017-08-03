osk-sdl: main.cpp Makefile
	g++ -o osk-sdl main.cpp -std=c++14 -Werror -g -lcryptsetup `sdl2-config --cflags --libs`
