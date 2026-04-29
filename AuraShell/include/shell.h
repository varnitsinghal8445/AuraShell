#pragma once
#include <windows.h>
#include <stdbool.h>
#include <stddef.h>
#include "alias.h"

typedef struct Theme {
    WORD foregroundAttr;
    WORD backgroundAttr;
    wchar_t windowTitle[128];
} Theme;

typedef struct HistoryBuffer {
    char **entries;
    size_t size;
    size_t capacity;
    wchar_t filePath[MAX_PATH];
} HistoryBuffer;

typedef struct ShellState {
    HANDLE stdIn;
    HANDLE stdOut;
    HANDLE stdErr;
    HistoryBuffer history;
    AliasBuffer aliases;
    Theme theme;
    wchar_t workingDirectory[MAX_PATH];
    wchar_t appDataDir[MAX_PATH];
    bool running;
    bool voiceEnabled;
    bool voiceRecognitionEnabled;
} ShellState;

void runShell(ShellState *state);
void printWelcomeBanner(void);


