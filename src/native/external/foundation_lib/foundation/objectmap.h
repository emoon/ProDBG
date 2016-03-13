/* objectmap.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API objectmap_t*         objectmap_allocate( unsigned int size );
FOUNDATION_API void                 objectmap_deallocate( objectmap_t* map );

FOUNDATION_API void                 objectmap_initialize( objectmap_t* map, unsigned int size );
FOUNDATION_API void                 objectmap_finalize( objectmap_t* map );

FOUNDATION_API unsigned int         objectmap_size( const objectmap_t* map );
FOUNDATION_API object_t             objectmap_reserve( objectmap_t* map );
FOUNDATION_API void                 objectmap_free( objectmap_t* map, object_t id );
FOUNDATION_API void                 objectmap_set( objectmap_t* map, object_t id, void* object );

FOUNDATION_API void*                objectmap_raw_lookup( const objectmap_t* map, unsigned int index );
FOUNDATION_API void*                objectmap_lookup_ref( const objectmap_t* map, object_t id );
FOUNDATION_API bool                 objectmap_lookup_unref( const objectmap_t* map, object_t id, object_deallocate_fn deallocate );

static FOUNDATION_FORCEINLINE FOUNDATION_PURECALL void* objectmap_lookup( const objectmap_t* map, object_t id )
{
	void* object = map->map[ id & map->mask_index ];
	return ( object && !( (uintptr_t)object & 1 ) &&
			( ( *( (uint64_t*)object + 1 ) & map->mask_id ) == ( id & map->mask_id ) ) ? //ID in object is offset by 8 bytes
			object : 0 );
}
