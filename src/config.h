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

#ifndef CONFIG_H
#define CONFIG_H
#include <map>
#include <string>

class Config {
public:
	std::string keyboardBackground = "#333333";
	std::string wallpaper = "#FF9900";
	std::string keyboardFont = "DejaVu";
	std::string keyboardMap = "us";
	std::string inputBoxRadius = "0";
	std::string keyRadius = "0";
	bool animations = true;

	/**
	  Constructor for Config
	  */
	Config();
	/**
	  Read from config file
	  @path Path to config file
	  */
	bool Read(std::string path);

private:
	std::map<std::string, std::string> options;

	/**
	  Parse configuration file
	  @file File to parse
	  */
	bool Parse(std::istream &file);
};

#endif // CONFIG_H
