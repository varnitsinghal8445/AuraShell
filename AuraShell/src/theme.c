#include <string.h>
#include <wchar.h>
#include <windows.h>
#include "theme.h"

typedef struct NamedTheme { const char *name; WORD fg; WORD bg; const wchar_t *title; } NamedTheme;

static const NamedTheme THEMES[] = {
    { "default", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, 0, L"AuraShell" },
    { "light", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE, BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY, L"AuraShell (Light)" },
    { "dark", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE, L"AuraShell (Dark)" },
    { "cyan", FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, 0, L"AuraShell (Cyan)" },
    { "cyan-bg", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY, L"AuraShell (Cyan BG)" },
    { "yellow", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY, 0, L"AuraShell (Yellow)" },
    { "yellow-bg", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_INTENSITY, L"AuraShell (Yellow BG)" },
    { "green", FOREGROUND_GREEN|FOREGROUND_INTENSITY, 0, L"AuraShell (Green)" },
    { "green-bg", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, BACKGROUND_GREEN|BACKGROUND_INTENSITY, L"AuraShell (Green BG)" },
    { "red", FOREGROUND_RED|FOREGROUND_INTENSITY, 0, L"AuraShell (Red)" },
    { "red-bg", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, BACKGROUND_RED|BACKGROUND_INTENSITY, L"AuraShell (Red BG)" },
    { "blue", FOREGROUND_BLUE|FOREGROUND_INTENSITY, 0, L"AuraShell (Blue)" },
    { "blue-bg", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, BACKGROUND_BLUE|BACKGROUND_INTENSITY, L"AuraShell (Blue BG)" },
    { "purple", FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY, 0, L"AuraShell (Purple)" },
    { "purple-bg", FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY, BACKGROUND_RED|BACKGROUND_BLUE|BACKGROUND_INTENSITY, L"AuraShell (Purple BG)" },
    { "terminal", FOREGROUND_GREEN|FOREGROUND_INTENSITY, 0, L"AuraShell (Terminal)" },
    { "matrix", FOREGROUND_GREEN, 0, L"AuraShell (Matrix)" }
};

void themeLoadDefault(Theme *t){
    t->foregroundAttr = THEMES[0].fg;
    t->backgroundAttr = THEMES[0].bg;
    wcscpy_s(t->windowTitle, 128, THEMES[0].title);
}

bool themeSetByName(Theme *t, const char *name){
    for (size_t i=0;i<sizeof(THEMES)/sizeof(THEMES[0]);++i){
        if (_stricmp(THEMES[i].name, name)==0){
            t->foregroundAttr = THEMES[i].fg;
            t->backgroundAttr = THEMES[i].bg;
            wcscpy_s(t->windowTitle, 128, THEMES[i].title);
            return true;
        }
    }
    return false;
}

void applyTheme(const Theme *t, HANDLE stdOut){
    // Check if virtual terminal processing is enabled
    DWORD mode = 0;
    GetConsoleMode(stdOut, &mode);
    
    if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {
        // Use ANSI escape codes when virtual terminal processing is enabled
        printf("\033[0m"); // Reset all attributes first
        
        // Apply background color if present
        if (t->backgroundAttr != 0) {
            if (t->backgroundAttr & BACKGROUND_RED) printf("\033[41m");
            if (t->backgroundAttr & BACKGROUND_GREEN) printf("\033[42m");
            if (t->backgroundAttr & BACKGROUND_BLUE) printf("\033[44m");
            if (t->backgroundAttr & BACKGROUND_INTENSITY) {
                // For bright backgrounds, use the 100-107 range
                if (t->backgroundAttr & BACKGROUND_RED && t->backgroundAttr & BACKGROUND_GREEN && t->backgroundAttr & BACKGROUND_BLUE) {
                    printf("\033[107m"); // Bright white background
                } else if (t->backgroundAttr & BACKGROUND_RED && t->backgroundAttr & BACKGROUND_GREEN) {
                    printf("\033[103m"); // Bright yellow background
                } else if (t->backgroundAttr & BACKGROUND_GREEN && t->backgroundAttr & BACKGROUND_BLUE) {
                    printf("\033[106m"); // Bright cyan background
                } else if (t->backgroundAttr & BACKGROUND_RED && t->backgroundAttr & BACKGROUND_BLUE) {
                    printf("\033[105m"); // Bright magenta background
                } else if (t->backgroundAttr & BACKGROUND_RED) {
                    printf("\033[101m"); // Bright red background
                } else if (t->backgroundAttr & BACKGROUND_GREEN) {
                    printf("\033[102m"); // Bright green background
                } else if (t->backgroundAttr & BACKGROUND_BLUE) {
                    printf("\033[104m"); // Bright blue background
                }
            }
        }
        
        // Apply foreground color
        if (t->foregroundAttr & FOREGROUND_RED && t->foregroundAttr & FOREGROUND_GREEN && t->foregroundAttr & FOREGROUND_BLUE) {
            if (t->foregroundAttr & FOREGROUND_INTENSITY) {
                printf("\033[97m"); // Bright white
            } else {
                printf("\033[37m"); // White
            }
        } else if (t->foregroundAttr & FOREGROUND_RED && t->foregroundAttr & FOREGROUND_GREEN) {
            if (t->foregroundAttr & FOREGROUND_INTENSITY) {
                printf("\033[93m"); // Bright yellow
            } else {
                printf("\033[33m"); // Yellow
            }
        } else if (t->foregroundAttr & FOREGROUND_GREEN && t->foregroundAttr & FOREGROUND_BLUE) {
            if (t->foregroundAttr & FOREGROUND_INTENSITY) {
                printf("\033[96m"); // Bright cyan
            } else {
                printf("\033[36m"); // Cyan
            }
        } else if (t->foregroundAttr & FOREGROUND_RED && t->foregroundAttr & FOREGROUND_BLUE) {
            if (t->foregroundAttr & FOREGROUND_INTENSITY) {
                printf("\033[95m"); // Bright magenta
            } else {
                printf("\033[35m"); // Magenta
            }
        } else if (t->foregroundAttr & FOREGROUND_RED) {
            if (t->foregroundAttr & FOREGROUND_INTENSITY) {
                printf("\033[91m"); // Bright red
            } else {
                printf("\033[31m"); // Red
            }
        } else if (t->foregroundAttr & FOREGROUND_GREEN) {
            if (t->foregroundAttr & FOREGROUND_INTENSITY) {
                printf("\033[92m"); // Bright green
            } else {
                printf("\033[32m"); // Green
            }
        } else if (t->foregroundAttr & FOREGROUND_BLUE) {
            if (t->foregroundAttr & FOREGROUND_INTENSITY) {
                printf("\033[94m"); // Bright blue
            } else {
                printf("\033[34m"); // Blue
            }
        }
    } else {
        // Use Windows Console API when virtual terminal processing is not enabled
        SetConsoleTextAttribute(stdOut, t->foregroundAttr | t->backgroundAttr);
    }
    
    if (t->windowTitle[0]) SetConsoleTitleW(t->windowTitle);
}

