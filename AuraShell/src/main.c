#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <shlobj.h>
#include "shell.h"
#include "theme.h"
#include "voice.h"

int wmain(void) {
    ShellState state = {0};
    state.stdIn  = GetStdHandle(STD_INPUT_HANDLE);
    state.stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    state.stdErr = GetStdHandle(STD_ERROR_HANDLE);
    SetConsoleTitleW(L"AuraShell");
    themeLoadDefault(&state.theme);
    applyTheme(&state.theme, state.stdOut);
    
    // Change to Desktop directory on startup
    wchar_t desktopPath[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, desktopPath) == S_OK) {
        SetCurrentDirectoryW(desktopPath);
    }
    
    state.running = true;
    state.voiceEnabled = false;
    state.voiceRecognitionEnabled = false;
    runShell(&state);
    voiceRecognitionCleanup();
    voiceCleanup();
    return 0;
}


