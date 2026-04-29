#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include "commands.h"
#include "theme.h"
#include "history.h"
#include "alias.h"
#include "voice.h"
#include "parser.h"
#include "process.h"

// Helper function to speak command results when voice is enabled
static void speakResult(ShellState *state, const char *message) {
    if (state->voiceEnabled && voiceIsAvailable() && message) {
        voiceSpeak(message);
    }
}

// Convert natural language spoken input to valid terminal commands
static void normalizeVoiceCommand(char *input, size_t inputSize) {
    if (!input || inputSize == 0) return;
    
    // Convert to lowercase for easier matching
    char lower[512] = {0};
    strncpy_s(lower, sizeof(lower), input, _TRUNCATE);
    for (size_t i = 0; i < strlen(lower); i++) {
        lower[i] = (char)tolower((unsigned char)lower[i]);
    }
    
    // Common natural language to command mappings
    // "open [app]" -> "open [app]"
    if (strncmp(lower, "open ", 5) == 0) {
        // Already correct format
        return;
    }
    
    // "launch [app]" -> "open [app]"
    if (strncmp(lower, "launch ", 7) == 0) {
        char app[256] = {0};
        strncpy_s(app, sizeof(app), input + 7, _TRUNCATE);
        snprintf(input, inputSize, "open %s", app);
        return;
    }
    
    // "start [app]" -> "open [app]"
    if (strncmp(lower, "start ", 6) == 0) {
        char app[256] = {0};
        strncpy_s(app, sizeof(app), input + 6, _TRUNCATE);
        snprintf(input, inputSize, "open %s", app);
        return;
    }
    
    // "run [app]" -> "open [app]"
    if (strncmp(lower, "run ", 4) == 0) {
        char app[256] = {0};
        strncpy_s(app, sizeof(app), input + 4, _TRUNCATE);
        snprintf(input, inputSize, "open %s", app);
        return;
    }
    
    // "show directory" or "list files" -> "dir"
    if (strcmp(lower, "show directory") == 0 || strcmp(lower, "list files") == 0 || 
        strcmp(lower, "list directory") == 0 || strcmp(lower, "show files") == 0) {
        strncpy_s(input, inputSize, "dir", _TRUNCATE);
        return;
    }
    
    // "current directory" or "where am i" -> "pwd"
    if (strcmp(lower, "current directory") == 0 || strcmp(lower, "where am i") == 0 ||
        strcmp(lower, "show current directory") == 0 || strcmp(lower, "print working directory") == 0) {
        strncpy_s(input, inputSize, "pwd", _TRUNCATE);
        return;
    }
    
    // "change directory to [path]" or "go to [path]" -> "cd [path]"
    if (strncmp(lower, "change directory to ", 20) == 0) {
        char path[256] = {0};
        strncpy_s(path, sizeof(path), input + 20, _TRUNCATE);
        snprintf(input, inputSize, "cd %s", path);
        return;
    }
    if (strncmp(lower, "go to ", 6) == 0) {
        char path[256] = {0};
        strncpy_s(path, sizeof(path), input + 6, _TRUNCATE);
        snprintf(input, inputSize, "cd %s", path);
        return;
    }
    if (strncmp(lower, "cd ", 3) == 0) {
        // Already correct
        return;
    }
    
    // "create folder [name]" or "make directory [name]" -> "mkdir [name]"
    if (strncmp(lower, "create folder ", 14) == 0) {
        char name[256] = {0};
        strncpy_s(name, sizeof(name), input + 14, _TRUNCATE);
        snprintf(input, inputSize, "mkdir %s", name);
        return;
    }
    if (strncmp(lower, "make directory ", 15) == 0) {
        char name[256] = {0};
        strncpy_s(name, sizeof(name), input + 15, _TRUNCATE);
        snprintf(input, inputSize, "mkdir %s", name);
        return;
    }
    
    // "delete [file]" or "remove [file]" -> "del [file]"
    if (strncmp(lower, "delete ", 7) == 0) {
        char file[256] = {0};
        strncpy_s(file, sizeof(file), input + 7, _TRUNCATE);
        snprintf(input, inputSize, "del %s", file);
        return;
    }
    if (strncmp(lower, "remove ", 7) == 0) {
        char file[256] = {0};
        strncpy_s(file, sizeof(file), input + 7, _TRUNCATE);
        snprintf(input, inputSize, "del %s", file);
        return;
    }
    
    // "copy [src] to [dst]" -> "copy [src] [dst]"
    if (strstr(lower, " copy ") != NULL && strstr(lower, " to ") != NULL) {
        // Try to parse "X copy Y to Z" or "copy Y to Z"
        char *copyPos = strstr(lower, "copy ");
        char *toPos = strstr(lower, " to ");
        if (copyPos && toPos && toPos > copyPos) {
            char src[256] = {0};
            char dst[256] = {0};
            size_t srcStart = (copyPos - lower) + 5;
            size_t srcLen = (toPos - lower) - srcStart;
            size_t dstStart = (toPos - lower) + 4;
            
            strncpy_s(src, sizeof(src), input + srcStart, srcLen);
            strncpy_s(dst, sizeof(dst), input + dstStart, _TRUNCATE);
            snprintf(input, inputSize, "copy %s %s", src, dst);
            return;
        }
    }
    
    // "move [src] to [dst]" -> "move [src] [dst]"
    if (strstr(lower, " move ") != NULL && strstr(lower, " to ") != NULL) {
        char *movePos = strstr(lower, "move ");
        char *toPos = strstr(lower, " to ");
        if (movePos && toPos && toPos > movePos) {
            char src[256] = {0};
            char dst[256] = {0};
            size_t srcStart = (movePos - lower) + 5;
            size_t srcLen = (toPos - lower) - srcStart;
            size_t dstStart = (toPos - lower) + 4;
            
            strncpy_s(src, sizeof(src), input + srcStart, srcLen);
            strncpy_s(dst, sizeof(dst), input + dstStart, _TRUNCATE);
            snprintf(input, inputSize, "move %s %s", src, dst);
            return;
        }
    }
    
    // "show [file]" or "display [file]" -> "type [file]"
    if (strncmp(lower, "show ", 5) == 0) {
        char file[256] = {0};
        strncpy_s(file, sizeof(file), input + 5, _TRUNCATE);
        // Check if it's a file (has extension) or directory command
        if (strstr(file, ".") != NULL || strlen(file) < 50) {
            snprintf(input, inputSize, "type %s", file);
            return;
        }
    }
    if (strncmp(lower, "display ", 8) == 0) {
        char file[256] = {0};
        strncpy_s(file, sizeof(file), input + 8, _TRUNCATE);
        snprintf(input, inputSize, "type %s", file);
        return;
    }
    
    // "say [text]" -> "echo [text]"
    if (strncmp(lower, "say ", 4) == 0) {
        char text[256] = {0};
        strncpy_s(text, sizeof(text), input + 4, _TRUNCATE);
        snprintf(input, inputSize, "echo %s", text);
        return;
    }
    
    // "print [text]" -> "echo [text]"
    if (strncmp(lower, "print ", 6) == 0) {
        char text[256] = {0};
        strncpy_s(text, sizeof(text), input + 6, _TRUNCATE);
        snprintf(input, inputSize, "echo %s", text);
        return;
    }
    
    // Common app name mappings
    // "chrome" -> "open chrome"
    if (strcmp(lower, "chrome") == 0) {
        strncpy_s(input, inputSize, "open chrome", _TRUNCATE);
        return;
    }
    
    // "notepad" -> "open notepad"
    if (strcmp(lower, "notepad") == 0) {
        strncpy_s(input, inputSize, "open notepad", _TRUNCATE);
        return;
    }
    
    // "calculator" or "calc" -> "open calculator"
    if (strcmp(lower, "calculator") == 0 || strcmp(lower, "calc") == 0) {
        strncpy_s(input, inputSize, "open calculator", _TRUNCATE);
        return;
    }
    
    // If no match, return as-is (might already be a valid command)
}

static bool cmd_exit(ShellState *state, const Command *cmd){ 
    speakResult(state, "Exiting AuraShell");
    state->running=false; 
    return true; 
}

