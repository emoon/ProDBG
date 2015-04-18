/* path.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API char*      path_base_file_name( const char* path );
FOUNDATION_API char*      path_base_file_name_with_path( const char* path );
FOUNDATION_API char*      path_file_extension( const char* path );
FOUNDATION_API char*      path_file_name( const char* path );
FOUNDATION_API char*      path_directory_name( const char* path );
FOUNDATION_API char*      path_subdirectory_name( const char* path, const char* root );
FOUNDATION_API char*      path_protocol( const char* uri );

FOUNDATION_API char*      path_merge( const char* first, const char* second );
FOUNDATION_API char*      path_append( char* base, const char* tail );
FOUNDATION_API char*      path_prepend( char* tail, const char* base );
FOUNDATION_API char*      path_clean( char* path, bool absolute );

FOUNDATION_API bool       path_is_absolute( const char* path );

FOUNDATION_API char*      path_make_absolute( const char* path );
FOUNDATION_API char*      path_make_temporary( void );

