/* tizen.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


#if FOUNDATION_PLATFORM_TIZEN

#ifndef __error_t_defined
#define __error_t_defined 1
#endif

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <appfw/app.h>
#include <system/system_settings.h>
#include <dlog/dlog.h>

FOUNDATION_API int       tizen_initialize( void );
FOUNDATION_API void      tizen_shutdown( void );
FOUNDATION_API void      tizen_start_main_thread( void );
FOUNDATION_API int       tizen_app_main( int argc, char** argv );

#endif