static bool cmd_cd(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) { fputs("Usage: cd <path>\n", stderr); return false; }
    
    wchar_t wpath[MAX_PATH];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[1], -1, wpath, MAX_PATH);
    if (!wlen) { 
        // Try direct conversion if UTF-8 conversion fails
        wlen = MultiByteToWideChar(CP_ACP, 0, cmd->argv[1], -1, wpath, MAX_PATH);
        if (!wlen) {
            fputs("cd: invalid path encoding\n", stderr); 
            return false; 
        }
    }
    
    // Try to change directory
    if (!SetCurrentDirectoryW(wpath)) { 
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            fputs("cd: directory not found\n", stderr);
        } else if (error == ERROR_ACCESS_DENIED) {
            fputs("cd: access denied\n", stderr);
        } else if (error == ERROR_PATH_NOT_FOUND) {
            fputs("cd: path not found\n", stderr);
        } else {
            char errorMsg[256];
            sprintf_s(errorMsg, sizeof(errorMsg), "cd: failed (error %lu)\n", error);
            fputs(errorMsg, stderr);
        }
        if (state->voiceEnabled) speakResult(state, "Directory change failed");
        return false; 
    }
    if (state->voiceEnabled) speakResult(state, "Directory changed");
    return true;
}

static bool cmd_help(ShellState *state, const Command *cmd){
    fputs("========================================\n", stdout);
    fputs("           AuraShell Help System\n", stdout);
    fputs("========================================\n", stdout);
    fputs("\nWelcome to AuraShell! 🚀\n", stdout);
    fputs("A modern Windows shell with enhanced features and beautiful interface.\n", stdout);
    fputs("\nAvailable Commands:\n", stdout);
    fputs("  exit, cd, help, history, open, theme, newwin, alias, welcome, search, explain\n", stdout);
    fputs("  mkdir, rmdir, dir, pwd, copy, move, del, type, echo\n", stdout);
    
    fputs("\nDirectory Commands:\n", stdout);
    fputs("  cd <path>     - Change directory\n", stdout);
    fputs("  pwd           - Print working directory\n", stdout);
    fputs("  mkdir <name>  - Create directory\n", stdout);
    fputs("  rmdir <name>  - Remove directory\n", stdout);
    fputs("  dir           - List directory contents\n", stdout);
    
    fputs("\nFile Operations:\n", stdout);
    fputs("  copy <src> <dst>  - Copy file (also: cp)\n", stdout);
    fputs("  move <src> <dst>  - Move/rename file (also: mv)\n", stdout);
    fputs("  del <file>        - Delete file (also: rm)\n", stdout);
    fputs("  type <file>       - Display file contents (also: cat)\n", stdout);
    fputs("  echo <text>       - Print text or write to file\n", stdout);
    fputs("  echo <text> > <file> - Write text to file\n", stdout);
    
    fputs("\nSystem Commands:\n", stdout);
    fputs("  help          - Show this help\n", stdout);
    fputs("  exit          - Exit shell\n", stdout);
    fputs("  open <path>   - Open file/folder\n", stdout);
    fputs("  theme list    - Show available themes (including background colors)\n", stdout);
    fputs("  theme set <name> - Change theme (try: dark, cyan-bg, green-bg, etc.)\n", stdout);
    fputs("  newwin        - Open new window\n", stdout);
    fputs("  history       - Show command history\n", stdout);
    fputs("  welcome       - Show welcome banner again\n", stdout);
    fputs("  explain <cmd> - Get detailed explanation of a command\n", stdout);
    fputs("  voice <on|off|listen|status> - Voice output and recognition\n", stdout);
    fputs("    voice on     - Enable text-to-speech output\n", stdout);
    fputs("    voice listen - Listen for voice command (speak your command)\n", stdout);
    
    fputs("\nSearch Commands:\n", stdout);
    fputs("  search <pattern> [options] - Search for files and directories\n", stdout);
    fputs("    Options:\n", stdout);
    fputs("      -d <dir>     - Search in specific directory (default: current)\n", stdout);
    fputs("      -f           - Search files only\n", stdout);
    fputs("      -dir         - Search directories only\n", stdout);
    fputs("      -r           - Recursive search (search subdirectories)\n", stdout);
    fputs("      -i           - Case insensitive search\n", stdout);
    fputs("    Examples:\n", stdout);
    fputs("      search *.txt           - Find all .txt files in current directory\n", stdout);
    fputs("      search test -r         - Find files/dirs named 'test' recursively\n", stdout);
    fputs("      search *.exe -f -d C:\\ - Find .exe files in C:\\ drive\n", stdout);
    
    fputs("\nNEW: Dynamic Alias Management:\n", stdout);
    fputs("  alias                    - List all aliases\n", stdout);
    fputs("  alias add <name> <cmd>    - Create new alias\n", stdout);
    fputs("  alias set <name> <cmd>    - Create/update alias\n", stdout);
    fputs("  alias remove <name>       - Remove alias\n", stdout);
    fputs("  alias help                - Show alias help\n", stdout);
    
    fputs("\nSpecial Features:\n", stdout);
    fputs("  • Beautiful welcome banner on startup\n", stdout);
    fputs("  • Colored terminal output with themes\n", stdout);
    fputs("  • Command history with arrow key navigation\n", stdout);
    fputs("  • Dynamic alias creation and management\n", stdout);
    fputs("  • Cross-platform command compatibility\n", stdout);
    fputs("  • Persistent settings and history\n", stdout);
    fputs("  • Built-in command explanations with 'explain' command\n", stdout);
    fputs("  • Text-to-speech voice output (use 'voice on' to enable)\n", stdout);
    
    fputs("\nExamples:\n", stdout);
    fputs("  echo Hello World > greeting.txt\n", stdout);
    fputs("  copy file.txt backup.txt\n", stdout);
    fputs("  move old.txt new.txt\n", stdout);
    fputs("  type config.txt\n", stdout);
    fputs("  theme set cyan\n", stdout);
    fputs("  theme set dark\n", stdout);
    fputs("  theme set cyan-bg\n", stdout);
    fputs("  alias add ll \"dir\"\n", stdout);
    fputs("  alias add grep \"findstr\"\n", stdout);
    fputs("  explain dir\n", stdout);
    fputs("  explain copy\n", stdout);
    fputs("========================================\n", stdout);
    return true;
}

static bool cmd_history(ShellState *state, const Command *cmd){
    int n = 0;
    if (cmd->argc >= 2) n = atoi(cmd->argv[1]);
    // For now, print cannot access shell's hb; this will be routed later.
    fputs("history listing is shown from current session only in this minimal build.\n", stdout);
    return true;
}

static bool cmd_open(ShellState *state, const Command *cmd){
    if (cmd->argc < 2){ fputs("Usage: open <url-or-path>\n", stderr); return false; }
    wchar_t target[2048];
    if (!MultiByteToWideChar(CP_UTF8, 0, cmd->argv[1], -1, target, 2048)){
        fputs("open: invalid string\n", stderr); return false;
    }
    HINSTANCE r = ShellExecuteW(NULL, L"open", target, NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)r <= 32){ fputs("open: failed\n", stderr); return false; }
    return true;
}

static bool cmd_theme(ShellState *state, const Command *cmd){
    if (cmd->argc < 2){ fputs("Usage: theme <list|set NAME>\n", stdout); return true; }
    if (strcmp(cmd->argv[1], "list")==0){ themeList(); return true; }
    if (strcmp(cmd->argv[1], "set")==0){
        if (cmd->argc < 3){ fputs("Usage: theme set <name>\n", stderr); return false; }
        if (!themeSetByName(&state->theme, cmd->argv[2])){ fputs("Unknown theme\n", stderr); return false; }
        applyTheme(&state->theme, state->stdOut);
        return true;
    }
    fputs("Usage: theme <list|set NAME>\n", stdout);
    return true;
}

