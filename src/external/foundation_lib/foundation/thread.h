/* thread.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API object_t        thread_create( thread_fn fn, const char* name, thread_priority_t priority, unsigned int stacksize );
FOUNDATION_API object_t        thread_ref( object_t thread );
FOUNDATION_API void            thread_destroy( object_t thread );

FOUNDATION_API bool            thread_start( object_t thread, void* data );
FOUNDATION_API void            thread_terminate( object_t thread );

FOUNDATION_API bool            thread_is_started( object_t thread );
FOUNDATION_API bool            thread_is_running( object_t thread );
FOUNDATION_API bool            thread_is_thread( object_t thread );
FOUNDATION_API bool            thread_is_main( void );
FOUNDATION_API bool            thread_should_terminate( object_t thread );

FOUNDATION_API void            thread_set_main( void );
FOUNDATION_API void            thread_set_name( const char* name );
FOUNDATION_API void            thread_set_hardware( uint64_t mask );

FOUNDATION_API void*           thread_result( object_t thread );
FOUNDATION_API object_t        thread_self( void );
FOUNDATION_API const char*     thread_name( void );
FOUNDATION_API uint64_t        thread_id( void );
FOUNDATION_API unsigned int    thread_hardware( void );

FOUNDATION_API void            thread_sleep( int milliseconds );
FOUNDATION_API void            thread_yield( void );

FOUNDATION_API void            thread_finalize( void );

#if FOUNDATION_PLATFORM_ANDROID

FOUNDATION_API void*           thread_attach_jvm( void );
FOUNDATION_API void            thread_detach_jvm( void );

#endif
