#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

static char *strdup_safe(const char *s){ size_t n=strlen(s)+1; char *p=(char*)malloc(n); if(p) memcpy(p,s,n); return p; }

bool parseCommand(const char *line, Command *out) {
    out->argc = 0; out->argv = NULL; out->newConsole = false;
    if (!line) return false;
    int capacity = 8;
    out->argv = (char**)malloc(sizeof(char*) * capacity);
    const char *p = line;
    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;
        char buf[256]; int bi = 0; int inq = 0;
        while (*p && (inq || !isspace((unsigned char)*p))) {
            if (*p == '"') { inq = !inq; p++; continue; }
            if (*p == '\\' && p[1]) { buf[bi++] = p[1]; p += 2; continue; }
            buf[bi++] = *p++;
            if (bi >= (int)sizeof(buf)-1) break;
        }
        buf[bi] = '\0';
        if (out->argc >= capacity) { capacity *= 2; out->argv = (char**)realloc(out->argv, sizeof(char*) * capacity); }
        out->argv[out->argc++] = strdup_safe(buf);
    }
    if (out->argc == 0) { free(out->argv); out->argv = NULL; return false; }
    out->argv = (char**)realloc(out->argv, sizeof(char*) * (out->argc + 1));
    out->argv[out->argc] = NULL;
    return true;
}

void freeCommand(Command *cmd) {
    if (!cmd || !cmd->argv) return;
    for (int i = 0; i < cmd->argc; ++i) free(cmd->argv[i]);
    free(cmd->argv); cmd->argv = NULL; cmd->argc = 0;
}