static bool cmd_newwin(ShellState *state, const Command *cmd){
    wchar_t exePath[MAX_PATH];
    if (!GetModuleFileNameW(NULL, exePath, MAX_PATH)) return false;
    STARTUPINFOW si; ZeroMemory(&si, sizeof si); si.cb = sizeof si;
    PROCESS_INFORMATION pi; ZeroMemory(&pi, sizeof pi);
    BOOL ok = CreateProcessW(exePath, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (!ok){ fputs("newwin: failed\n", stderr); return false; }
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    return true;
}

static bool cmd_mkdir(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) { fputs("Usage: mkdir <directory_name>\n", stderr); return false; }
    wchar_t wpath[MAX_PATH];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[1], -1, wpath, MAX_PATH);
    if (!wlen) { fputs("mkdir: invalid path encoding\n", stderr); return false; }
    if (!CreateDirectoryW(wpath, NULL)) {
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            fputs("mkdir: directory already exists\n", stderr);
            if (state->voiceEnabled) speakResult(state, "Directory already exists");
        } else {
            fputs("mkdir: failed to create directory\n", stderr);
            if (state->voiceEnabled) speakResult(state, "Failed to create directory");
        }
        return false;
    }
    if (state->voiceEnabled) speakResult(state, "Directory created");
    return true;
}

static bool cmd_rmdir(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) { fputs("Usage: rmdir <directory_name>\n", stderr); return false; }
    wchar_t wpath[MAX_PATH];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[1], -1, wpath, MAX_PATH);
    if (!wlen) { fputs("rmdir: invalid path encoding\n", stderr); return false; }
    if (!RemoveDirectoryW(wpath)) {
        DWORD error = GetLastError();
        if (error == ERROR_DIR_NOT_EMPTY) {
            fputs("rmdir: directory not empty\n", stderr);
        } else if (error == ERROR_FILE_NOT_FOUND) {
            fputs("rmdir: directory not found\n", stderr);
        } else {
            fputs("rmdir: failed to remove directory\n", stderr);
        }
        return false;
    }
    return true;
}

static bool cmd_dir(ShellState *state, const Command *cmd){
    WIN32_FIND_DATAW findData;
    HANDLE hFind;
    wchar_t searchPath[MAX_PATH];
    
    // Get current directory and create search pattern
    if (!GetCurrentDirectoryW(MAX_PATH, searchPath)) {
        fputs("dir: failed to get current directory\n", stderr);
        return false;
    }
    
    // Append wildcard to search for all files
    wcscat_s(searchPath, MAX_PATH, L"\\*");
    
    hFind = FindFirstFileW(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        fputs("dir: failed to list directory\n", stderr);
        return false;
    }
    
    do {
        // Skip current and parent directory entries
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) {
            continue;
        }
        
        // Convert to UTF-8 for output
        char utf8Name[MAX_PATH];
        WideCharToMultiByte(CP_UTF8, 0, findData.cFileName, -1, utf8Name, MAX_PATH, NULL, NULL);
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("[DIR]  %s\n", utf8Name);
        } else {
            printf("[FILE] %s\n", utf8Name);
        }
    } while (FindNextFileW(hFind, &findData));
    
    FindClose(hFind);
    return true;
}

static bool cmd_pwd(ShellState *state, const Command *cmd){
    wchar_t currentPath[MAX_PATH];
    if (!GetCurrentDirectoryW(MAX_PATH, currentPath)) {
        fputs("pwd: failed to get current directory\n", stderr);
        return false;
    }
    
    char utf8Path[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, currentPath, -1, utf8Path, MAX_PATH, NULL, NULL);
    printf("%s\n", utf8Path);
    if (state->voiceEnabled) speakResult(state, utf8Path);
    return true;
}

static bool cmd_copy(ShellState *state, const Command *cmd){
    if (cmd->argc < 3) { fputs("Usage: copy <source> <destination>\n", stderr); return false; }
    
    wchar_t wsource[MAX_PATH], wdest[MAX_PATH];
    int wlen1 = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[1], -1, wsource, MAX_PATH);
    int wlen2 = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[2], -1, wdest, MAX_PATH);
    
    if (!wlen1 || !wlen2) { fputs("copy: invalid path encoding\n", stderr); return false; }
    
    if (!CopyFileW(wsource, wdest, FALSE)) {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            fputs("copy: source file not found\n", stderr);
            if (state->voiceEnabled) speakResult(state, "Source file not found");
        } else if (error == ERROR_ACCESS_DENIED) {
            fputs("copy: access denied\n", stderr);
            if (state->voiceEnabled) speakResult(state, "Access denied");
        } else if (error == ERROR_FILE_EXISTS) {
            fputs("copy: destination file already exists\n", stderr);
            if (state->voiceEnabled) speakResult(state, "Destination file already exists");
        } else {
            fputs("copy: failed to copy file\n", stderr);
            if (state->voiceEnabled) speakResult(state, "Failed to copy file");
        }
        return false;
    }
    if (state->voiceEnabled) speakResult(state, "File copied successfully");
    return true;
}

static bool cmd_move(ShellState *state, const Command *cmd){
    if (cmd->argc < 3) { fputs("Usage: move <source> <destination>\n", stderr); return false; }
    
    wchar_t wsource[MAX_PATH], wdest[MAX_PATH];
    int wlen1 = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[1], -1, wsource, MAX_PATH);
    int wlen2 = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[2], -1, wdest, MAX_PATH);
    
    if (!wlen1 || !wlen2) { fputs("move: invalid path encoding\n", stderr); return false; }
    
    if (!MoveFileW(wsource, wdest)) {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            fputs("move: source file not found\n", stderr);
        } else if (error == ERROR_ACCESS_DENIED) {
            fputs("move: access denied\n", stderr);
        } else if (error == ERROR_FILE_EXISTS) {
            fputs("move: destination file already exists\n", stderr);
        } else {
            fputs("move: failed to move file\n", stderr);
        }
        return false;
    }
    return true;
}

static bool cmd_del(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) { fputs("Usage: del <file>\n", stderr); return false; }
    
    wchar_t wpath[MAX_PATH];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[1], -1, wpath, MAX_PATH);
    if (!wlen) { fputs("del: invalid path encoding\n", stderr); return false; }
    
    if (!DeleteFileW(wpath)) {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            fputs("del: file not found\n", stderr);
        } else if (error == ERROR_ACCESS_DENIED) {
            fputs("del: access denied\n", stderr);
        } else {
            fputs("del: failed to delete file\n", stderr);
        }
        return false;
    }
    return true;
}

static bool cmd_type(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) { fputs("Usage: type <file>\n", stderr); return false; }
    
    wchar_t wpath[MAX_PATH];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd->argv[1], -1, wpath, MAX_PATH);
    if (!wlen) { fputs("type: invalid path encoding\n", stderr); return false; }
    
    FILE *file = _wfopen(wpath, L"r");
    if (!file) {
        fputs("type: cannot open file\n", stderr);
        return false;
    }
    
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        fputs(buffer, stdout);
    }
    fclose(file);
    return true;
}

static bool cmd_echo(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) { fputs("Usage: echo <text> [> file]\n", stderr); return false; }
    
    // Check for redirection
    bool redirect = false;
    char *outputFile = NULL;
    for (int i = 1; i < cmd->argc; i++) {
        if (strcmp(cmd->argv[i], ">") == 0 && i + 1 < cmd->argc) {
            redirect = true;
            outputFile = cmd->argv[i + 1];
            break;
        }
    }
    
    if (redirect && outputFile) {
        // Write to file
        wchar_t wpath[MAX_PATH];
        int wlen = MultiByteToWideChar(CP_UTF8, 0, outputFile, -1, wpath, MAX_PATH);
        if (!wlen) { fputs("echo: invalid path encoding\n", stderr); return false; }
        
        FILE *file = _wfopen(wpath, L"w");
        if (!file) {
            fputs("echo: cannot create file\n", stderr);
            return false;
        }
        
        // Write all arguments before ">" to file
        for (int i = 1; i < cmd->argc; i++) {
            if (strcmp(cmd->argv[i], ">") == 0) break;
            if (i > 1) fputc(' ', file);
            fputs(cmd->argv[i], file);
        }
        fclose(file);
    } else {
        // Print to console
        char output[1024] = {0};
        for (int i = 1; i < cmd->argc; i++) {
            if (i > 1) {
                fputc(' ', stdout);
                strcat_s(output, sizeof(output), " ");
            }
            fputs(cmd->argv[i], stdout);
            strcat_s(output, sizeof(output), cmd->argv[i]);
        }
        fputc('\n', stdout);
        // Speak echo output when voice is enabled
        if (state->voiceEnabled && strlen(output) > 0) {
            speakResult(state, output);
        }
    }
    return true;
}

static bool cmd_welcome(ShellState *state, const Command *cmd){
    printWelcomeBanner();
    return true;
}

