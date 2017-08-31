#pragma once

#include "BBTypes.h"

BBRETCODE BBUtGetDateStr(CHAR *str, UINT8 len, CHAR delimiter, BOOL reverseDateOrder); // 0 means no delimiter
BBRETCODE BBUtGetTimeStr(CHAR *str, UINT8 len, CHAR delimiter, BOOL addMilliSec); // 0 means no delimiter

void BBUtMakeDirTree(char *path); // recursively creates folder tree
