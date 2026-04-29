#pragma once
#include <windows.h>
#include <stdbool.h>
#include "shell.h"

void themeLoadDefault(Theme *t);
bool themeSetByName(Theme *t, const char *name);
void applyTheme(const Theme *t, HANDLE stdOut);
void themeList(void);
const char* getThemeForegroundAnsi(const Theme *t);