static bool cmd_voice(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) {
        fputs("Usage: voice <on|off|listen|test|status>\n", stderr);
        fputs("  voice on     - Enable text-to-speech output\n", stderr);
        fputs("  voice off    - Disable text-to-speech output\n", stderr);
        fputs("  voice listen - Listen for voice command and execute it\n", stderr);
        fputs("                 Converts natural language to terminal commands\n", stderr);
        fputs("  voice test   - Test microphone and voice recognition setup\n", stderr);
        fputs("  voice status - Show current voice status\n", stderr);
        fputs("\n", stderr);
        fputs("Voice Command Examples:\n", stderr);
        fputs("  \"open chrome\" or \"launch chrome\" -> opens Chrome\n", stderr);
        fputs("  \"show directory\" -> runs 'dir'\n", stderr);
        fputs("  \"current directory\" -> runs 'pwd'\n", stderr);
        fputs("  \"go to Documents\" -> runs 'cd Documents'\n", stderr);
        fputs("  \"create folder test\" -> runs 'mkdir test'\n", stderr);
        return false;
    }
    
    if (strcmp(cmd->argv[1], "on") == 0) {
        if (!voiceIsAvailable()) {
            if (!voiceInit()) {
                fputs("voice: Failed to initialize voice system. Make sure SAPI is available.\n", stderr);
                return false;
            }
        }
        state->voiceEnabled = true;
        puts("Voice output enabled. Command outputs will be spoken.");
        if (state->voiceEnabled) voiceSpeak("Voice output enabled");
        return true;
    } else if (strcmp(cmd->argv[1], "off") == 0) {
        state->voiceEnabled = false;
        state->voiceRecognitionEnabled = false;
        voiceRecognitionStopListening();
        puts("Voice disabled.");
        return true;
    } else if (strcmp(cmd->argv[1], "listen") == 0) {
        if (!voiceRecognitionInit()) {
            const char* error = voiceRecognitionGetLastError();
            if (error) {
                fprintf(stderr, "voice: %s\n", error);
            } else {
                fputs("voice: Failed to initialize voice recognition. Make sure microphone is available.\n", stderr);
            }
            fputs("\nTroubleshooting:\n", stderr);
            fputs("  1. Check Windows Settings > Privacy > Microphone - ensure access is enabled\n", stderr);
            fputs("  2. Make sure Windows Speech Recognition is set up (Settings > Time & Language > Speech)\n", stderr);
            fputs("  3. Test your microphone in Windows Sound settings\n", stderr);
            fputs("  4. Try: voice test - to test microphone connection\n", stderr);
            return false;
        }
        puts("Listening for voice command... Speak now (30 second timeout)");
        fputs("(Make sure your microphone is working and speak clearly)\n", stdout);
        if (state->voiceEnabled) voiceSpeak("Listening for command");
        
        char recognized[512] = {0};
        if (voiceRecognitionListen(recognized, sizeof(recognized), 30)) {
            printf("Recognized: %s\n", recognized);
            
            // Convert natural language to valid terminal command
            char normalized[512] = {0};
            strncpy_s(normalized, sizeof(normalized), recognized, _TRUNCATE);
            normalizeVoiceCommand(normalized, sizeof(normalized));
            
            if (strcmp(normalized, recognized) != 0) {
                printf("Converted to: %s\n", normalized);
            }
            
            if (state->voiceEnabled) {
                char msg[600];
                snprintf(msg, sizeof(msg), "Recognized: %s", recognized);
                voiceSpeak(msg);
            }
            
            // Execute the normalized command
            if (strlen(normalized) > 0) {
                // Parse and execute the command
                Command recoCmd = {0};
                if (parseCommand(normalized, &recoCmd)) {
                    printf("Executing: %s\n", normalized);
                    if (isBuiltin(&recoCmd)) {
                        runBuiltin(state, &recoCmd);
                    } else {
                        ProcessResult res = spawnProcess(&recoCmd, false);
                        if (!res.success) {
                            fprintf(stderr, "Error: %s\n", res.errorMessage);
                            if (state->voiceEnabled) voiceSpeak(res.errorMessage);
                        } else {
                            if (state->voiceEnabled) voiceSpeak("Command executed successfully");
                        }
                    }
                    freeCommand(&recoCmd);
                } else {
                    fprintf(stderr, "Error: Could not parse command: %s\n", normalized);
                    if (state->voiceEnabled) voiceSpeak("Could not parse command");
                }
            }
        } else {
            const char* error = voiceRecognitionGetLastError();
            if (error) {
                fprintf(stderr, "Error: %s\n", error);
            } else {
                puts("No voice input detected or recognition failed.");
            }
            fputs("\nTroubleshooting tips:\n", stderr);
            fputs("  - Make sure microphone is connected and working\n", stderr);
            fputs("  - Check Windows microphone permissions (Settings > Privacy > Microphone)\n", stderr);
            fputs("  - Ensure Windows Speech Recognition is enabled\n", stderr);
            fputs("  - Speak clearly and wait a moment after speaking\n", stderr);
            fputs("  - Try: voice test - to test microphone\n", stderr);
            if (state->voiceEnabled) voiceSpeak("No voice input detected");
        }
        return true;
    } else if (strcmp(cmd->argv[1], "test") == 0) {
        puts("Testing microphone and voice recognition setup...");
        if (voiceRecognitionTestMicrophone()) {
            puts("✓ Microphone connection: OK");
            puts("✓ Recognition system: Initialized");
            if (voiceRecognitionIsAvailable()) {
                puts("✓ Voice recognition: Available");
                puts("\nMicrophone test passed! Try 'voice listen' to use voice commands.");
            } else {
                puts("✗ Voice recognition: Not available");
                fputs("\nTry running 'voice listen' to initialize recognition.\n", stderr);
            }
        } else {
            const char* error = voiceRecognitionGetLastError();
            puts("✗ Microphone test failed!");
            if (error) {
                fprintf(stderr, "Error: %s\n", error);
            }
            fputs("\nTroubleshooting:\n", stderr);
            fputs("  1. Check Windows Settings > Privacy > Microphone - ensure access is enabled\n", stderr);
            fputs("  2. Make sure Windows Speech Recognition is set up\n", stderr);
            fputs("  3. Test your microphone in Windows Sound settings\n", stderr);
            fputs("  4. Restart AuraShell and try again\n", stderr);
        }
        return true;
    } else if (strcmp(cmd->argv[1], "status") == 0) {
        printf("Voice Output: %s\n", state->voiceEnabled && voiceIsAvailable() ? "ON" : "OFF");
        printf("Voice Recognition: %s\n", voiceRecognitionIsAvailable() ? "Available" : "Not available");
        return true;
    } else {
        fputs("Usage: voice <on|off|listen|status>\n", stderr);
        return false;
    }
}

