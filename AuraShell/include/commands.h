#pragma once
#include <stdbool.h>
#include "parser.h"
#include "shell.h"

bool isBuiltin(const Command *cmd);
bool runBuiltin(ShellState *state, const Command *cmd);


