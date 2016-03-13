/* windows.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
 *
 * This library provides a cross-platform foundation library in C11 providing basic support data types and
 * functions to write applications and games in a platform-independent fashion. The latest source code is
 * always available at
 *
 * https://github.com/rampantpixels/foundation_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any restrictions.
 *
 */

#pragma once

#include <foundation/platform.h>
#include <foundation/types.h>


#if FOUNDATION_PLATFORM_WINDOWS

#undef IN
#undef OUT
#undef far
#undef near
#undef FAR
#undef NEAR

#define IN
#define OUT
#define far
#define near
#define FAR
#define NEAR
#define STREAM_SEEK_END _STREAM_SEEK_END

#define UUID_DEFINED 1
#define UUID uuid_t

#if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
__MINGW_EXTENSION unsigned __int64 __readgsqword(unsigned __LONG32 Offset);
#define __INTRINSIC_DEFINED___readgsqword
#endif

#define WIN32_LEAN_AND_MEAN
//Work around broken dbghlp.h header
#pragma warning( disable : 4091 )

#include <windows.h>
#include <winsock2.h>
#include <iptypes.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
//#include <mmsystem.h>
//#include <mmreg.h>
#include <share.h>
#include <io.h>
#include <shellapi.h>
#include <dbghelp.h>
#include <stdlib.h>
#if FOUNDATION_COMPILER_MSVC
//From shlobj.h
EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE SHGetFolderPathW(__reserved HWND hwnd, __in int csidl, __in_opt HANDLE hToken, __in DWORD dwFlags, __out_ecount(MAX_PATH) LPWSTR pszPath);
#  define CSIDL_LOCAL_APPDATA             0x001c        // <user name>\Local Settings\Application Data (non roaming)
#else
#  include <shlobj.h>
#endif

#include <crtdbg.h>

#undef ERROR
#undef min
#undef max
#undef IN
#undef OUT
#undef far
#undef near
#undef FAR
#undef NEAR
#undef BINARY
#undef LITTLEENDIAN
#undef BIGENDIAN
#undef STREAM_SEEK_END

#if FOUNDATION_COMPILER_CLANG
#  undef WINAPI
#  define WINAPI STDCALL
#endif

#endif