static bool cmd_explain(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) {
        fputs("Usage: explain <command>\n", stderr);
        fputs("Explains what a command does and how to use it.\n", stderr);
        fputs("\nAvailable commands to explain:\n", stderr);
        fputs("  exit, cd, help, history, open, theme, newwin, alias, welcome, search\n", stderr);
        fputs("  mkdir, rmdir, dir, pwd, copy, move, del, type, echo, voice\n", stderr);
        return false;
    }
    
    const char *cmdName = cmd->argv[1];
    
    // Convert to lowercase for comparison
    char lowerCmd[64];
    strncpy_s(lowerCmd, sizeof(lowerCmd), cmdName, _TRUNCATE);
    for (int i = 0; lowerCmd[i]; i++) {
        lowerCmd[i] = (char)tolower(lowerCmd[i]);
    }
    
    printf("\n");
    printf("========================================\n");
    printf("  Command Explanation: %s\n", cmdName);
    printf("========================================\n");
    printf("\n");
    
    if (strcmp(lowerCmd, "dir") == 0) {
        printf("NAME:\n");
        printf("  dir - List directory contents\n");
        printf("\nDESCRIPTION:\n");
        printf("  The dir command displays a list of files and directories in the current\n");
        printf("  working directory. It shows each item with a [DIR] or [FILE] prefix to\n");
        printf("  indicate whether it's a directory or a file.\n");
        printf("\nUSAGE:\n");
        printf("  dir\n");
        printf("\nEXAMPLES:\n");
        printf("  dir              - List all files and directories in current directory\n");
        printf("\nNOTES:\n");
        printf("  - This command does not show hidden files by default\n");
        printf("  - The output shows [DIR] for directories and [FILE] for files\n");
        printf("  - Similar to 'ls' in Unix/Linux systems\n");
    } else if (strcmp(lowerCmd, "cd") == 0) {
        printf("NAME:\n");
        printf("  cd - Change directory\n");
        printf("\nDESCRIPTION:\n");
        printf("  The cd command changes the current working directory to the specified path.\n");
        printf("  You can use both absolute paths (e.g., C:\\Users) and relative paths\n");
        printf("  (e.g., .. or Documents).\n");
        printf("\nUSAGE:\n");
        printf("  cd <path>\n");
        printf("\nEXAMPLES:\n");
        printf("  cd C:\\Users        - Change to C:\\Users directory\n");
        printf("  cd Documents       - Change to Documents subdirectory\n");
        printf("  cd ..              - Move up one directory level\n");
        printf("  cd \\               - Change to root of current drive\n");
        printf("\nNOTES:\n");
        printf("  - Use quotes if the path contains spaces\n");
        printf("  - The command will show an error if the directory doesn't exist\n");
    } else if (strcmp(lowerCmd, "pwd") == 0) {
        printf("NAME:\n");
        printf("  pwd - Print working directory\n");
        printf("\nDESCRIPTION:\n");
        printf("  The pwd command displays the full path of the current working directory.\n");
        printf("  This is useful when you need to know your current location in the\n");
        printf("  file system.\n");
        printf("\nUSAGE:\n");
        printf("  pwd\n");
        printf("\nEXAMPLES:\n");
        printf("  pwd               - Display current directory path\n");
        printf("\nNOTES:\n");
        printf("  - Similar to 'pwd' in Unix/Linux systems\n");
        printf("  - Shows the full absolute path\n");
    } else if (strcmp(lowerCmd, "mkdir") == 0) {
        printf("NAME:\n");
        printf("  mkdir - Create directory\n");
        printf("\nDESCRIPTION:\n");
        printf("  The mkdir command creates a new directory with the specified name in the\n");
        printf("  current working directory or at the specified path.\n");
        printf("\nUSAGE:\n");
        printf("  mkdir <directory_name>\n");
        printf("\nEXAMPLES:\n");
        printf("  mkdir MyFolder    - Create a directory named 'MyFolder'\n");
        printf("  mkdir C:\\NewDir   - Create a directory at C:\\NewDir\n");
        printf("\nNOTES:\n");
        printf("  - The command will fail if the directory already exists\n");
        printf("  - Use quotes if the directory name contains spaces\n");
        printf("  - Similar to 'mkdir' in Unix/Linux systems\n");
    } else if (strcmp(lowerCmd, "rmdir") == 0) {
        printf("NAME:\n");
        printf("  rmdir - Remove directory\n");
        printf("\nDESCRIPTION:\n");
        printf("  The rmdir command removes (deletes) an empty directory. The directory\n");
        printf("  must be empty for this command to succeed.\n");
        printf("\nUSAGE:\n");
        printf("  rmdir <directory_name>\n");
        printf("\nEXAMPLES:\n");
        printf("  rmdir MyFolder    - Remove the 'MyFolder' directory\n");
        printf("  rmdir C:\\OldDir   - Remove directory at C:\\OldDir\n");
        printf("\nNOTES:\n");
        printf("  - The directory must be empty to be removed\n");
        printf("  - Use quotes if the directory name contains spaces\n");
        printf("  - Similar to 'rmdir' in Unix/Linux systems\n");
    } else if (strcmp(lowerCmd, "copy") == 0 || strcmp(lowerCmd, "cp") == 0) {
        printf("NAME:\n");
        printf("  copy, cp - Copy file\n");
        printf("\nDESCRIPTION:\n");
        printf("  The copy command creates a duplicate of a file at a new location. You can\n");
        printf("  copy a file to another directory or give it a new name in the same directory.\n");
        printf("\nUSAGE:\n");
        printf("  copy <source> <destination>\n");
        printf("  cp <source> <destination>\n");
        printf("\nEXAMPLES:\n");
        printf("  copy file.txt backup.txt        - Copy file.txt to backup.txt\n");
        printf("  copy file.txt C:\\Backup\\        - Copy file.txt to C:\\Backup\\\n");
        printf("  cp document.doc document.bak    - Create a backup copy\n");
        printf("\nNOTES:\n");
        printf("  - The destination file will be overwritten if it already exists\n");
        printf("  - Use quotes if paths contain spaces\n");
        printf("  - 'cp' is an alias for 'copy' (Unix/Linux compatibility)\n");
    } else if (strcmp(lowerCmd, "move") == 0 || strcmp(lowerCmd, "mv") == 0) {
        printf("NAME:\n");
        printf("  move, mv - Move or rename file\n");
        printf("\nDESCRIPTION:\n");
        printf("  The move command moves a file from one location to another, or renames a\n");
        printf("  file if the source and destination are in the same directory.\n");
        printf("\nUSAGE:\n");
        printf("  move <source> <destination>\n");
        printf("  mv <source> <destination>\n");
        printf("\nEXAMPLES:\n");
        printf("  move old.txt new.txt            - Rename old.txt to new.txt\n");
        printf("  move file.txt C:\\Backup\\        - Move file.txt to C:\\Backup\\\n");
        printf("  mv document.doc document.bak    - Rename document.doc\n");
        printf("\nNOTES:\n");
        printf("  - The source file is removed after moving\n");
        printf("  - Use quotes if paths contain spaces\n");
        printf("  - 'mv' is an alias for 'move' (Unix/Linux compatibility)\n");
    } else if (strcmp(lowerCmd, "del") == 0 || strcmp(lowerCmd, "rm") == 0) {
        printf("NAME:\n");
        printf("  del, rm - Delete file\n");
        printf("\nDESCRIPTION:\n");
        printf("  The del command permanently deletes a file from the file system. This\n");
        printf("  action cannot be undone, so use with caution.\n");
        printf("\nUSAGE:\n");
        printf("  del <file>\n");
        printf("  rm <file>\n");
        printf("\nEXAMPLES:\n");
        printf("  del file.txt            - Delete file.txt\n");
        printf("  del C:\\Temp\\old.log     - Delete old.log from C:\\Temp\\\n");
        printf("  rm unwanted.txt         - Delete unwanted.txt\n");
        printf("\nNOTES:\n");
        printf("  - This action is permanent and cannot be undone\n");
        printf("  - Use quotes if the filename contains spaces\n");
        printf("  - 'rm' is an alias for 'del' (Unix/Linux compatibility)\n");
    } else if (strcmp(lowerCmd, "type") == 0 || strcmp(lowerCmd, "cat") == 0) {
        printf("NAME:\n");
        printf("  type, cat - Display file contents\n");
        printf("\nDESCRIPTION:\n");
        printf("  The type command displays the contents of a text file on the screen.\n");
        printf("  This is useful for viewing configuration files, logs, or any text file.\n");
        printf("\nUSAGE:\n");
        printf("  type <file>\n");
        printf("  cat <file>\n");
        printf("\nEXAMPLES:\n");
        printf("  type config.txt         - Display contents of config.txt\n");
        printf("  type C:\\Logs\\app.log    - Display contents of app.log\n");
        printf("  cat readme.txt          - Display contents of readme.txt\n");
        printf("\nNOTES:\n");
        printf("  - Best used for text files\n");
        printf("  - Binary files may display incorrectly\n");
        printf("  - 'cat' is an alias for 'type' (Unix/Linux compatibility)\n");
    } else if (strcmp(lowerCmd, "echo") == 0) {
        printf("NAME:\n");
        printf("  echo - Print text or write to file\n");
        printf("\nDESCRIPTION:\n");
        printf("  The echo command prints text to the console or writes text to a file.\n");
        printf("  When used with the '>' redirection operator, it writes to a file instead\n");
        printf("  of displaying on screen.\n");
        printf("\nUSAGE:\n");
        printf("  echo <text>                    - Print text to console\n");
        printf("  echo <text> > <file>           - Write text to file\n");
        printf("\nEXAMPLES:\n");
        printf("  echo Hello World               - Print 'Hello World'\n");
        printf("  echo Hello > greeting.txt      - Write 'Hello' to greeting.txt\n");
        printf("  echo Line 1 > file.txt         - Create file.txt with 'Line 1'\n");
        printf("\nNOTES:\n");
        printf("  - Use '>' to redirect output to a file\n");
        printf("  - The file will be created if it doesn't exist\n");
        printf("  - Existing files will be overwritten\n");
    } else if (strcmp(lowerCmd, "theme") == 0) {
        printf("NAME:\n");
        printf("  theme - Manage shell themes\n");
        printf("\nDESCRIPTION:\n");
        printf("  The theme command allows you to change the color scheme of AuraShell.\n");
        printf("  You can list available themes or set a specific theme to customize the\n");
        printf("  appearance of your shell.\n");
        printf("\nUSAGE:\n");
        printf("  theme list                     - List all available themes\n");
        printf("  theme set <name>               - Change to a specific theme\n");
        printf("\nEXAMPLES:\n");
        printf("  theme list                     - Show all available themes\n");
        printf("  theme set dark                 - Switch to dark theme\n");
        printf("  theme set cyan-bg              - Switch to cyan background theme\n");
        printf("  theme set green                - Switch to green theme\n");
        printf("\nNOTES:\n");
        printf("  - Themes change both foreground and background colors\n");
        printf("  - Some themes include background colors (e.g., cyan-bg, green-bg)\n");
        printf("  - Available themes: default, light, dark, cyan, yellow, green, red, blue, purple, terminal, matrix\n");
    } else if (strcmp(lowerCmd, "alias") == 0) {
        printf("NAME:\n");
        printf("  alias - Manage command aliases\n");
        printf("\nDESCRIPTION:\n");
        printf("  The alias command allows you to create shortcuts for commands. You can\n");
        printf("  define custom aliases that expand to longer commands, making your\n");
        printf("  workflow more efficient.\n");
        printf("\nUSAGE:\n");
        printf("  alias                          - List all aliases\n");
        printf("  alias list                     - List all aliases\n");
        printf("  alias add <name> <command>     - Create a new alias\n");
        printf("  alias set <name> <command>     - Create or update an alias\n");
        printf("  alias remove <name>            - Remove an alias\n");
        printf("  alias help                     - Show alias help\n");
        printf("\nEXAMPLES:\n");
        printf("  alias add ll \"dir\"             - Create 'll' alias for 'dir'\n");
        printf("  alias add grep \"findstr\"       - Create 'grep' alias for 'findstr'\n");
        printf("  alias remove ll                - Remove the 'll' alias\n");
        printf("  alias                          - Show all defined aliases\n");
        printf("\nNOTES:\n");
        printf("  - Aliases are saved and persist across sessions\n");
        printf("  - You cannot create aliases that conflict with builtin commands\n");
        printf("  - Use quotes around the command if it contains spaces\n");
    } else if (strcmp(lowerCmd, "search") == 0) {
        printf("NAME:\n");
        printf("  search - Search for files and directories\n");
        printf("\nDESCRIPTION:\n");
        printf("  The search command finds files and directories matching a pattern. It\n");
        printf("  supports wildcards, recursive searching, and various filtering options.\n");
        printf("\nUSAGE:\n");
        printf("  search <pattern> [options]\n");
        printf("\nOPTIONS:\n");
        printf("  -d <dir>     - Search in specific directory (default: current)\n");
        printf("  -f           - Search files only\n");
        printf("  -dir         - Search directories only\n");
        printf("  -r           - Recursive search (search subdirectories)\n");
        printf("  -i           - Case insensitive search\n");
        printf("\nEXAMPLES:\n");
        printf("  search *.txt                   - Find all .txt files in current directory\n");
        printf("  search test -r                 - Find files/dirs named 'test' recursively\n");
        printf("  search *.exe -f -d C:\\         - Find .exe files in C:\\ drive\n");
        printf("  search config -dir             - Find directories named 'config'\n");
        printf("\nNOTES:\n");
        printf("  - Supports wildcard patterns (e.g., *.txt, test*)\n");
        printf("  - Use -r for deep directory searches\n");
        printf("  - Combine options for more specific searches\n");
    } else if (strcmp(lowerCmd, "history") == 0) {
        printf("NAME:\n");
        printf("  history - Show command history\n");
        printf("\nDESCRIPTION:\n");
        printf("  The history command displays previously executed commands. You can also\n");
        printf("  navigate history using arrow keys (Up/Down) or use history expansion\n");
        printf("  with '!' notation.\n");
        printf("\nUSAGE:\n");
        printf("  history [n]                    - Show last n commands (optional)\n");
        printf("\nEXAMPLES:\n");
        printf("  history                        - Show all command history\n");
        printf("  history 10                     - Show last 10 commands\n");
        printf("  !!                             - Repeat last command\n");
        printf("  !5                             - Repeat command number 5\n");
        printf("\nNOTES:\n");
        printf("  - History is saved across sessions\n");
        printf("  - Use Up/Down arrows to navigate history interactively\n");
        printf("  - Use '!!' to repeat the last command\n");
        printf("  - Use '!n' to repeat command number n\n");
    } else if (strcmp(lowerCmd, "open") == 0) {
        printf("NAME:\n");
        printf("  open - Open file or folder\n");
        printf("\nDESCRIPTION:\n");
        printf("  The open command opens a file or folder using the default Windows\n");
        printf("  application associated with that file type. This is similar to\n");
        printf("  double-clicking a file in Windows Explorer.\n");
        printf("\nUSAGE:\n");
        printf("  open <path>\n");
        printf("\nEXAMPLES:\n");
        printf("  open file.txt                  - Open file.txt with default application\n");
        printf("  open C:\\Users                  - Open C:\\Users folder in Explorer\n");
        printf("  open https://example.com       - Open URL in default browser\n");
        printf("\nNOTES:\n");
        printf("  - Works with files, folders, and URLs\n");
        printf("  - Uses Windows default application associations\n");
        printf("  - Similar to 'start' command in cmd.exe\n");
    } else if (strcmp(lowerCmd, "newwin") == 0) {
        printf("NAME:\n");
        printf("  newwin - Open new shell window\n");
        printf("\nDESCRIPTION:\n");
        printf("  The newwin command opens a new AuraShell window. This is useful when\n");
        printf("  you want to run multiple shell sessions simultaneously.\n");
        printf("\nUSAGE:\n");
        printf("  newwin\n");
        printf("\nEXAMPLES:\n");
        printf("  newwin                         - Open a new AuraShell window\n");
        printf("\nNOTES:\n");
        printf("  - Each new window is independent\n");
        printf("  - Useful for multitasking\n");
    } else if (strcmp(lowerCmd, "help") == 0) {
        printf("NAME:\n");
        printf("  help - Show help information\n");
        printf("\nDESCRIPTION:\n");
        printf("  The help command displays comprehensive information about AuraShell,\n");
        printf("  including all available commands, their usage, and examples.\n");
        printf("\nUSAGE:\n");
        printf("  help\n");
        printf("\nEXAMPLES:\n");
        printf("  help                           - Show full help documentation\n");
        printf("\nNOTES:\n");
        printf("  - Provides overview of all commands\n");
        printf("  - Use 'explain <command>' for detailed information about a specific command\n");
    } else if (strcmp(lowerCmd, "exit") == 0) {
        printf("NAME:\n");
        printf("  exit - Exit the shell\n");
        printf("\nDESCRIPTION:\n");
        printf("  The exit command terminates the AuraShell session and closes the\n");
        printf("  current shell window.\n");
        printf("\nUSAGE:\n");
        printf("  exit\n");
        printf("\nEXAMPLES:\n");
        printf("  exit                           - Close the shell\n");
        printf("\nNOTES:\n");
        printf("  - All unsaved history and aliases are saved before exiting\n");
        printf("  - You can also close the window directly\n");
    } else if (strcmp(lowerCmd, "welcome") == 0) {
        printf("NAME:\n");
        printf("  welcome - Show welcome banner\n");
        printf("\nDESCRIPTION:\n");
        printf("  The welcome command displays the AuraShell welcome banner again. This\n");
        printf("  is the same banner that appears when you first start the shell.\n");
        printf("\nUSAGE:\n");
        printf("  welcome\n");
        printf("\nEXAMPLES:\n");
        printf("  welcome                        - Display welcome banner\n");
        printf("\nNOTES:\n");
        printf("  - Useful if you want to see the welcome message again\n");
    } else if (strcmp(lowerCmd, "voice") == 0) {
        printf("NAME:\n");
        printf("  voice - Enable or disable text-to-speech output\n");
        printf("\nDESCRIPTION:\n");
        printf("  The voice command controls text-to-speech functionality in AuraShell.\n");
        printf("  When enabled, command outputs, success messages, and error messages\n");
        printf("  will be spoken using Windows Speech API (SAPI). This is useful for\n");
        printf("  accessibility and hands-free operation.\n");
        printf("\nUSAGE:\n");
        printf("  voice on                       - Enable text-to-speech output\n");
        printf("  voice off                      - Disable text-to-speech output\n");
        printf("  voice status                   - Show current voice status\n");
        printf("\nEXAMPLES:\n");
        printf("  voice on                       - Turn on voice output\n");
        printf("  voice off                      - Turn off voice output\n");
        printf("  voice status                   - Check if voice is enabled\n");
        printf("  voice on && echo Hello         - Enable voice and speak 'Hello'\n");
        printf("\nNOTES:\n");
        printf("  - Requires Windows Speech API (SAPI) to be available\n");
        printf("  - When enabled, command outputs will be spoken\n");
        printf("  - Useful for accessibility and hands-free operation\n");
        printf("  - Error messages and success messages are also spoken\n");
    } else if (strcmp(lowerCmd, "explain") == 0) {
        printf("NAME:\n");
        printf("  explain - Explain what a command does\n");
        printf("\nDESCRIPTION:\n");
        printf("  The explain command provides detailed information about what a command\n");
        printf("  does, how to use it, and includes examples. This is useful for learning\n");
        printf("  about commands you're not familiar with.\n");
        printf("\nUSAGE:\n");
        printf("  explain <command>\n");
        printf("\nEXAMPLES:\n");
        printf("  explain dir                    - Get detailed explanation of 'dir' command\n");
        printf("  explain copy                   - Learn how to use the 'copy' command\n");
        printf("  explain theme                  - Understand theme management\n");
        printf("  explain voice                  - Learn about voice command\n");
        printf("\nNOTES:\n");
        printf("  - Provides more detailed information than 'help'\n");
        printf("  - Works with all builtin commands\n");
    } else {
        printf("Sorry, I don't have an explanation for '%s'.\n", cmdName);
        printf("\nAvailable commands to explain:\n");
        printf("  exit, cd, help, history, open, theme, newwin, alias, welcome, search\n");
        printf("  mkdir, rmdir, dir, pwd, copy, move, del, type, echo, voice\n");
        printf("\nUse 'help' to see a general overview of all commands.\n");
    }
    
    printf("\n");
    printf("========================================\n");
    printf("\n");
    
    return true;
}

