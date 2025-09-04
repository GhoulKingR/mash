#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

ssize_t getcmdline(char** line, size_t* len) {
    printf("$ ");
    ssize_t read = getline(line, len, stdin);
    (*line)[read - 1] = '\0';
    return read;
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
            }

            continue;
        }


        collected_arg[collected_arg_size] = line[i];
        collected_arg_size++;
        escape = false;
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

int main(int argc, char** argv) {
    if (argc > 1) {
        printf("You ran this with some arguments\n");
    } else {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        while (getcmdline(&line, &len) != -1) {
            char** args = NULL;
            size_t cli_len = parseargs(line, &args);

            if (args != NULL)
                printf("First argument: %s\n", args[0]);

            cleanup_args(&args, cli_len);
        }

        free(line);
        return 0;
    }
}
