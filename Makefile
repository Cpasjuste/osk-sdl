osk-sdl: main.cpp
	g++ -o osk-sdl main.cpp -std=c++14 -Werror -w -lcryptsetup `sdl-config --cflags --libs`