static bool cmd_search(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) { 
        fputs("Usage: search <pattern> [options]\n", stderr);
        fputs("Options:\n", stderr);
        fputs("  -d <dir>     - Search in specific directory (default: current)\n", stderr);
        fputs("  -f           - Search files only\n", stderr);
        fputs("  -dir         - Search directories only\n", stderr);
        fputs("  -r           - Recursive search (search subdirectories)\n", stderr);
        fputs("  -i           - Case insensitive search\n", stderr);
        fputs("Examples:\n", stderr);
        fputs("  search *.txt           - Find all .txt files in current directory\n", stderr);
        fputs("  search test -r         - Find files/dirs named 'test' recursively\n", stderr);
        fputs("  search *.exe -f -d C:\\ - Find .exe files in C:\\ drive\n", stderr);
        return false; 
    }
    
    const char* pattern = cmd->argv[1];
    const char* searchDir = ".";
    bool filesOnly = false;
    bool dirsOnly = false;
    bool recursive = false;
    bool caseInsensitive = false;
    
    // Parse options
    for (int i = 2; i < cmd->argc; i++) {
        if (strcmp(cmd->argv[i], "-d") == 0 && i + 1 < cmd->argc) {
            searchDir = cmd->argv[++i];
        } else if (strcmp(cmd->argv[i], "-f") == 0) {
            filesOnly = true;
        } else if (strcmp(cmd->argv[i], "-dir") == 0) {
            dirsOnly = true;
        } else if (strcmp(cmd->argv[i], "-r") == 0) {
            recursive = true;
        } else if (strcmp(cmd->argv[i], "-i") == 0) {
            caseInsensitive = true;
        }
    }
    
    // Convert search directory to wide char
    wchar_t wSearchDir[MAX_PATH];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, searchDir, -1, wSearchDir, MAX_PATH);
    if (!wlen) {
        wlen = MultiByteToWideChar(CP_ACP, 0, searchDir, -1, wSearchDir, MAX_PATH);
        if (!wlen) {
            fputs("search: invalid directory path\n", stderr);
            return false;
        }
    }
    
    // Convert pattern to wide char
    wchar_t wPattern[MAX_PATH];
    wlen = MultiByteToWideChar(CP_UTF8, 0, pattern, -1, wPattern, MAX_PATH);
    if (!wlen) {
        wlen = MultiByteToWideChar(CP_ACP, 0, pattern, -1, wPattern, MAX_PATH);
        if (!wlen) {
            fputs("search: invalid pattern\n", stderr);
            return false;
        }
    }
    
    // Create search path
    wchar_t searchPath[MAX_PATH];
    wcscpy_s(searchPath, MAX_PATH, wSearchDir);
    wcscat_s(searchPath, MAX_PATH, L"\\*");
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath, &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        fputs("search: failed to search directory\n", stderr);
        return false;
    }
    
    int foundCount = 0;
    
    do {
        // Skip current and parent directory entries
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) {
            continue;
        }
        
        // Check if it matches the pattern
        bool matches = false;
        if (wcscmp(wPattern, L"*") == 0) {
            matches = true; // Match everything
        } else {
            // Simple wildcard matching
            matches = PathMatchSpecW(findData.cFileName, wPattern);
        }
        
        if (matches) {
            // Check type filters
            bool isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            
            if ((filesOnly && isDirectory) || (dirsOnly && !isDirectory)) {
                continue;
            }
            
            // Convert to UTF-8 for output
            char utf8Name[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, findData.cFileName, -1, utf8Name, MAX_PATH, NULL, NULL);
            
            // Get full path
            wchar_t fullPath[MAX_PATH];
            wcscpy_s(fullPath, MAX_PATH, wSearchDir);
            wcscat_s(fullPath, MAX_PATH, L"\\");
            wcscat_s(fullPath, MAX_PATH, findData.cFileName);
            
            char utf8Path[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, fullPath, -1, utf8Path, MAX_PATH, NULL, NULL);
            
            if (isDirectory) {
                printf("[DIR]  %s\n", utf8Path);
            } else {
                printf("[FILE] %s\n", utf8Path);
            }
            foundCount++;
        }
        
        // Recursive search in subdirectories
        if (recursive && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            wchar_t subDir[MAX_PATH];
            wcscpy_s(subDir, MAX_PATH, wSearchDir);
            wcscat_s(subDir, MAX_PATH, L"\\");
            wcscat_s(subDir, MAX_PATH, findData.cFileName);
            
            char subDirUtf8[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, subDir, -1, subDirUtf8, MAX_PATH, NULL, NULL);
            
            // Create a new command for recursive search
            Command subCmd = {0};
            subCmd.argc = cmd->argc;
            subCmd.argv = (char**)malloc(cmd->argc * sizeof(char*));
            for (int i = 0; i < cmd->argc; i++) {
                subCmd.argv[i] = (char*)malloc(strlen(cmd->argv[i]) + 1);
                strcpy(subCmd.argv[i], cmd->argv[i]);
            }
            // Replace the directory argument
            free(subCmd.argv[2]); // -d
            subCmd.argv[2] = (char*)malloc(strlen(subDirUtf8) + 1);
            strcpy(subCmd.argv[2], subDirUtf8);
            
            // Recursive call (simplified - just search this subdirectory)
            wchar_t subSearchPath[MAX_PATH];
            wcscpy_s(subSearchPath, MAX_PATH, subDir);
            wcscat_s(subSearchPath, MAX_PATH, L"\\*");
            
            WIN32_FIND_DATAW subFindData;
            HANDLE subHFind = FindFirstFileW(subSearchPath, &subFindData);
            
            if (subHFind != INVALID_HANDLE_VALUE) {
                do {
                    if (wcscmp(subFindData.cFileName, L".") == 0 || wcscmp(subFindData.cFileName, L"..") == 0) {
                        continue;
                    }
                    
                    bool subMatches = false;
                    if (wcscmp(wPattern, L"*") == 0) {
                        subMatches = true;
                    } else {
                        subMatches = PathMatchSpecW(subFindData.cFileName, wPattern);
                    }
                    
                    if (subMatches) {
                        bool subIsDirectory = (subFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                        
                        if ((filesOnly && subIsDirectory) || (dirsOnly && !subIsDirectory)) {
                            continue;
                        }
                        
                        char subUtf8Name[MAX_PATH];
                        WideCharToMultiByte(CP_UTF8, 0, subFindData.cFileName, -1, subUtf8Name, MAX_PATH, NULL, NULL);
                        
                        wchar_t subFullPath[MAX_PATH];
                        wcscpy_s(subFullPath, MAX_PATH, subDir);
                        wcscat_s(subFullPath, MAX_PATH, L"\\");
                        wcscat_s(subFullPath, MAX_PATH, subFindData.cFileName);
                        
                        char subUtf8Path[MAX_PATH];
                        WideCharToMultiByte(CP_UTF8, 0, subFullPath, -1, subUtf8Path, MAX_PATH, NULL, NULL);
                        
                        if (subIsDirectory) {
                            printf("[DIR]  %s\n", subUtf8Path);
                        } else {
                            printf("[FILE] %s\n", subUtf8Path);
                        }
                        foundCount++;
                    }
                } while (FindNextFileW(subHFind, &subFindData));
                FindClose(subHFind);
            }
            
            // Clean up subCmd
            for (int i = 0; i < subCmd.argc; i++) {
                free(subCmd.argv[i]);
            }
            free(subCmd.argv);
        }
        
    } while (FindNextFileW(hFind, &findData));
    
    FindClose(hFind);
    
    if (foundCount == 0) {
        printf("No matches found for pattern: %s\n", pattern);
    } else {
        printf("\nFound %d match(es) for pattern: %s\n", foundCount, pattern);
    }
    
    return true;
}

