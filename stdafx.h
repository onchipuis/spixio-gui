#ifndef STDAFX_H
#define STDAFX_H

/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<inttypes.h>
#include<gtk/gtk.h>
#include<math.h>
/* OS specific libraries */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinNT.h>
#include <Shlobj.h>
#include <tchar.h>
#include <direct.h>
#endif

#ifdef __linux
#include<dlfcn.h>
#endif

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX_PATH 260

#endif // STDAFX_H
