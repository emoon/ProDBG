/* stream.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


FOUNDATION_API stream_t*         stream_open( const char* path, unsigned int mode );
FOUNDATION_API stream_t*         stream_clone( stream_t* stream );
FOUNDATION_API void              stream_deallocate( stream_t* stream );
FOUNDATION_API void              stream_initialize( stream_t* stream, byteorder_t order );
FOUNDATION_API void              stream_finalize( stream_t* stream );

FOUNDATION_API int64_t           stream_tell( stream_t* stream );
FOUNDATION_API void              stream_seek( stream_t* stream, int64_t offset, stream_seek_mode_t direction );
FOUNDATION_API bool              stream_eos( stream_t* stream );
FOUNDATION_API uint64_t          stream_size( stream_t* stream );

FOUNDATION_API void              stream_set_byteorder( stream_t* stream, byteorder_t byteorder );
FOUNDATION_API void              stream_set_binary( stream_t* stream, bool binary );
FOUNDATION_API void              stream_determine_binary_mode( stream_t* stream, unsigned int num );

FOUNDATION_API bool              stream_is_binary( const stream_t* stream );
FOUNDATION_API bool              stream_is_sequential( const stream_t* stream );
FOUNDATION_API bool              stream_is_reliable( const stream_t* stream );
FOUNDATION_API bool              stream_is_inorder( const stream_t* stream );
FOUNDATION_API bool              stream_is_swapped( const stream_t* stream );

FOUNDATION_API byteorder_t       stream_byteorder( const stream_t* stream );
FOUNDATION_API const char*       stream_path( const stream_t* stream );
FOUNDATION_API uint64_t          stream_last_modified( const stream_t* stream );

FOUNDATION_API uint64_t          stream_read( stream_t* stream, void* buffer, uint64_t num_bytes );
FOUNDATION_API uint64_t          stream_read_line_buffer( stream_t* stream, char* dest, unsigned int count, char delimiter );
FOUNDATION_API char*             stream_read_line( stream_t* stream, char delimiter );
FOUNDATION_API bool              stream_read_bool( stream_t* stream );
FOUNDATION_API int8_t            stream_read_int8( stream_t* stream );
FOUNDATION_API uint8_t           stream_read_uint8( stream_t* stream );
FOUNDATION_API int16_t           stream_read_int16( stream_t* stream );
FOUNDATION_API uint16_t          stream_read_uint16( stream_t* stream );
FOUNDATION_API int32_t           stream_read_int32( stream_t* stream );
FOUNDATION_API uint32_t          stream_read_uint32( stream_t* stream );
FOUNDATION_API int64_t           stream_read_int64( stream_t* stream );
FOUNDATION_API uint64_t          stream_read_uint64( stream_t* stream );
FOUNDATION_API float32_t         stream_read_float32( stream_t* stream );
FOUNDATION_API float64_t         stream_read_float64( stream_t* stream );
FOUNDATION_API char*             stream_read_string( stream_t* stream );
FOUNDATION_API uint64_t          stream_read_string_buffer( stream_t* stream, char* buffer, uint64_t size );

FOUNDATION_API uint64_t          stream_write( stream_t* stream, const void* buffer, uint64_t num_bytes );
FOUNDATION_API void              stream_write_bool( stream_t* stream, bool data );
FOUNDATION_API void              stream_write_int8( stream_t* stream, int8_t data );
FOUNDATION_API void              stream_write_uint8( stream_t* stream, uint8_t data );
FOUNDATION_API void              stream_write_int16( stream_t* stream, int16_t data );
FOUNDATION_API void              stream_write_uint16( stream_t* stream, uint16_t data );
FOUNDATION_API void              stream_write_int32( stream_t* stream, int32_t data );
FOUNDATION_API void              stream_write_uint32( stream_t* stream, uint32_t data );
FOUNDATION_API void              stream_write_int64( stream_t* stream, int64_t data );
FOUNDATION_API void              stream_write_uint64( stream_t* stream, uint64_t data );
FOUNDATION_API void              stream_write_float32( stream_t* stream, float32_t data );
FOUNDATION_API void              stream_write_float64( stream_t* stream, float64_t data );
FOUNDATION_API void              stream_write_string( stream_t* stream, const char* data );
FOUNDATION_API void              stream_write_endl( stream_t* stream );
FOUNDATION_API void              stream_write_format( stream_t* stream, const char* format, ... );

FOUNDATION_API void              stream_buffer_read( stream_t* stream );
FOUNDATION_API unsigned int      stream_available_read( stream_t* stream );

FOUNDATION_API uint128_t         stream_md5( stream_t* stream );

FOUNDATION_API void              stream_truncate( stream_t* stream, uint64_t length );
FOUNDATION_API void              stream_flush( stream_t* stream );

FOUNDATION_API stream_t*         stream_open_stdout( void );
FOUNDATION_API stream_t*         stream_open_stderr( void );
FOUNDATION_API stream_t*         stream_open_stdin( void );

FOUNDATION_API void              stream_set_protocol_handler( const char* protocol, stream_open_fn fn );
FOUNDATION_API stream_open_fn    stream_protocol_handler( const char* protocol, unsigned int length );
