/* process.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API process_t*  process_allocate( void );
FOUNDATION_API void        process_deallocate( process_t* proc );

FOUNDATION_API void        process_initialize( process_t* proc );
FOUNDATION_API void        process_finalize( process_t* proc );

FOUNDATION_API void        process_set_working_directory( process_t* proc, const char* path );
FOUNDATION_API void        process_set_executable_path( process_t* proc, const char* path );
FOUNDATION_API void        process_set_arguments( process_t* proc, const char** args, unsigned int num );
FOUNDATION_API void        process_set_flags( process_t* proc, unsigned int flags );
FOUNDATION_API void        process_set_verb( process_t* proc, const char* verb );
FOUNDATION_API void        process_set_exit_code( int code );

FOUNDATION_API int         process_spawn( process_t* proc );

FOUNDATION_API stream_t*   process_stdout( process_t* proc );
FOUNDATION_API stream_t*   process_stdin( process_t* proc );

FOUNDATION_API int         process_wait( process_t* proc );

FOUNDATION_API int         process_exit_code( void );
FOUNDATION_API void        process_exit( int code ) FOUNDATION_ATTRIBUTE( noreturn );
