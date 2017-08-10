#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Config::Config() {}

bool Config::Read(string path) {
  std::ifstream is(path, std::ifstream::binary);
  if (!is) {
    fprintf(stderr, "Could not open config file: %s\n", path.c_str());
    return false;
  }
  if (!Config::Parse(is)) {
    fprintf(stderr, "Could not parse config file: %s\n", path.c_str());
    return false;
  }

  return true;
}

bool Config::Parse(istream &file) {
  int lineno=0;
  for (std::string line; std::getline(file, line);) {
    lineno++;

    std::istringstream iss(line);
    std::string id, eq, val;

    bool error = false;

    if (!(iss >> id)) {
      continue;
    } else if (id[0] == '#') {
      continue;
    } else if (id == "") {
      continue;
    } else if (!(iss >> eq >> val >> std::ws) || eq != "=" ||
               iss.get() != EOF) {
      error = true;
    }

    if (error) {
      fprintf(stderr, "Syntax error on line %d\n", lineno);
      return false;
    } else {
      Config::options[id] = val;
    }
  }
  return true;
}
