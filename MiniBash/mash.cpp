#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <deque>
#include <iostream>
#include <ostream>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

#define HISTORY_MAX_SIZE 500

struct Configs {
    
} configs;

class History {
    std::deque<std::string> items;
    size_t position = 0;

public:
    std::string get_current() {
        return items[position];
    }

    bool move_up() {
        if (position > 0) {
            position--;
            return true;
        }
        return false;
    }

    bool move_down() {
        if (items.size() > 0 && position < items.size() - 1) {
            position++;
            return true;
        }
        return false;
    }

    void add_item(std::string new_item) {
        items.push_back(new_item);
        while (items.size() > HISTORY_MAX_SIZE) {
            items.pop_front();
        }
        position = items.size();
    }
} history;

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

struct termios original, noecho;

void exit_handler() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

void cmd_exec(std::vector<std::string>& args) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
    if (args[0] == "exit") {
        exit(EXIT_SUCCESS);
    } else if (args[0] == "echo") {
        std::cout << args[1] << std::endl;
    } else {
        // unset the terminal configurations
        // look for the command in path
        // fork the process
        // execute the executable it has found
        // return to normal
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &noecho);
}

int main(int argc, char** argv) {
    signal(SIGINT, exit);
    int result = atexit(exit_handler);

    if (result != 0) {
        printf("Could not register exit handler!");
        exit(EXIT_FAILURE);
    }
    
    tcgetattr(STDIN_FILENO, &original);
    noecho = original;
    noecho.c_lflag &= ~(ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &noecho);

    if (argc > 1) {
        printf("You ran this with some arguments\n");
    } else {
        size_t len = 0;
        ssize_t read;

        while (true) {
            std::string line = getcmdline();
            if (line.size() == 0)
                printf("\n");

            std::vector<std::string> args = parseargs(line);

            if (!args.empty()) {
                cmd_exec(args);
            }
        }
    }

    return EXIT_SUCCESS;
}
