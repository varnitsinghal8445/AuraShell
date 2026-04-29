#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <conio.h>
#include <string.h>
#include "shell.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif
#include "parser.h"
#include "commands.h"
#include "process.h"
#include "history.h"
#include "alias.h"
#include "theme.h"
#include "voice.h"

// Text selection state
typedef struct {
    int start;
    int end;
    bool active;
} TextSelection;

// Global selection state
static TextSelection selection = {0};

// Enable virtual terminal processing for colors
static void enableVirtualTerminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

// Print colored text
void printColor(const wchar_t* text, const wchar_t* color) {
    wprintf(L"%s%s\033[0m", color, text);
}

// Print welcome banner
void printWelcomeBanner() {
    printf("\n");
    printf("Welcome to AuraShell\n");
    printf("\n");
}

void runShell(ShellState *state) {
    wchar_t line[512];
    HistoryBuffer hb; 
    historyInit(&hb, L"."); 
    historyLoad(&hb);
    
    // Initialize aliases
    aliasInit(&state->aliases, L".");
    aliasLoad(&state->aliases);
    
    // Enable virtual terminal for colors
    enableVirtualTerminal();
    
    // Show welcome banner
    printWelcomeBanner();
    
    while (state->running) {
        // Print colored prompt using theme colors
        const char* themeColor = getThemeForegroundAnsi(&state->theme);
        wchar_t themeColorW[16];
        MultiByteToWideChar(CP_UTF8, 0, themeColor, -1, themeColorW, 16);
        printColor(L"AuraShell", themeColorW);
        printColor(L" > ", L"\033[1;37m");      // White text for prompt separator
        fflush(stdout);
        
        // Reset selection and history navigation
        selection.active = false;
        historyResetPosition(&hb);
        // Get input with basic line editing
        int pos = 0;
        int ch;
        while ((ch = _getwch()) != '\r' && ch != '\n') {
            if (ch == 0xE0 || ch == 0) { // Handle extended keys (arrows, etc.)
                ch = _getwch();
                if (ch == 0x48) { // Up arrow - previous command
                    const wchar_t* prevCmd = historyPrevious(&hb);
                    if (prevCmd) {
                        // Clear current line
                        for (int i = 0; i < pos; i++) fputws(L"\b \b", stdout);
                        // Print and store previous command
                        wcscpy_s(line, ARRAYSIZE(line), prevCmd);
                        pos = wcslen(line);
                        fputws(line, stdout);
                    }
                    continue;
                } else if (ch == 0x50) { // Down arrow - next command
                    const wchar_t* nextCmd = historyNext(&hb);
                    // Clear current line
                    for (int i = 0; i < pos; i++) fputws(L"\b \b", stdout);
                    if (nextCmd) {
                        wcscpy_s(line, ARRAYSIZE(line), nextCmd);
                        pos = wcslen(line);
                        fputws(line, stdout);
                    } else {
                        line[0] = L'\0';
                        pos = 0;
                    }
                    continue;
                } else if (ch == 0x4B) { // Left arrow
                    if (pos > 0) {
                        pos--;
                        fputwc('\b', stdout);
                    }
                    continue;
                } else if (ch == 0x4D) { // Right arrow
                    if (pos < wcslen(line)) {
                        fputwc(line[pos], stdout);
                        pos++;
                    }
                    continue;
                }
            } else if (ch == 0x03) { // Ctrl+C
                fputws(L"^C\n", stdout);
                line[0] = '\0';
                break;
            } else if (ch == 0x08 || ch == 0x7F) { // Backspace/Delete
                if (pos > 0) {
                    pos--;
                    fputws(L"\b \b", stdout); // Backspace, space, backspace
                    line[pos] = '\0';
                }
            } else if (ch == 0x16) { // Ctrl+V for paste
                if (OpenClipboard(NULL)) {
                    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
                    if (hData != NULL) {
                        wchar_t* pszText = (wchar_t*)GlobalLock(hData);
                        if (pszText != NULL) {
                            int i = 0;
                            while (pszText[i] && pos < 511) {
                                if (pszText[i] != '\r' && pszText[i] != '\n') {
                                    line[pos++] = pszText[i];
                                    fputwc(pszText[i], stdout);
                                }
                                i++;
                            }
                            line[pos] = '\0';
                            GlobalUnlock(hData);
                        }
                    }
                    CloseClipboard();
                }
            } else if (ch == 0x01) { // Ctrl+A for select all
                // Selection handling could be added here
            } else if (ch == 0x03) { // Ctrl+C for copy (when text is selected)
                // Copy to clipboard could be implemented here
            } else if (pos < 511) { // Regular character input
                line[pos++] = (wchar_t)ch;
                line[pos] = '\0';
                fputwc(ch, stdout);
            }
        }
        fputws(L"\n", stdout); // New line after input
        if (pos == 0) continue; // Skip empty lines
        if (wcscmp(line, L"\n") == 0 || wcscmp(line, L"\r\n") == 0) continue;
        // convert to UTF-8
        char u8[1024];
        int len = WideCharToMultiByte(CP_UTF8, 0, line, -1, u8, (int)sizeof(u8), NULL, NULL);
        if (!len) continue;
        // strip newline in UTF-8 buffer
        if (len>1 && (u8[len-2]=='\r' || u8[len-2]=='\n')) u8[len-2]='\0';
        if (len>1 && (u8[len-1]=='\r' || u8[len-1]=='\n')) u8[len-1]='\0';

        char expanded[1024];
        const char *execLine = u8;
        
        // First check for history expansion
        if (u8[0] == '!'){
            if (!historyExpand(&hb, u8, expanded, sizeof expanded)){
                fputs("History: invalid reference\n", stderr);
                continue;
            }
            execLine = expanded;
            puts(execLine);
        }
        
        // Then check for alias expansion
        char aliasExpanded[1024];
        if (aliasExpand(&state->aliases, execLine, aliasExpanded, sizeof aliasExpanded)) {
            execLine = aliasExpanded;
        }

        Command cmd={0};
        if (!parseCommand(execLine, &cmd)) continue;
        if (isBuiltin(&cmd)) {
            runBuiltin(state, &cmd);
        } else {
            ProcessResult res = spawnProcess(&cmd, false);
            if (!res.success) {
                fprintf(stderr, "Error: %s\n", res.errorMessage);
                // Speak error messages when voice is enabled
                if (state->voiceEnabled && voiceIsAvailable()) {
                    char errorMsg[512];
                    snprintf(errorMsg, sizeof(errorMsg), "Error: %s", res.errorMessage);
                    voiceSpeak(errorMsg);
                }
            }
        }
        historyAdd(&hb, execLine);
        freeCommand(&cmd);
    }
    historySave(&hb);
    aliasSave(&state->aliases);
    aliasFree(&state->aliases);
}