static bool cmd_alias(ShellState *state, const Command *cmd){
    if (cmd->argc < 2) {
        // List all aliases
        aliasList(&state->aliases);
        return true;
    }
    
    if (strcmp(cmd->argv[1], "list") == 0) {
        aliasList(&state->aliases);
        return true;
    }
    
    if (strcmp(cmd->argv[1], "add") == 0 || strcmp(cmd->argv[1], "set") == 0) {
        if (cmd->argc < 4) {
            fputs("Usage: alias add <name> <command>\n", stderr);
            return false;
        }
        
        // Check if the alias name conflicts with builtin commands
        Command testCmd = {0};
        testCmd.argv = (char**)malloc(2 * sizeof(char*));
        testCmd.argv[0] = cmd->argv[2];
        testCmd.argv[1] = NULL;
        testCmd.argc = 1;
        
        if (isBuiltin(&testCmd)) {
            fprintf(stderr, "Error: Cannot create alias '%s' - conflicts with builtin command\n", cmd->argv[2]);
            free(testCmd.argv);
            return false;
        }
        free(testCmd.argv);
        
        aliasAdd(&state->aliases, cmd->argv[2], cmd->argv[3]);
        printf("Alias '%s' created: %s\n", cmd->argv[2], cmd->argv[3]);
        aliasSave(&state->aliases);
        return true;
    }
    
    if (strcmp(cmd->argv[1], "remove") == 0 || strcmp(cmd->argv[1], "del") == 0) {
        if (cmd->argc < 3) {
            fputs("Usage: alias remove <name>\n", stderr);
            return false;
        }
        
        if (!aliasExists(&state->aliases, cmd->argv[2])) {
            fprintf(stderr, "Alias '%s' not found\n", cmd->argv[2]);
            return false;
        }
        
        aliasRemove(&state->aliases, cmd->argv[2]);
        printf("Alias '%s' removed\n", cmd->argv[2]);
        aliasSave(&state->aliases);
        return true;
    }
    
    if (strcmp(cmd->argv[1], "help") == 0) {
        fputs("Alias Management Commands:\n", stdout);
        fputs("  alias                    - List all aliases\n", stdout);
        fputs("  alias list               - List all aliases\n", stdout);
        fputs("  alias add <name> <cmd>   - Create new alias\n", stdout);
        fputs("  alias set <name> <cmd>   - Create/update alias\n", stdout);
        fputs("  alias remove <name>      - Remove alias\n", stdout);
        fputs("  alias del <name>         - Remove alias\n", stdout);
        fputs("  alias help               - Show this help\n", stdout);
        fputs("\nExamples:\n", stdout);
        fputs("  alias add ll \"dir\"\n", stdout);
        fputs("  alias add grep \"findstr\"\n", stdout);
        fputs("  alias add cls \"clear\"\n", stdout);
        return true;
    }
    
    fputs("Usage: alias [list|add|set|remove|del|help] [args...]\n", stderr);
    return false;
}

