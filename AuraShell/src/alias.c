#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include "alias.h"

static char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static void freeAlias(Alias *alias) {
    if (!alias) return;
    free(alias->name);
    free(alias->command);
    if (alias->args) {
        for (int i = 0; i < alias->argc; i++) {
            free(alias->args[i]);
        }
        free(alias->args);
    }
    memset(alias, 0, sizeof(Alias));
}

void aliasInit(AliasBuffer *ab, const wchar_t *appDataDir) {
    ab->aliases = NULL;
    ab->size = 0;
    ab->capacity = 0;
    swprintf(ab->filePath, MAX_PATH, L"%ls\\aliases.txt", appDataDir);
}

void aliasAdd(AliasBuffer *ab, const char *name, const char *command) {
    if (!name || !command) return;
    
    // Check if alias already exists
    for (size_t i = 0; i < ab->size; i++) {
        if (strcmp(ab->aliases[i].name, name) == 0) {
            // Update existing alias
            free(ab->aliases[i].command);
            ab->aliases[i].command = strdup_safe(command);
            return;
        }
    }
    
    // Add new alias
    if (ab->size >= ab->capacity) {
        ab->capacity = ab->capacity ? ab->capacity * 2 : 16;
        ab->aliases = (Alias*)realloc(ab->aliases, ab->capacity * sizeof(Alias));
    }
    
    Alias *newAlias = &ab->aliases[ab->size++];
    newAlias->name = strdup_safe(name);
    newAlias->command = strdup_safe(command);
    newAlias->args = NULL;
    newAlias->argc = 0;
}

void aliasRemove(AliasBuffer *ab, const char *name) {
    if (!name) return;
    
    for (size_t i = 0; i < ab->size; i++) {
        if (strcmp(ab->aliases[i].name, name) == 0) {
            freeAlias(&ab->aliases[i]);
            // Shift remaining aliases
            for (size_t j = i; j < ab->size - 1; j++) {
                ab->aliases[j] = ab->aliases[j + 1];
            }
            ab->size--;
            return;
        }
    }
}

bool aliasExists(const AliasBuffer *ab, const char *name) {
    if (!name) return false;
    
    for (size_t i = 0; i < ab->size; i++) {
        if (strcmp(ab->aliases[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

const Alias* aliasFind(const AliasBuffer *ab, const char *name) {
    if (!name) return NULL;
    
    for (size_t i = 0; i < ab->size; i++) {
        if (strcmp(ab->aliases[i].name, name) == 0) {
            return &ab->aliases[i];
        }
    }
    return NULL;
}

void aliasList(const AliasBuffer *ab) {
    if (ab->size == 0) {
        printf("No aliases defined.\n");
        return;
    }
    
    printf("Defined aliases:\n");
    for (size_t i = 0; i < ab->size; i++) {
        printf("  %s = %s\n", ab->aliases[i].name, ab->aliases[i].command);
    }
}

void aliasSave(const AliasBuffer *ab) {
    // Create directory if it doesn't exist
    wchar_t dir[MAX_PATH];
    wcscpy_s(dir, MAX_PATH, ab->filePath);
    for (int i = (int)wcslen(dir) - 1; i >= 0; i--) {
        if (dir[i] == L'\\') {
            dir[i] = L'\0';
            break;
        }
    }
    CreateDirectoryW(dir, NULL);
    
    FILE *f = NULL;
    _wfopen_s(&f, ab->filePath, L"wb");
    if (!f) return;
    
    for (size_t i = 0; i < ab->size; i++) {
        fprintf(f, "%s=%s\n", ab->aliases[i].name, ab->aliases[i].command);
    }
    fclose(f);
}

void aliasLoad(AliasBuffer *ab) {
    FILE *f = NULL;
    _wfopen_s(&f, ab->filePath, L"rb");
    if (!f) return;
    
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        // Remove newline
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        
        // Skip empty lines
        if (len == 0) continue;
        
        // Find equals sign
        char *equals = strchr(line, '=');
        if (!equals) continue;
        
        *equals = '\0';
        char *name = line;
        char *command = equals + 1;
        
        // Skip leading/trailing whitespace
        while (*name == ' ' || *name == '\t') name++;
        while (*command == ' ' || *command == '\t') command++;
        
        if (*name && *command) {
            aliasAdd(ab, name, command);
        }
    }
    fclose(f);
}

void aliasFree(AliasBuffer *ab) {
    if (!ab) return;
    
    for (size_t i = 0; i < ab->size; i++) {
        freeAlias(&ab->aliases[i]);
    }
    free(ab->aliases);
    ab->aliases = NULL;
    ab->size = 0;
    ab->capacity = 0;
}

bool aliasExpand(const AliasBuffer *ab, const char *input, char *output, size_t outputSize) {
    if (!input || !output || outputSize == 0) return false;
    
    // Find the first word (command name)
    const char *start = input;
    while (*start == ' ' || *start == '\t') start++;
    
    const char *end = start;
    while (*end && *end != ' ' && *end != '\t') end++;
    
    if (end == start) return false; // No command found
    
    // Extract command name
    size_t cmdLen = end - start;
    char cmdName[256];
    if (cmdLen >= sizeof(cmdName)) return false;
    
    strncpy_s(cmdName, sizeof(cmdName), start, cmdLen);
    cmdName[cmdLen] = '\0';
    
    // Check if it's an alias
    const Alias *alias = aliasFind(ab, cmdName);
    if (!alias) return false;
    
    // Build expanded command
    size_t remaining = outputSize - 1;
    size_t pos = 0;
    
    // Copy alias command
    size_t cmdLen2 = strlen(alias->command);
    if (cmdLen2 >= remaining) return false;
    
    strcpy_s(output + pos, remaining, alias->command);
    pos += cmdLen2;
    remaining -= cmdLen2;
    
    // Add remaining arguments from original input
    const char *remainingArgs = end;
    while (*remainingArgs == ' ' || *remainingArgs == '\t') remainingArgs++;
    
    if (*remainingArgs && remaining > 1) {
        output[pos++] = ' ';
        remaining--;
        
        size_t argsLen = strlen(remainingArgs);
        if (argsLen >= remaining) return false;
        
        strcpy_s(output + pos, remaining, remainingArgs);
    }
    
    return true;
}

void aliasFreeAlias(Alias *alias) {
    freeAlias(alias);
}
