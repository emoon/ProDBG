/* uuid.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API const uuid_t           UUID_DNS;

FOUNDATION_API uuid_t                 uuid_generate_time( void );
FOUNDATION_API uuid_t                 uuid_generate_name( const uuid_t ns, const char* name );
FOUNDATION_API uuid_t                 uuid_generate_random( void );

static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL bool     uuid_equal( const uuid_t u0, const uuid_t u1 );
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL uuid_t   uuid_null( void );
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL bool     uuid_is_null( const uuid_t uuid );


static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL bool uuid_equal( const uuid_t u0, const uuid_t u1 )
{
	return uint128_equal( u0, u1 );
}


static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL uuid_t uuid_null( void )
{
	return uint128_make( 0, 0 );
}


static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL bool uuid_is_null( const uuid_t uuid )
{
	return uint128_is_null( uuid );
}
