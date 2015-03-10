/* regex.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API regex_t*  regex_compile( const char* pattern );
FOUNDATION_API bool      regex_match( regex_t* regex, const char* input, int inlength, regex_capture_t* captures, int maxcaptures );
FOUNDATION_API void      regex_deallocate( regex_t* regex );

