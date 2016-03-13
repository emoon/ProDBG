/* config.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API bool         config_bool( hash_t section, hash_t key );
FOUNDATION_API int64_t      config_int( hash_t section, hash_t key );
FOUNDATION_API real         config_real( hash_t section, hash_t key );
FOUNDATION_API const char*  config_string( hash_t section, hash_t key );
FOUNDATION_API hash_t       config_string_hash( hash_t section, hash_t key );

FOUNDATION_API void         config_set_bool( hash_t section, hash_t key, bool value );
FOUNDATION_API void         config_set_int( hash_t section, hash_t key, int64_t value );
FOUNDATION_API void         config_set_real( hash_t section, hash_t key, real value );
FOUNDATION_API void         config_set_string( hash_t section, hash_t key, const char* value );
FOUNDATION_API void         config_set_string_constant( hash_t section, hash_t key, const char* value );

FOUNDATION_API void         config_load( const char* name, hash_t filter_section, bool built_in, bool overwrite );
FOUNDATION_API void         config_parse( stream_t* stream, hash_t filter_section, bool overwrite );
FOUNDATION_API void         config_write( stream_t* stream, hash_t filter_section, const char* (*string_mapper)( hash_t ) );

FOUNDATION_API void         config_parse_commandline( const char* const* cmdline, unsigned int num );
