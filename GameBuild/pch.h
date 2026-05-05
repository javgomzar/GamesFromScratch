// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// C RunTime Header Files
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <string>
#include <cstring>
#include <format>
#include <math.h>
#include <float.h>

// XXHash library - non cryptographic hash
#include <xxhash.h>

#ifdef _WIN32
// Windows Header Files
/*  
    Including SDKDDKVer.h defines the highest available Windows platform.
    If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
    set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
*/
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

// ANSI & Unicode text
#include <tchar.h>

// Process status API
#include <psapi.h>

#ifndef _DEBUG
#include <wincrypt.h>
#endif

#include <XInput.h>
#include <xaudio2.h>

#include "Resource.h"
#elif __linux__
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <linux/input.h>
#include <dirent.h>
#include <cerrno>
#endif

#endif