// Convert theme foreground color to ANSI escape code
const char* getThemeForegroundAnsi(const Theme *t) {
    if (t->foregroundAttr & FOREGROUND_RED && t->foregroundAttr & FOREGROUND_GREEN && t->foregroundAttr & FOREGROUND_BLUE) {
        if (t->foregroundAttr & FOREGROUND_INTENSITY) {
            return "\033[97m"; // Bright white
        } else {
            return "\033[37m"; // White
        }
    } else if (t->foregroundAttr & FOREGROUND_RED && t->foregroundAttr & FOREGROUND_GREEN) {
        if (t->foregroundAttr & FOREGROUND_INTENSITY) {
            return "\033[93m"; // Bright yellow
        } else {
            return "\033[33m"; // Yellow
        }
    } else if (t->foregroundAttr & FOREGROUND_GREEN && t->foregroundAttr & FOREGROUND_BLUE) {
        if (t->foregroundAttr & FOREGROUND_INTENSITY) {
            return "\033[96m"; // Bright cyan
        } else {
            return "\033[36m"; // Cyan
        }
    } else if (t->foregroundAttr & FOREGROUND_RED && t->foregroundAttr & FOREGROUND_BLUE) {
        if (t->foregroundAttr & FOREGROUND_INTENSITY) {
            return "\033[95m"; // Bright magenta
        } else {
            return "\033[35m"; // Magenta
        }
    } else if (t->foregroundAttr & FOREGROUND_RED) {
        if (t->foregroundAttr & FOREGROUND_INTENSITY) {
            return "\033[91m"; // Bright red
        } else {
            return "\033[31m"; // Red
        }
    } else if (t->foregroundAttr & FOREGROUND_GREEN) {
        if (t->foregroundAttr & FOREGROUND_INTENSITY) {
            return "\033[92m"; // Bright green
        } else {
            return "\033[32m"; // Green
        }
    } else if (t->foregroundAttr & FOREGROUND_BLUE) {
        if (t->foregroundAttr & FOREGROUND_INTENSITY) {
            return "\033[94m"; // Bright blue
        } else {
            return "\033[34m"; // Blue
        }
    }
    return "\033[37m"; // Default to white
}

void themeList(void){
    printf("Available Themes:\n");
    printf("================\n");
    for (size_t i=0;i<sizeof(THEMES)/sizeof(THEMES[0]);++i){
        const char* bgInfo = (THEMES[i].bg == 0) ? " (no background)" : " (with background)";
        printf("  %s%s\n", THEMES[i].name, bgInfo);
    }
    printf("\nBackground Color Themes:\n");
    printf("=======================\n");
    for (size_t i=0;i<sizeof(THEMES)/sizeof(THEMES[0]);++i){
        if (THEMES[i].bg != 0) {
            printf("  %s\n", THEMES[i].name);
        }
    }
    printf("\nForeground Only Themes:\n");
    printf("======================\n");
    for (size_t i=0;i<sizeof(THEMES)/sizeof(THEMES[0]);++i){
        if (THEMES[i].bg == 0) {
            printf("  %s\n", THEMES[i].name);
        }
    }
}


