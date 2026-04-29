#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

static char *strdup_safe(const char *s){ size_t n=strlen(s)+1; char *p=(char*)malloc(n); if(p) memcpy(p,s,n); return p; }

void historyInit(HistoryBuffer *hb, const wchar_t *appDataDir){
    hb->entries = NULL; hb->size = 0; hb->capacity = 0;
    swprintf(hb->filePath, 260, L"%ls\\history.txt", appDataDir);
}

void historyAdd(HistoryBuffer *hb, const char *line){
    if (!line || !*line) return;
    if (hb->size + 1 > hb->capacity){
        hb->capacity = hb->capacity ? hb->capacity * 2 : 128;
        hb->entries = (char**)realloc(hb->entries, hb->capacity * sizeof(char*));
    }
    hb->entries[hb->size++] = strdup_safe(line);
}

bool historyExpand(const HistoryBuffer *hb, const char *in, char *out, size_t outCap){
    if (!in || !*in) return false;
    if (in[0] != '!') { strncpy_s(out, outCap, in, _TRUNCATE); return true; }
    if (strcmp(in, "!!") == 0){ if (hb->size == 0) return false; strncpy_s(out, outCap, hb->entries[hb->size-1], _TRUNCATE); return true; }
    int n = atoi(in+1);
    if (n <= 0) return false;
    if ((size_t)n > hb->size) return false;
    strncpy_s(out, outCap, hb->entries[hb->size - n], _TRUNCATE);
    return true;
}

void historyPrint(const HistoryBuffer *hb, int lastN){
    size_t start = 0;
    if (lastN > 0 && (size_t)lastN < hb->size) start = hb->size - (size_t)lastN;
    for (size_t i = start; i < hb->size; ++i){
        printf("%zu  %s\n", hb->size - i, hb->entries[i]);
    }
}

void historySave(const HistoryBuffer *hb){
    wchar_t dir[260]; wcscpy_s(dir, 260, hb->filePath);
    for (int i = (int)wcslen(dir)-1; i >= 0; --i){ if (dir[i] == L'\\'){ dir[i] = L'\0'; break; } }
    CreateDirectoryW(dir, NULL);
    FILE *f = NULL; _wfopen_s(&f, hb->filePath, L"wb");
    if (!f) return;
    for (size_t i=0;i<hb->size;++i){
        fputs(hb->entries[i], f); fputc('\n', f);
    }
    fclose(f);
}

void historyLoad(HistoryBuffer *hb){
    FILE *f = NULL; _wfopen_s(&f, hb->filePath, L"rb");
    if (!f) return;
    char line[2048];
    while (fgets(line, sizeof line, f)){
        size_t n = strlen(line);
        while (n>0 && (line[n-1]=='\n' || line[n-1]=='\r')) line[--n]='\0';
        historyAdd(hb, line);
    }
    fclose(f);
}

// History navigation state
static int historyPosition = 0;

void historyResetPosition(HistoryBuffer *hb) {
    historyPosition = (int)hb->size;
}

const wchar_t* historyPrevious(HistoryBuffer *hb) {
    if (historyPosition > 0) {
        historyPosition--;
        // Convert to wide char for return
        static wchar_t wbuf[1024];
        MultiByteToWideChar(CP_UTF8, 0, hb->entries[historyPosition], -1, wbuf, 1024);
        return wbuf;
    }
    return NULL;
}

const wchar_t* historyNext(HistoryBuffer *hb) {
    if (historyPosition < (int)hb->size - 1) {
        historyPosition++;
        // Convert to wide char for return
        static wchar_t wbuf[1024];
        MultiByteToWideChar(CP_UTF8, 0, hb->entries[historyPosition], -1, wbuf, 1024);
        return wbuf;
    }
    return NULL;
}


