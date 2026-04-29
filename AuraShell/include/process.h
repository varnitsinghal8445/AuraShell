#pragma once
#include <windows.h>
#include <stdbool.h>
#include "parser.h"

typedef struct ProcessResult {
    bool success;
    DWORD exitCode;
    char errorMessage[256];
} ProcessResult;

void buildWindowsCmdLine(const Command *cmd, wchar_t *out, size_t outCap);
ProcessResult spawnProcess(const Command *cmd, bool newConsole);


