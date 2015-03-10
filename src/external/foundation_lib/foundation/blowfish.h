/* blowfish.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API blowfish_t*  blowfish_allocate( void );
FOUNDATION_API void         blowfish_deallocate( blowfish_t* blowfish );

FOUNDATION_API void         blowfish_initialize( blowfish_t* blowfish, const void* key, const unsigned int length );
FOUNDATION_API void         blowfish_finalize( blowfish_t* blowfish );

FOUNDATION_API void         blowfish_encrypt( const blowfish_t* blowfish, void* data, unsigned int length, const blowfish_mode_t mode, const uint64_t vec );
FOUNDATION_API void         blowfish_decrypt( const blowfish_t* blowfish, void* data, unsigned int length, const blowfish_mode_t mode, const uint64_t vec );
