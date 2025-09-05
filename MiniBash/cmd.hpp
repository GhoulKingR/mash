//
//  cmd.hpp
//  MiniBash
//
//  Created by Chigozie Oduah on 05/09/2025.
//
#ifndef CMD_HPP
#define CMD_HPP

#include "configs.hpp"

class Command {
    Command() = default;
    ~Command() = default;
        
public:
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;
    Command(Command&&) = delete;
    Command& operator=(Command&&) = delete;
    
    void exec(std::vector<std::string>&);
    static Command& getInstance();
};

#endif
