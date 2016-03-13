/* internal.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

/*! \file internal.h
    Internal types */

#include <foundation/platform.h>
#include <foundation/types.h>


//Internal entry points

FOUNDATION_API int _atomic_initialize( void );
FOUNDATION_API void _atomic_shutdown( void );

FOUNDATION_API void _ringbuffer_stream_initialize( void );
FOUNDATION_API void _buffer_stream_initialize( void );
#if FOUNDATION_PLATFORM_ANDROID
FOUNDATION_API void _asset_stream_initialize( void );
#endif
FOUNDATION_API void _pipe_stream_initialize( void );

FOUNDATION_API int _log_initialize( void );
FOUNDATION_API void _log_shutdown( void );

FOUNDATION_API int _memory_initialize( const memory_system_t memory );
FOUNDATION_API void _memory_preallocate( void );
FOUNDATION_API void _memory_shutdown( void );

FOUNDATION_API int _time_initialize( void );
FOUNDATION_API void _time_shutdown( void );

FOUNDATION_API int _thread_initialize( void );
FOUNDATION_API void _thread_shutdown( void );

FOUNDATION_API int _environment_initialize( const application_t application );
FOUNDATION_API void _environment_shutdown( void );

FOUNDATION_API int _library_initialize( void );
FOUNDATION_API void _library_shutdown( void );

FOUNDATION_API int _system_initialize( void );
FOUNDATION_API void _system_shutdown( void );

FOUNDATION_API int _stream_initialize( void );
FOUNDATION_API void _stream_shutdown( void );

FOUNDATION_API int _fs_initialize( void );
FOUNDATION_API void _fs_shutdown( void );

FOUNDATION_API int _random_initialize( void );
FOUNDATION_API void _random_shutdown( void );

FOUNDATION_API int _config_initialize( void );
FOUNDATION_API void _config_shutdown( void );

FOUNDATION_API void _profile_thread_finalize( void );

FOUNDATION_API int _static_hash_initialize( void );
FOUNDATION_API void _static_hash_shutdown( void );

FOUNDATION_API void foundation_startup( void );


// Internal functions

FOUNDATION_API uint32_t      hashtable32_raw( hashtable32_t* table, uint32_t key );
FOUNDATION_API uint64_t      hashtable64_raw( hashtable64_t* table, uint64_t key );

FOUNDATION_API void          _object_initialize( object_base_t* obj, object_t id );
FOUNDATION_API object_t      _object_ref( object_base_t* obj );
FOUNDATION_API object_t      _object_unref( object_base_t* obj );

FOUNDATION_API void          _environment_main_args( int argc, const char* const* argv );

