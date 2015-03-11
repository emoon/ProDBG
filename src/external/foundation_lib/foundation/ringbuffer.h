/* ringbuffer.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API ringbuffer_t*   ringbuffer_allocate( unsigned int size );
FOUNDATION_API void            ringbuffer_deallocate( ringbuffer_t* buffer );

FOUNDATION_API void            ringbuffer_initialize( ringbuffer_t* buffer, unsigned int size );
FOUNDATION_API void            ringbuffer_finalize( ringbuffer_t* buffer );

FOUNDATION_API unsigned int    ringbuffer_size( ringbuffer_t* buffer );
FOUNDATION_API void            ringbuffer_reset( ringbuffer_t* buffer );

FOUNDATION_API unsigned int    ringbuffer_read( ringbuffer_t* buffer, void* dest, unsigned int num );
FOUNDATION_API unsigned int    ringbuffer_write( ringbuffer_t* buffer, const void* source, unsigned int num );

FOUNDATION_API uint64_t        ringbuffer_total_read( ringbuffer_t* buffer );
FOUNDATION_API uint64_t        ringbuffer_total_written( ringbuffer_t* buffer );

FOUNDATION_API stream_t*       ringbuffer_stream_allocate( unsigned int buffer_size, uint64_t total_size );
FOUNDATION_API void            ringbuffer_stream_initialize( stream_ringbuffer_t* stream, unsigned int buffer_size, uint64_t total_size );
