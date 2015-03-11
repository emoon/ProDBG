/* bufferstream.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API stream_t*  buffer_stream_allocate( void* buffer, unsigned int mode, uint64_t size, uint64_t capacity, bool adopt, bool grow );
FOUNDATION_API void       buffer_stream_initialize( stream_buffer_t* stream, void* buffer, unsigned int mode, uint64_t size, uint64_t capacity, bool adopt, bool grow );
