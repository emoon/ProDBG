/* semaphore.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API void   semaphore_initialize( semaphore_t* semaphore, unsigned int value );
FOUNDATION_API void   semaphore_initialize_named( semaphore_t* semaphore, const char* name, unsigned int value );
FOUNDATION_API void   semaphore_finalize( semaphore_t* semaphore );

FOUNDATION_API bool   semaphore_wait( semaphore_t* semaphore );
FOUNDATION_API bool   semaphore_try_wait( semaphore_t* semaphore, int milliseconds );
FOUNDATION_API void   semaphore_post( semaphore_t* semaphore );

