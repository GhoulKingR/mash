//
//  configs.hpp
//  MiniBash
//
//  Created by Chigozie Oduah on 05/09/2025.
//

#ifndef CONFIGS_HPP
#define CONFIGS_HPP

#include <unordered_map>
#include <string>
#include <termios.h>

struct Configs {
    std::unordered_map<std::string, std::string> envariables;
    int history_max_size = 500;
    struct termios original, noecho;
    void load_envs();
    
    Configs();
    ~Configs() = default;
    
public:
    Configs(Configs&) = delete;
    Configs(Configs&&) = delete;
    Configs& operator=(const Configs&) = delete;
    Configs& operator=(Configs&&) = delete;
    
    void  set_raw_mode(bool);
    void add_env(std::string);
    static Configs& getInstance();
};

#endif
