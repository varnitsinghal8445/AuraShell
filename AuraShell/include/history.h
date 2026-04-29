#pragma once
#include <stddef.h>
#include <stdbool.h>
#include "shell.h"

void historyInit(HistoryBuffer *hb, const wchar_t *appDataDir);
void historyAdd(HistoryBuffer *hb, const char *line);
bool historyExpand(const HistoryBuffer *hb, const char *in, char *out, size_t outCap);
void historyPrint(const HistoryBuffer *hb, int lastN);
void historySave(const HistoryBuffer *hb);
void historyLoad(HistoryBuffer *hb);
void historyResetPosition(HistoryBuffer *hb);
const wchar_t* historyPrevious(HistoryBuffer *hb);
const wchar_t* historyNext(HistoryBuffer *hb);


