//
//  configs.cpp
//  MiniBash
//
//  Created by Chigozie Oduah on 05/09/2025.
//
#include <termios.h>
#include <unistd.h>
#include <sstream>

#include "configs.hpp"

extern char** environ;

Configs& Configs::getInstance() {
    static Configs instance;
    return instance;
}

Configs::Configs() {
    load_envs();
    tcgetattr(STDIN_FILENO, &original);
    noecho = original;
    noecho.c_lflag &= ~(ECHO | ICANON | ISIG);
}

void Configs::load_envs() {
    for (char **current = environ; *current; current++) {
        std::string line (*current);
        size_t pos = line.find("=");
        if (pos != line.npos) {
            std::string variable = line.substr(0, pos);
            std::string value = line.substr(pos+1, line.size());
            envariables[variable] = value;
            continue;
        }
    }
}

void Configs::add_env(std::string name) {
    putenv((char*) name.c_str());
    
    std::string line (name);
    size_t pos = line.find("=");
    if (pos != line.npos) {
        std::string variable = line.substr(0, pos);
        std::string value = line.substr(pos+1, line.size());
        envariables[variable] = value;
    }
}

void Configs::set_raw_mode(bool raw_mode) {
    if (raw_mode) {
        tcsetattr(STDIN_FILENO, TCSANOW, &noecho);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &original);
    }
}
