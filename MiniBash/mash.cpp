#include <iostream>
#include <signal.h>

#include "history.hpp"
#include "cmd.hpp"
#include "configs.hpp"


void print(std::string content, int backspace) {
    std::string backspace_text = backspace > 0 ? "\e[" + std::to_string(backspace) + "D" : "";
    std::cout << "\e[2K\r$ " << content << backspace_text;
}

void print() {
    return print("", 0);
}

void print(std::string& content) {
    return print(content, 0);
}

std::string getcmdline() {
    std::string line = "";
    auto& history = History::getInstance();

    print();
    int backspace = 0;

    while(true) {
        char c = getchar();
        if (c == '\n') {
            if (line.size() > 0)
                printf("\n");
            break;
        } else if (c == '\x04') {
            raise(SIGINT);
            return "";
        } else if (c == '\x03') {
            return "";
        } else if (c == '\b' || c == '\x7f') {
            size_t text_size = line.size();
            if (text_size > 0) {
                if (backspace > 0) {
                    for (size_t i = text_size - backspace - 1; i < text_size - 1; i++) {
                        char t = line[i];
                        line[i] = line[i + 1];
                        line[i + 1] = t;
                    }
                }

                line.pop_back();

                print(line, backspace);
                continue;
            }
        } else if (c == '\e') {
            char c1 = getchar();
            char c2 = getchar();
            if (c1 == '[') {
                if (c2 == 'A') { // up
                    if (history.move_up()) {
                        line = history.get_current();
                        backspace = 0;
                        print(line);
                    }
                } else if (c2 == 'B') { // down
                    if (history.move_down()) {
                        line = history.get_current();
                        backspace = 0;
                        print(line);
                    }
                } else if (c2 == 'C') { // right
                    if (backspace > 0)
                        backspace--;

                    print(line, backspace);
                } else if (c2 == 'D') { // left
                    if (backspace < line.size())
                        backspace++;
                    print(line, backspace);
                }
                continue;
            }
        }

        // change the add text logic
        if (backspace > 0 && isprint(c)) {
            char tmp = c;
            size_t text_size = line.size();
            line += ' ';
            for (size_t i = text_size - backspace; i <= text_size; i++) {
                char t = line[i];
                line[i] = tmp;
                tmp = t;
            }
        } else if (isprint(c)) {
            line += c;
        }
        print(line, backspace);
    }

    history.add_item(line);
    return line;
}

std::vector<std::string> parseargs(std::string& line) {
    std::vector<std::string> args;
    std::string collected_arg = "";
    size_t len = line.size();

    bool escape = false;
    bool quotes = false;

    for (size_t i = 0; i < len + 1; i++) {
        if (line[i] == '\\') {
            escape = true;
            continue;
        }

        if (line[i] == '"' && !escape) {
            quotes = !quotes;
            continue;
        }

        if ((line[i] == ' ' && !escape && !quotes) || line[i] == '\0') {
            if (collected_arg.size() > 0) {
                args.push_back(collected_arg);
                collected_arg = "";
            }
            continue;
        }

        if (escape) {
            switch (line[i]) {
                case 'n':
                    collected_arg += '\n';
                    break;

                case 'b':
                    collected_arg += '\b';
                    break;

                default:
                    collected_arg += line[i];
                    break;
            }
            escape = false;
        } else {
            collected_arg += line[i];
        }
    }

    return args;
}


void exit_handler() {
    Configs::getInstance().set_raw_mode(false);
}

void fill_envs(std::string& line) {
    Configs& configs = Configs::getInstance();
    for (auto envs : configs.envariables) {
        std::string lookfor = "$" + envs.first;
        std::string replacewith = envs.second;
        
        // escape all spaces in replacewith
        size_t pr = replacewith.find(" ");
        while (pr != std::string::npos) {
            replacewith.replace(pr, 1, "\\s");
            pr = replacewith.find(" ");
        }
        pr = replacewith.find("\\s");
        while (pr != std::string::npos) {
            replacewith.replace(pr, 2, "\\ ");
            pr = replacewith.find("\\s");
        }
                
        size_t pos = line.find(lookfor);
        while (pos != std::string::npos) {
            line.replace(pos, lookfor.size(), replacewith);
            pos = line.find(lookfor);
        }
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, exit);
    
    Configs& configs = Configs::getInstance();
    Command& cmd = Command::getInstance();

    int result = atexit(exit_handler);
    if (result != 0) {
        printf("Could not register exit handler!");
        exit(EXIT_FAILURE);
    }
    
    configs.set_raw_mode(true);

    if (argc > 1) {
        printf("You ran this with some arguments\n");
    } else {
        while (true) {
            std::string line = getcmdline();
            if (line.size() == 0) {
                printf("\n");
                continue;
            }

            fill_envs(line);
            std::vector<std::string> args = parseargs(line);

            if (!args.empty()) {
                cmd.exec(args);
            }
        }
    }

    return EXIT_SUCCESS;
}
