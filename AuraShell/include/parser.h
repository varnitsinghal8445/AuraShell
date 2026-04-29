#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct Command {
    char **argv;
    int argc;
    bool newConsole;
} Command;

bool parseCommand(const char *line, Command *out);
void freeCommand(Command *cmd);


