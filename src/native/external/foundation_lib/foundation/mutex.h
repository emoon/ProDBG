/* mutex.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API mutex_t*      mutex_allocate( const char* name );
FOUNDATION_API void          mutex_deallocate( mutex_t* mutex );

FOUNDATION_API const char*   mutex_name( mutex_t* mutex );
FOUNDATION_API bool          mutex_try_lock( mutex_t* mutex );
FOUNDATION_API bool          mutex_lock( mutex_t* mutex );
FOUNDATION_API bool          mutex_unlock( mutex_t* mutex );
FOUNDATION_API bool          mutex_wait( mutex_t* mutex, unsigned int timeout );
FOUNDATION_API void          mutex_signal( mutex_t* mutex );

#if FOUNDATION_PLATFORM_WINDOWS

FOUNDATION_API void*         mutex_event_object( mutex_t* mutex );

#endif
