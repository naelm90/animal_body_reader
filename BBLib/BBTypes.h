// ..\CommonBB\Code\BBTypes.h ****
#pragma once

#define TRUE    1
#define FALSE   0

typedef signed char		BBRETCODE;

// macro
#define BBPI				3.1415926535898
#define BBRAD2DEG			((double)180.0 / (double)BBPI)
#define BBDEG2RAD			(double)((double)BBPI    / (double)180.0)


#ifndef _WIN32
////// MCU-only part ///////

#include <stdint.h>	// needed for TivaWare libraries

#define NO_ERROR 0
#define MAXWORD 0xFFFF

typedef char				CHAR;
typedef char				INT8;
typedef unsigned char		UINT8;
typedef unsigned char		BYTE;       
typedef unsigned char		BOOL;
#ifndef __cplusplus
typedef unsigned char		bool;   
#endif
typedef unsigned char		tBoolean;
typedef unsigned short		WORD;
typedef unsigned long		DWORD;
typedef unsigned int		UINT32;       
typedef int					INT32;        
typedef long				LONG32;       
typedef unsigned long		ULONG32;      
typedef short				SHORT16;      
typedef unsigned short		USHORT16;
typedef short           	INT16;      
typedef unsigned short  	UINT16;
typedef long long			LONG64;
typedef unsigned long long	ULONG64;
typedef long long			INT64;
typedef unsigned long long	UINT64;

typedef void				*PVOID;
typedef BYTE				*PBYTE;
typedef UINT32				*PUINT32;
typedef INT32				*PINT32;                 
typedef LONG32			    *PLONG32;
typedef ULONG32				*PULONG32;
typedef SHORT16				*PSHORT16;
typedef USHORT16		    *PUSHORT16;
typedef INT16           	*PINT16;       
typedef UINT16          	*PUINT16;
#ifndef _MCU
typedef void				FILE;
#endif
#define _FUNC_ ""


#include <string.h>
#include "driverlib/rom.h"

#define true    TRUE
#define false   FALSE

////// MACRO //////

#define BBEnterCriticalSection()	(bEnableInt = ROM_IntMasterDisable() ? false : true)
#define BBExitCriticalSection()		if(bEnableInt) ROM_IntMasterEnable()

#define PRINT_LAST_ERROR(failedActionStr) { } // TODO: define using strerror(errno)

#else
////// Windows-only part ///////

#include <windows.h>

#define _FUNC_ "[" __FUNCTION__ "] "

#define PRINT_LAST_ERROR(failedActionStr) { char lastErrorStr[300]; \
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_NEUTRAL, lastErrorStr, 300, NULL); \
		printf(_FUNC_ "ERROR: %s failed - %s\n", failedActionStr, lastErrorStr); }

#endif
