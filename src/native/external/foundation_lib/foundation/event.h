/* event.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

FOUNDATION_API void                 event_post( event_stream_t* stream, uint16_t id, uint16_t size, object_t object, const void* payload, tick_t delivery );
FOUNDATION_API event_t*             event_next( const event_block_t* block, event_t* event );
FOUNDATION_API uint16_t             event_payload_size( const event_t* event );

FOUNDATION_API event_stream_t*      event_stream_allocate( unsigned int size );
FOUNDATION_API void                 event_stream_deallocate( event_stream_t* stream );

FOUNDATION_API void                 event_stream_initialize( event_stream_t* stream, unsigned int size );
FOUNDATION_API void                 event_stream_finalize( event_stream_t* stream );

FOUNDATION_API event_block_t*       event_stream_process( event_stream_t* stream );

