/* bitbuffer.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

FOUNDATION_API bitbuffer_t*  bitbuffer_allocate_buffer( void* buffer, unsigned int size, bool swap );
FOUNDATION_API bitbuffer_t*  bitbuffer_allocate_stream( stream_t* stream );
FOUNDATION_API void          bitbuffer_deallocate( bitbuffer_t* bitbuffer );

FOUNDATION_API void          bitbuffer_initialize_buffer( bitbuffer_t* bitbuffer, void* buffer, unsigned int size, bool swap );
FOUNDATION_API void          bitbuffer_initialize_stream( bitbuffer_t* bitbuffer, stream_t* stream );
FOUNDATION_API void          bitbuffer_finalize( bitbuffer_t* bitbuffer );

FOUNDATION_API uint32_t      bitbuffer_read32( bitbuffer_t* bitbuffer, unsigned int bits );
FOUNDATION_API uint64_t      bitbuffer_read64( bitbuffer_t* bitbuffer, unsigned int bits );
FOUNDATION_API uint128_t     bitbuffer_read128( bitbuffer_t* bitbuffer, unsigned int bits );
FOUNDATION_API float32_t     bitbuffer_read_float32( bitbuffer_t* bitbuffer );
FOUNDATION_API float64_t     bitbuffer_read_float64( bitbuffer_t* bitbuffer );

FOUNDATION_API void          bitbuffer_write32( bitbuffer_t* bitbuffer, uint32_t value, unsigned int bits );
FOUNDATION_API void          bitbuffer_write64( bitbuffer_t* bitbuffer, uint64_t value, unsigned int bits );
FOUNDATION_API void          bitbuffer_write128( bitbuffer_t* bitbuffer, uint128_t value, unsigned int bits );
FOUNDATION_API void          bitbuffer_write_float32( bitbuffer_t* bitbuffer, float32_t value );
FOUNDATION_API void          bitbuffer_write_float64( bitbuffer_t* bitbuffer, float64_t value );

FOUNDATION_API void          bitbuffer_align_read( bitbuffer_t* bitbuffer, bool force );
FOUNDATION_API void          bitbuffer_align_write( bitbuffer_t* bitbuffer, bool force );

FOUNDATION_API void          bitbuffer_discard_read( bitbuffer_t* bitbuffer );
FOUNDATION_API void          bitbuffer_discard_write( bitbuffer_t* bitbuffer );

