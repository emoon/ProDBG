/* random.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API uint32_t     random32( void );
FOUNDATION_API uint32_t     random32_range( uint32_t low, uint32_t high );
FOUNDATION_API uint64_t     random64( void );
FOUNDATION_API uint64_t     random64_range( uint64_t low, uint64_t high );
FOUNDATION_API real         random_normalized( void );
FOUNDATION_API real         random_range( real low, real high );
FOUNDATION_API int32_t      random32_gaussian_range( int32_t low, int32_t high );
FOUNDATION_API real         random_gaussian_range( real low, real high );
FOUNDATION_API int32_t      random32_triangle_range( int32_t low, int32_t high );
FOUNDATION_API real         random_triangle_range( real low, real high );
FOUNDATION_API uint32_t     random32_weighted( uint32_t limit, const real* weights );

FOUNDATION_API void         random_thread_deallocate( void );

