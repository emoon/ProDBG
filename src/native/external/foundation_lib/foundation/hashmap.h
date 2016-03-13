/* hashmap.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API hashmap_t*      hashmap_allocate( unsigned int buckets, unsigned int bucketsize );
FOUNDATION_API void            hashmap_deallocate( hashmap_t* map );

FOUNDATION_API void            hashmap_initialize( hashmap_t* map, unsigned int buckets, unsigned int bucketsize );
FOUNDATION_API void            hashmap_finalize( hashmap_t* map );

FOUNDATION_API void*           hashmap_insert( hashmap_t* map, hash_t key, void* value );
FOUNDATION_API void*           hashmap_erase( hashmap_t* map, hash_t key );

FOUNDATION_API void*           hashmap_lookup( hashmap_t* map, hash_t key );
FOUNDATION_API bool            hashmap_has_key( hashmap_t* map, hash_t key );

FOUNDATION_API unsigned int    hashmap_size( hashmap_t* map );

FOUNDATION_API void            hashmap_clear( hashmap_t* map );
