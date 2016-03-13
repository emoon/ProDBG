/* android.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


#if FOUNDATION_PLATFORM_ANDROID

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <android_native_app_glue.h>


FOUNDATION_API void                  android_entry( struct android_app* app );
FOUNDATION_API int                   android_initialize( void );
FOUNDATION_API void                  android_shutdown( void );
FOUNDATION_API struct android_app*   android_app( void );
FOUNDATION_API void                  android_handle_cmd( struct android_app* app, int32_t cmd );
FOUNDATION_API int                   android_sensor_callback( int fd, int events, void* data );

#endif
