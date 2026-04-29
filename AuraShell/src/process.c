#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include "process.h"

static void quoteArg(const char *arg, wchar_t *out, size_t *idx, size_t cap){
    out[(*idx)++] = L'"';
    while (*arg && *idx < cap-2){
        if (*arg == '"' || *arg == '\\') out[(*idx)++] = L'\\';
        out[(*idx)++] = (wchar_t)(unsigned char)*arg++;
    }
    out[(*idx)++] = L'"';
}

void buildWindowsCmdLine(const Command *cmd, wchar_t *out, size_t outCap){
    size_t i=0; out[0]=L'\0';
    for (int a=0; a<cmd->argc; ++a){
        if (a>0 && i<outCap-1) out[i++]=L' ';
        quoteArg(cmd->argv[a], out, &i, outCap);
    }
    out[i]=L'\0';
}

ProcessResult spawnProcess(const Command *cmd, bool newConsole){
    ProcessResult r = {0};
    wchar_t cmdline[2048];
    buildWindowsCmdLine(cmd, cmdline, 2048);
    STARTUPINFOW si; ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    PROCESS_INFORMATION pi; ZeroMemory(&pi, sizeof(pi));
    DWORD flags = newConsole ? CREATE_NEW_CONSOLE : 0;
    
    // First, try to create the process directly
    BOOL ok = CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);
    
    // If that fails with ERROR_FILE_NOT_FOUND, try running through cmd.exe
    if (!ok && GetLastError() == ERROR_FILE_NOT_FOUND) {
        wchar_t cmdExePath[MAX_PATH];
        // Try to find cmd.exe in system directory
        UINT len = GetSystemDirectoryW(cmdExePath, MAX_PATH);
        if (len > 0 && len < MAX_PATH - 10) {
            wcscat_s(cmdExePath, MAX_PATH, L"\\cmd.exe");
            wchar_t fullCmdLine[4096];
            swprintf_s(fullCmdLine, 4096, L"/c %s", cmdline);
            ok = CreateProcessW(cmdExePath, fullCmdLine, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);
        }
    }
    
    if (!ok) {
        DWORD error = GetLastError();
        const char* cmdName = cmd->argc > 0 ? cmd->argv[0] : "unknown";
        
        // Check if this might be a builtin command that's not recognized
        // (This happens when running an old executable built before the command was added)
        const char* commonBuiltins[] = {"explain", "search", "alias", "welcome", "theme", "newwin", 
                                        "open", "history", "help", "cd", "exit", "mkdir", "rmdir", 
                                        "dir", "pwd", "copy", "cp", "move", "mv", "del", "rm", 
                                        "type", "cat", "echo"};
        bool mightBeBuiltin = false;
        for (size_t i = 0; i < sizeof(commonBuiltins)/sizeof(commonBuiltins[0]); i++) {
            if (strcmp(cmdName, commonBuiltins[i]) == 0) {
                mightBeBuiltin = true;
                break;
            }
        }
        
        if (error == ERROR_FILE_NOT_FOUND) {
            if (mightBeBuiltin) {
                snprintf(r.errorMessage, sizeof r.errorMessage, 
                    "Command '%s' not found. This might be a builtin command. "
                    "If you recently added this command, you may need to rebuild AuraShell.", cmdName);
            } else {
                snprintf(r.errorMessage, sizeof r.errorMessage, 
                    "Command '%s' not found. Make sure the command exists and is in your PATH.", cmdName);
            }
        } else {
            snprintf(r.errorMessage, sizeof r.errorMessage, 
                "CreateProcess failed for '%s' (error %lu)", cmdName, error);
        }
        r.success = false;
        return r;
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &r.exitCode);
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    r.success = true;
    return r;
}


