/* fs.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

struct regex_t;

FOUNDATION_API stream_t*        fs_open_file( const char* path, unsigned int mode );
FOUNDATION_API void             fs_copy_file( const char* source, const char* dest );
FOUNDATION_API bool             fs_remove_file( const char* path );
FOUNDATION_API bool             fs_is_file( const char* path );
FOUNDATION_API bool             fs_make_directory( const char* path );
FOUNDATION_API bool             fs_remove_directory( const char* path );
FOUNDATION_API bool             fs_is_directory( const char* path );
FOUNDATION_API uint64_t         fs_last_modified( const char* path );
FOUNDATION_API void             fs_touch( const char* path );
FOUNDATION_API uint128_t        fs_md5( const char* path );
FOUNDATION_API char**           fs_matching_files( const char* path, const char* pattern, bool recurse );
FOUNDATION_API char**           fs_matching_files_regex( const char* path, struct regex_t* pattern, bool recurse );
FOUNDATION_API char**           fs_files( const char* path );
FOUNDATION_API char**           fs_subdirs( const char* path );
FOUNDATION_API void             fs_monitor( const char* path );
FOUNDATION_API void             fs_unmonitor( const char* path );
FOUNDATION_API stream_t*        fs_temporary_file( void );
FOUNDATION_API void             fs_post_event( foundation_event_id id, const char* path, unsigned int pathlen );
FOUNDATION_API event_stream_t*  fs_event_stream( void );

