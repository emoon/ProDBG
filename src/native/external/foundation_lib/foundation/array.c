/* array.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/foundation.h>


//'FARR' in ascii
static const int ARRAY_WATERMARK = 0x52524145;

static const unsigned int ARRAY_DEFAULT_ALIGN = 16U;


const void* _array_verifyfn( const void* const* arr )
{
	if( !FOUNDATION_VALIDATE_MSG( !(*arr) || ( _array_raw_const(*arr)[2] == ARRAY_WATERMARK ), "Invalid array (bad watermark)" ) )
		return 0;
	if( !FOUNDATION_VALIDATE_MSG( !(*arr) || ( _array_raw_const(*arr)[1] <= _array_raw_const(*arr)[0] ), "Invalid array (size > capacity)" ) )
		return 0;
	return *arr;
}


void* _array_resizefn( void** arr, int elements, int itemsize )
{
	if( elements > 0 )
	{
		if( !(*arr) )
			_array_growfn( arr, elements, 1, itemsize );
		else if( _array_rawcapacity( *arr ) < elements )
			_array_growfn( arr, elements - _array_rawcapacity( *arr ), 1, itemsize );
	}
	else
	{
		elements = 0;
	}
	if( *arr )
		_array_rawsize( *arr ) = elements;
	return *arr;
}


void* _array_growfn( void** arr, int increment, int factor, int itemsize )
{
	int      prev_capacity = *arr ? _array_rawcapacity( *arr ) : 0;
	int      capacity = *arr ? ( factor * prev_capacity + increment ) : increment;
	int      prev_used_size = itemsize * prev_capacity;
	int      storage_size = itemsize * capacity;
	uint64_t header_size = 4ULL * _array_header_size;
	uint64_t prev_used_buffer_size = (unsigned int)prev_used_size + header_size;
	uint64_t buffer_size = (unsigned int)storage_size + header_size;
	int*     buffer = *arr ? memory_reallocate( _array_raw( *arr ), buffer_size, ARRAY_DEFAULT_ALIGN, prev_used_buffer_size ) : memory_allocate( 0, buffer_size, ARRAY_DEFAULT_ALIGN, MEMORY_PERSISTENT );
	FOUNDATION_ASSERT_MSG( buffer, "Failed to reallocate array storage" );
	if( buffer )
	{
		buffer[0] = capacity;
		if( !*arr )
		{
			buffer[1] = 0;
			buffer[2] = ARRAY_WATERMARK;
			buffer[3] = itemsize;
		}
		*arr = buffer + _array_header_size;
	}
	return *arr;
}
