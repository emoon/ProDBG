/* profile.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API void profile_initialize( const char* identifier, void* buffer, uint64_t size );
FOUNDATION_API void profile_shutdown( void );
FOUNDATION_API void profile_enable( bool enable );

FOUNDATION_API void profile_set_output( profile_write_fn writer );
FOUNDATION_API void profile_set_output_wait( int ms );

FOUNDATION_API void profile_end_frame( uint64_t counter );
FOUNDATION_API void profile_begin_block( const char* message );
FOUNDATION_API void profile_update_block( void );
FOUNDATION_API void profile_end_block( void );

FOUNDATION_API void profile_log( const char* message );
FOUNDATION_API void profile_trylock( const char* name );
FOUNDATION_API void profile_lock( const char* name );
FOUNDATION_API void profile_unlock( const char* name );
FOUNDATION_API void profile_wait( const char* name );
FOUNDATION_API void profile_signal( const char* name );


#if !BUILD_ENABLE_PROFILE

#define profile_initialize( identifier, buffer, size ) do { (void)sizeof( identifier ); (void)sizeof( buffer ); (void)sizeof( size ); } while(0)
#define profile_shutdown() do {} while(0)

#define profile_enable( enable ) do { (void)sizeof( enable ); } while(0)
#define profile_set_output( writer ) do { (void)writer; } while(0)
#define profile_set_output_wait( ms ) do{ (void)sizeof( ms ); } while(0)

#define profile_end_frame( counter ) do { (void)sizeof( counter ); } while(0)
#define profile_begin_block( msg ) do { (void)sizeof( msg ); } while(0)
#define profile_update_block() do {} while(0)
#define profile_end_block() do {} while(0)
#define profile_log( message ) do { (void)sizeof( message ); } while(0)
#define profile_trylock( name ) do { (void)sizeof( name ); } while(0)
#define profile_lock( name ) do { (void)sizeof( name ); } while(0)
#define profile_unlock( name ) do { (void)sizeof( name ); } while(0)
#define profile_wait( name ) do { (void)sizeof( name ); } while(0)
#define profile_signal( name ) do { (void)sizeof( name ); } while(0)

#endif
