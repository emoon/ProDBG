/* hashtable.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

FOUNDATION_API hashtable32_t*   hashtable32_allocate( unsigned int buckets );
FOUNDATION_API void             hashtable32_deallocate( hashtable32_t* table );

FOUNDATION_API void             hashtable32_initialize( hashtable32_t* table, unsigned int buckets );
FOUNDATION_API void             hashtable32_finalize( hashtable32_t* table );

FOUNDATION_API void             hashtable32_set( hashtable32_t* table, uint32_t key, uint32_t value );
FOUNDATION_API void             hashtable32_erase( hashtable32_t* table, uint32_t key );
FOUNDATION_API uint32_t         hashtable32_get( hashtable32_t* table, uint32_t key );
FOUNDATION_API unsigned int     hashtable32_size( hashtable32_t* table );
FOUNDATION_API void             hashtable32_clear( hashtable32_t* table );


FOUNDATION_API hashtable64_t*   hashtable64_allocate( unsigned int buckets );
FOUNDATION_API void             hashtable64_deallocate( hashtable64_t* table );

FOUNDATION_API void             hashtable64_initialize( hashtable64_t* table, unsigned int buckets );
FOUNDATION_API void             hashtable64_finalize( hashtable64_t* table );

FOUNDATION_API void             hashtable64_set( hashtable64_t* table, uint64_t key, uint64_t value );
FOUNDATION_API void             hashtable64_erase( hashtable64_t* table, uint64_t key );
FOUNDATION_API uint64_t         hashtable64_get( hashtable64_t* table, uint64_t key );
FOUNDATION_API unsigned int     hashtable64_size( hashtable64_t* table );
FOUNDATION_API void             hashtable64_clear( hashtable64_t* table );


#if FOUNDATION_SIZE_POINTER == 4

#define hashtable_t             hashtable32_t
#define hashtable_allocate      hashtable32_allocate
#define hashtable_initialize    hashtable32_initialize
#define hashtable_finalize      hashtable32_finalize
#define hashtable_deallocate    hashtable32_deallocate
#define hashtable_set           hashtable32_set
#define hashtable_erase         hashtable32_erase
#define hashtable_get           hashtable32_get
#define hashtable_size          hashtable32_size
#define hashtable_clear         hashtable32_clear

#else

#define hashtable_t             hashtable64_t
#define hashtable_allocate      hashtable64_allocate
#define hashtable_initialize    hashtable64_initialize
#define hashtable_finalize      hashtable64_finalize
#define hashtable_deallocate    hashtable64_deallocate
#define hashtable_set           hashtable64_set
#define hashtable_erase         hashtable64_erase
#define hashtable_get           hashtable64_get
#define hashtable_size          hashtable64_size
#define hashtable_clear         hashtable64_clear

#endif
