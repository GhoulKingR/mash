#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

#define PREFIX "\e[2K\r$ %s"
#define prefix_withb(line, backspace) printf(PREFIX "\e[%dD", line, backspace);

// TODO: Add support for up and down arrow keys for history

size_t getcmdline(char** result) {
    size_t line_size = 10;
    size_t text_size = 0;
    char* line = malloc(line_size * sizeof(char));
    memset(line, '\0', line_size * sizeof(char));

    printf(PREFIX, "");
    int backspace = 0;

    while(true) {
        if ((text_size + 1) == line_size) {
            line_size *= 2;
            line = realloc(line, line_size * sizeof(char));
        }

        char c = getchar();
        if (c == '\n') {
            printf("\n");
            break;
        } else if (c == '\x04' || c == '\x03') {
            free(line);
            return -1;
        } else if (c == '\b' || c == '\x7f') {
            if (text_size > 0) {
                if (backspace > 0) {
                    for (int i = text_size - backspace - 1; i <= text_size; i++) {
                        char t = line[i];
                        line[i] = line[i + 1];
                        line[i + 1] = t;
                    }
                }
                line[text_size - 1] = '\0';
                text_size--;

                if (backspace == 0) {
                    printf(PREFIX, line);
                } else {
                    prefix_withb(line, backspace);
                }
                continue;
            }
        } else if (c == '\e') {
            char c1 = getchar();
            char c2 = getchar();
            if (c1 == '[') {
                if (c2 == 'A') {
                    // up
                } else if (c2 == 'B') {
                    // down
                } else if (c2 == 'C') {
                    // right
                    if (backspace > 0)
                        backspace--;

                    if (backspace > 0) {
                        prefix_withb(line, backspace);
                    } else {
                        printf(PREFIX, line);
                    }
                } else if (c2 == 'D') {
                    // left
                    if (backspace < text_size)
                        backspace++;
                    prefix_withb(line, backspace);
                }
                continue;
            }
        }

        // change the add text logic
        if (backspace > 0) {
            char tmp = c;
            for (int i = text_size - backspace; i <= text_size; i++) {
                char t = line[i];
                line[i] = tmp;
                tmp = t;
            }
            line[text_size + 1] = '\0';
            text_size++;
            prefix_withb(line, backspace)
        } else {
            line[text_size] = c;
            line[text_size + 1] = '\0';
            text_size++;
            printf(PREFIX, line);
        }
    }

    *result = realloc(line, text_size * sizeof(char));
    return text_size;
}


size_t parseargs(char* line, char*** args) {
    size_t len = strlen(line);
    size_t allocated_size = 5;
    *args = malloc(allocated_size * sizeof(char*));
    size_t added = 0;

    size_t allocated_text_size = 10;
    char* collected_arg = malloc(allocated_text_size * sizeof(char));
    size_t collected_arg_size = 0;
    memset(collected_arg, '\0', allocated_text_size);

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

        if (added == allocated_size) {
            allocated_size *= 2;
            *args = realloc(*args, allocated_size * sizeof(char*));
        }

        if (collected_arg_size == allocated_text_size) {
            allocated_text_size *= 2;
            collected_arg = realloc(collected_arg, allocated_text_size * sizeof(char));
        }

        if ((line[i] == ' ' && !escape && !quotes) || line[i] == '\0') {
            if (collected_arg_size > 0) {
                collected_arg[collected_arg_size] = '\0';
                collected_arg_size++;
                (*args)[added] = realloc(collected_arg, collected_arg_size * sizeof(char));
                added++;

                if (line[i] == ' ') {
                    allocated_text_size = 10;
                    collected_arg = malloc(allocated_text_size * sizeof(char));
                    collected_arg_size = 0;
                    memset(collected_arg, '\0', allocated_text_size);
                }
            } else if (line[i] == '\0') {
                free(collected_arg);
            }

            continue;
        }

        if (escape) {
            switch (line[i]) {
                case 'n':
                    collected_arg[collected_arg_size] = '\n';
                    collected_arg_size++;
                    break;

                case 'b':
                    collected_arg[collected_arg_size] = '\b';
                    collected_arg_size++;
                    break;

                default:
                    collected_arg[collected_arg_size] = line[i];
                    collected_arg_size++;
                    break;
            }
            escape = false;
        } else {
            collected_arg[collected_arg_size] = line[i];
            collected_arg_size++;
        }
    }


    if (added > 0) {
        *args = realloc(*args, added * sizeof(char*));
    } else {
        free(*args);
        *args = NULL;
    }

    return added;
}


void cleanup_args(char*** args, size_t count) {
    if (*args != NULL) {
        for (int i = 0; i < count; i++) {
            free((*args)[i]);
        }
        free(*args);
        *args = NULL;
    }
}

char *line = NULL;

struct termios original, noecho;

void exit_handler() {
    printf("\n");
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
    if (line != NULL)
        free(line);
}

int main(int argc, char** argv) {
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

        while (getcmdline(&line) != -1) {
            char** args = NULL;
            size_t cli_len = parseargs(line, &args);

            if (args != NULL)
                printf("First argument: %s\n", args[0]);

            cleanup_args(&args, cli_len);
        }
    }

    return EXIT_SUCCESS;
}