bool isBuiltin(const Command *cmd){
    if (cmd->argc==0) return false;
    const char *n = cmd->argv[0];
    return strcmp(n, "exit")==0 || strcmp(n, "cd")==0 || strcmp(n, "help")==0 ||
           strcmp(n, "history")==0 || strcmp(n, "open")==0 || strcmp(n, "theme")==0 || strcmp(n, "newwin")==0 ||
           strcmp(n, "mkdir")==0 || strcmp(n, "rmdir")==0 || strcmp(n, "dir")==0 || strcmp(n, "pwd")==0 ||
           strcmp(n, "copy")==0 || strcmp(n, "cp")==0 || strcmp(n, "move")==0 || strcmp(n, "mv")==0 ||
           strcmp(n, "del")==0 || strcmp(n, "rm")==0 || strcmp(n, "type")==0 || strcmp(n, "cat")==0 ||
           strcmp(n, "echo")==0 || strcmp(n, "alias")==0 || strcmp(n, "welcome")==0 || strcmp(n, "search")==0 ||
           strcmp(n, "explain")==0 || strcmp(n, "voice")==0;
}

bool runBuiltin(ShellState *state, const Command *cmd){
    const char *n = cmd->argv[0];
    if (strcmp(n, "exit")==0) return cmd_exit(state, cmd);
    if (strcmp(n, "cd")==0) return cmd_cd(state, cmd);
    if (strcmp(n, "help")==0) return cmd_help(state, cmd);
    if (strcmp(n, "history")==0) return cmd_history(state, cmd);
    if (strcmp(n, "open")==0) return cmd_open(state, cmd);
    if (strcmp(n, "theme")==0) return cmd_theme(state, cmd);
    if (strcmp(n, "newwin")==0) return cmd_newwin(state, cmd);
    if (strcmp(n, "mkdir")==0) return cmd_mkdir(state, cmd);
    if (strcmp(n, "rmdir")==0) return cmd_rmdir(state, cmd);
    if (strcmp(n, "dir")==0) return cmd_dir(state, cmd);
    if (strcmp(n, "pwd")==0) return cmd_pwd(state, cmd);
    if (strcmp(n, "copy")==0 || strcmp(n, "cp")==0) return cmd_copy(state, cmd);
    if (strcmp(n, "move")==0 || strcmp(n, "mv")==0) return cmd_move(state, cmd);
    if (strcmp(n, "del")==0 || strcmp(n, "rm")==0) return cmd_del(state, cmd);
    if (strcmp(n, "type")==0 || strcmp(n, "cat")==0) return cmd_type(state, cmd);
    if (strcmp(n, "echo")==0) return cmd_echo(state, cmd);
    if (strcmp(n, "alias")==0) return cmd_alias(state, cmd);
    if (strcmp(n, "welcome")==0) return cmd_welcome(state, cmd);
    if (strcmp(n, "search")==0) return cmd_search(state, cmd);
    if (strcmp(n, "explain")==0) return cmd_explain(state, cmd);
    if (strcmp(n, "voice")==0) return cmd_voice(state, cmd);
    return false;
}


