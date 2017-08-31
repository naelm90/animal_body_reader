#include "BBWinUtils.h"
#include <stdio.h>
#include <errno.h>
#include <direct.h>
#include <sys/stat.h>

BBRETCODE BBUtGetDateStr(CHAR *str, UINT8 len, CHAR delimiter, BOOL reverseDateOrder)
{
	if (len < 11) return 1; // "YYYY-MM-DD" (not really safe if used incorrectly)
	
	int tempLen;
	SYSTEMTIME st;
	GetLocalTime(&st);

	if (delimiter != 0) {
		if (reverseDateOrder) {
			tempLen = sprintf(str, "%04d%c%02d%c%02d", st.wYear, delimiter, st.wMonth, delimiter, st.wDay);
		}
		else {
			tempLen = sprintf(str, "%02d%c%02d%c%04d", st.wDay, delimiter, st.wMonth, delimiter, st.wYear);			
		}
	}
	else {
		if (reverseDateOrder) {
			tempLen = sprintf(str, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
		}
		else {
			tempLen = sprintf(str, "%02d%02d%04d", st.wDay, st.wMonth, st.wYear);
		}
	}
	return NO_ERROR;
}


BBRETCODE BBUtGetTimeStr(CHAR *str, UINT8 len, CHAR delimiter, BOOL addMilliSec)
{
	if (len < 13) return 1; // "hh:mm:ss.xxx" (not really safe if used incorrectly)

	int tempLen;
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	if (delimiter != 0) {
		tempLen = sprintf(str, "%02d%c%02d%c%02d", st.wHour, delimiter, st.wMinute, delimiter, st.wSecond);
	}
	else {
		tempLen = sprintf(str, "%02d%02d%02d", st.wHour, st.wMinute, st.wSecond);
	}

	if (addMilliSec) {
		sprintf(str+tempLen, ".%03d", st.wMilliseconds);
	}
	return NO_ERROR;
}

// the code in the two following functions was taken almost unchanged (including comments) from:
// http://stackoverflow.com/questions/1530760/how-do-i-recursively-create-a-folder-in-win32
int isfexist(char *fn)
{
    struct _stat stbuf;
    extern int errno;

    if (_stat(fn, &stbuf)) {
        if (errno == ENOENT) return(0);
        else {
            printf("isfexist: stat");
            return(0);
        }
    } else {
        if (stbuf.st_mode & _S_IFDIR) return(2);
        else return(1);
    }
}

void BBUtMakeDirTree(char *path)
{
    char *end1, *end2;

    if (path[0] == '\\') end1 = path + 1;       // Case '\aa\bb'
    else if (path[1] == ':' && path[2] == '\\') end1 = path + 3;    // Case 'C:\\aa\\bb'
    else end1 = path;

    for(;;) {
        end2 = strchr(end1, '\\');
        if (end2 == NULL) {
            // Case '\aa\bb\'
            if (*end1 == 0) break;
            // Last segment '\aa\bb\"cc"' not yet proceed
        } else *end2 = 0;
        if (isfexist(path) <= 0) _mkdir(path);
        if (end2 == NULL) break;    // Last segment finished
        else {
            *end2 = '\\';
            end1 = end2 + 1;
        }
    }
}
