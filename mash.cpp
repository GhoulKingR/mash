#include <csignal>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <ctype.h>
#include <deque>
#include <sstream>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

#define PREFIX "\e[2K\r$ %s"
#define prefix_withb(line, backspace) printf(PREFIX "\e[%dD", line, backspace);
#define HISTORY_MAX_SIZE 500

class History {
    std::deque<std::string> items;
    int position = 0;

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

std::string getcmdline() {
    std::string line = "";

    printf(PREFIX, "");
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
                    for (int i = text_size - backspace - 1; i < text_size - 1; i++) {
                        char t = line[i];
                        line[i] = line[i + 1];
                        line[i + 1] = t;
                    }
                }

                line.pop_back();

                if (backspace == 0) {
                    printf(PREFIX, line.c_str());
                } else {
                    prefix_withb(line.c_str(), backspace);
                }
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
                        printf(PREFIX, line.c_str());
                    }
                } else if (c2 == 'B') { // down
                    if (history.move_down()) {
                        line = history.get_current();
                        backspace = 0;
                        printf(PREFIX, line.c_str());
                    }
                } else if (c2 == 'C') { // right
                    if (backspace > 0)
                        backspace--;

                    if (backspace > 0) {
                        prefix_withb(line.c_str(), backspace);
                    } else {
                        printf(PREFIX, line.c_str());
                    }
                } else if (c2 == 'D') { // left
                    if (backspace < line.size())
                        backspace++;
                    prefix_withb(line.c_str(), backspace);
                }
                continue;
            }
        }

        // change the add text logic
        if (backspace > 0 && isprint(c)) {
            char tmp = c;
            size_t text_size = line.size();
            line += ' ';
            for (int i = text_size - backspace; i <= text_size; i++) {
                char t = line[i];
                line[i] = tmp;
                tmp = t;
            }
            prefix_withb(line.c_str(), backspace)
        } else if (isprint(c)) {
            line += c;
            printf(PREFIX, line.c_str());
        }
    }

    history.add_item(line);
    return line;
}


std::vector<std::string> parseargs(std::string line) {
    std::vector<std::string> args;
    std::stringstream liness(line);
    std::string tmp;

    while (std::getline(liness, tmp, ' ')) {
        if (tmp.size() > 0) {
            args.push_back(tmp);
        }
    }

    return args;
}

struct termios original, noecho;

void exit_handler() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
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

            if (!args.empty())
                printf("First argument: %s\n", args[0].c_str());
        }
    }

    return EXIT_SUCCESS;
}
