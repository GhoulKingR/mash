//
//  cmd.cpp
//  MiniBash
//
//  Created by Chigozie Oduah on 05/09/2025.
//
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <filesystem>
#include <sys/stat.h>
#include <stdlib.h>

#include "cmd.hpp"
#include "configs.hpp"

namespace fs = std::filesystem;

Command& Command::getInstance() {
    static Command instance;
    return instance;
}

std::vector<std::string> split(std::string full_line) {
    std::stringstream liness(full_line);
    std::vector<std::string> result;
    std::string tmp;
    
    while (std::getline(liness, tmp, ':')) {
        result.push_back(tmp);
    }
    
    return result;
}

void Command::exec(std::vector<std::string>& args) {
    Configs& configs = Configs::getInstance();
    configs.set_raw_mode(false);
    
    if (args[0] == "exit") {
        exit(EXIT_SUCCESS);
    } else if (args[0] == "echo") {
        std::cout << args[1] << std::endl;
    } else if (args[0] == "set") {
        configs.add_env(args[1]);
    } else {
        bool found = false;
        
        // look for the command in path
        for (auto dir : split(configs.envariables["PATH"])) {
            fs::path dir_path (dir);
            fs::path command (args[0]);
            fs::path full_path = dir_path / command;
            struct stat sb;
            
            // execute the executable it has found
            if (stat(full_path.c_str(), &sb) == 0) {
                int pid, status;
                
                // fork the process
                if ((pid = fork())) {
                    // Parent process; wait for child to exit
                    waitpid(pid, &status, 0);
                    
                } else {
                    // child process
                    std::vector<char*> cstrs;
                    cstrs.reserve(args.size());
                    
                    for (auto& s : args) {
                        cstrs.push_back(const_cast<char*>(s.c_str()));
                    }
                    
                    execv(full_path.c_str(), cstrs.data());
                    exit(EXIT_SUCCESS);
                }
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << "mash: " << args[0] << " not found" << std::endl;
        }
    }
    configs.set_raw_mode(true);
}
