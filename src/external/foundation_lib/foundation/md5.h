/* md5.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API md5_t*        md5_allocate( void );
FOUNDATION_API void          md5_deallocate( md5_t* digest );

FOUNDATION_API void          md5_initialize( md5_t* digest );
FOUNDATION_API void          md5_finalize( md5_t* digest );

FOUNDATION_API md5_t*        md5_digest( md5_t* digest, const char* message );
FOUNDATION_API md5_t*        md5_digest_raw( md5_t* digest, const void* buffer, size_t size );
FOUNDATION_API void          md5_digest_finalize( md5_t* digest );
FOUNDATION_API char*         md5_get_digest( const md5_t* digest );
FOUNDATION_API uint128_t     md5_get_digest_raw( const md5_t* digest );
