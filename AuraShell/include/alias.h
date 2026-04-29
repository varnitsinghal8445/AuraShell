#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct Alias {
    char *name;        // Alias name (e.g., "ll")
    char *command;     // Command it maps to (e.g., "dir")
    char **args;       // Additional arguments (optional)
    int argc;          // Number of additional arguments
} Alias;

typedef struct AliasBuffer {
    Alias *aliases;
    size_t size;
    size_t capacity;
    char filePath[MAX_PATH];
} AliasBuffer;

// Alias management functions
void aliasInit(AliasBuffer *ab, const wchar_t *appDataDir);
void aliasAdd(AliasBuffer *ab, const char *name, const char *command);
void aliasRemove(AliasBuffer *ab, const char *name);
bool aliasExists(const AliasBuffer *ab, const char *name);
const Alias* aliasFind(const AliasBuffer *ab, const char *name);
void aliasList(const AliasBuffer *ab);
void aliasSave(const AliasBuffer *ab);
void aliasLoad(AliasBuffer *ab);
void aliasFree(AliasBuffer *ab);

// Alias expansion functions
bool aliasExpand(const AliasBuffer *ab, const char *input, char *output, size_t outputSize);
void aliasFreeAlias(Alias *alias);
