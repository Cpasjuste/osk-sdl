osk-sdl: main.cpp
	g++ -o osk-sdl main.cpp -w -lcryptsetup `sdl-config --cflags --libs`
