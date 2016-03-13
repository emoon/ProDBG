/* time.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API tick_t         time_current( void );
FOUNDATION_API tick_t         time_diff( const tick_t from, const tick_t to );
FOUNDATION_API deltatime_t    time_elapsed( const tick_t since );
FOUNDATION_API tick_t         time_elapsed_ticks( const tick_t since );
FOUNDATION_API tick_t         time_ticks_per_second( void );
FOUNDATION_API deltatime_t    time_ticks_to_seconds( const tick_t dt );
FOUNDATION_API tick_t         time_startup( void );
FOUNDATION_API tick_t         time_system( void );

