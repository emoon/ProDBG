/* bitbuffer.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


static void _bitbuffer_get( bitbuffer_t* FOUNDATION_RESTRICT bitbuffer )
{
	if( bitbuffer->buffer < bitbuffer->end )
	{
		void* bufferptr = bitbuffer->buffer; //For alignment required archs, we know it is 32-bit aligned already so safe casts below
		bitbuffer->pending_read = bitbuffer->swap ? byteorder_swap32( *(uint32_t*)bufferptr ) : *(uint32_t*)bufferptr;
		bitbuffer->buffer += 4;
	}
	else if( bitbuffer->stream )
	{
		bitbuffer->pending_read = stream_read_uint32( bitbuffer->stream );
	}
	else
	{
		bitbuffer->pending_read = 0;
	}
	bitbuffer->offset_read = 0;
}


static void _bitbuffer_put( bitbuffer_t* FOUNDATION_RESTRICT bitbuffer )
{
	if( bitbuffer->buffer < bitbuffer->end )
	{
		void* bufferptr = bitbuffer->buffer; //For alignment required archs, we know it is 32-bit aligned already so safe casts below
		*(uint32_t*)bufferptr = bitbuffer->swap ? byteorder_swap32( bitbuffer->pending_write ) : bitbuffer->pending_write;
		bitbuffer->buffer += 4;
	}
	else if( bitbuffer->stream )
	{
		stream_write_uint32( bitbuffer->stream, bitbuffer->pending_write );
	}
	bitbuffer->offset_write = 0;
	bitbuffer->pending_write = 0;
}


bitbuffer_t* bitbuffer_allocate_buffer( void* buffer, unsigned int size, bool swap )
{
	bitbuffer_t* bitbuffer = memory_allocate( 0, sizeof( bitbuffer_t ), 0, MEMORY_PERSISTENT );

	bitbuffer_initialize_buffer( bitbuffer, buffer, size, swap );

	return bitbuffer;
}


bitbuffer_t* bitbuffer_allocate_stream( stream_t* stream )
{
	bitbuffer_t* bitbuffer = memory_allocate( 0, sizeof( bitbuffer_t ), 0, MEMORY_PERSISTENT );

	bitbuffer_initialize_stream( bitbuffer, stream );

	return bitbuffer;
}


void bitbuffer_deallocate( bitbuffer_t* bitbuffer )
{
	bitbuffer_finalize( bitbuffer );
	memory_deallocate( bitbuffer );
}


void bitbuffer_finalize( bitbuffer_t* bitbuffer )
{
	FOUNDATION_UNUSED( bitbuffer );
}


void bitbuffer_initialize_buffer( bitbuffer_t* bitbuffer, void* buffer, unsigned int size, bool swap )
{
	FOUNDATION_ASSERT( !( size % 4 ) );
	FOUNDATION_ASSERT( (uintptr_t)buffer );
	FOUNDATION_ASSERT( !( (uintptr_t)buffer % FOUNDATION_SIZE_POINTER ) );
	memset( bitbuffer, 0, sizeof( bitbuffer_t ) );
	bitbuffer->offset_read = 32;
	bitbuffer->buffer = buffer;
	bitbuffer->end = buffer ? pointer_offset( buffer, size ) : 0;
	bitbuffer->swap = swap;
}


void bitbuffer_initialize_stream( bitbuffer_t* bitbuffer, stream_t* stream )
{
	memset( bitbuffer, 0, sizeof( bitbuffer_t ) );
	bitbuffer->offset_read = 32;
	bitbuffer->stream = stream;

	stream_set_binary( stream, true );
}


uint128_t bitbuffer_read128( bitbuffer_t* bitbuffer, unsigned int bits )
{
	if( bits <= 64 )
	{
#if !FOUNDATION_COMPILER_MSVC
		const uint128_t value = {
			.word[0] = bitbuffer_read64( bitbuffer, bits ),
			.word[1] = 0
		};
#else
		uint128_t value;
		value.word[0] = bitbuffer_read64( bitbuffer, bits );
		value.word[1] = 0;
#endif
		return value;
	}
	{
		uint128_t value;
		if( bits > 128 )
			bits = 128;

		value.word[0] = bitbuffer_read64( bitbuffer, 64U );
		value.word[1] = bitbuffer_read64( bitbuffer, bits - 64 );
		return value;
	}
}


uint64_t bitbuffer_read64( bitbuffer_t* bitbuffer, unsigned int bits )
{
	uint32_t val0, val1;

	if( bits <= 32 )
		return bitbuffer_read32( bitbuffer, bits );

	if( bits > 64 )
		bits = 64;

	val0 = bitbuffer_read32( bitbuffer, 32U );
	val1 = bitbuffer_read32( bitbuffer, bits - 32 );
	return (uint64_t)val0 | ( (uint64_t)val1 << 32ULL );
}


float64_t bitbuffer_read_float64( bitbuffer_t* bitbuffer )
{
#if !FOUNDATION_COMPILER_MSVC
	const float64_cast_t conv = { .ival = bitbuffer_read64( bitbuffer, 64U ) };
#else
	float64_cast_t conv; conv.ival = bitbuffer_read64( bitbuffer, 64U );
#endif
	return conv.fval;
}


float32_t bitbuffer_read_float32( bitbuffer_t* bitbuffer )
{
#if !FOUNDATION_COMPILER_MSVC
	const float32_cast_t conv = { .ival = bitbuffer_read32( bitbuffer, 32U ) };
#else
	float32_cast_t conv; conv.ival = bitbuffer_read32( bitbuffer, 32U );
#endif
	return conv.fval;
}


uint32_t bitbuffer_read32( bitbuffer_t* bitbuffer, unsigned int bits )
{
	unsigned int ret;
	unsigned int curbits;

	if( !bits )
		return 0;

	if( bits > 32 )
		bits = 32;

	if( bitbuffer->offset_read >= 32 )
		_bitbuffer_get( bitbuffer );

	curbits = 32 - bitbuffer->offset_read;
	if( bits < curbits )
		curbits = bits;

	ret = ( curbits == 32 ) ? bitbuffer->pending_read : ( ( bitbuffer->pending_read >> bitbuffer->offset_read ) & ( ( 1U << curbits ) - 1 ) );

	bitbuffer->offset_read += curbits;
	bitbuffer->count_read  += curbits;

	if( curbits == bits )
		return ret;

	FOUNDATION_ASSERT( bits && curbits );
	FOUNDATION_ASSERT( bitbuffer->offset_read == 32 );

	_bitbuffer_get( bitbuffer );

	ret |= ( bitbuffer->pending_read & ( ( 1U << ( bits - curbits ) ) - 1 ) ) << curbits;

	bitbuffer->offset_read  = ( bits - curbits );
	bitbuffer->count_read  += ( bits - curbits );

	return ret;
}


void bitbuffer_write128( bitbuffer_t* bitbuffer, uint128_t value, unsigned int bits )
{
	if( bits <= 64 )
	{
		bitbuffer_write64( bitbuffer, value.word[0], bits );
		return;
	}

	if( bits > 128 )
		bits = 128;

	bitbuffer_write64( bitbuffer, value.word[0], 64U );
	bitbuffer_write64( bitbuffer, value.word[1], bits - 64 );
}


void bitbuffer_write64( bitbuffer_t* bitbuffer, uint64_t value, unsigned int bits )
{
	if( bits <= 32 )
	{
		bitbuffer_write32( bitbuffer, (uint32_t)value, bits );
		return;
	}

	if( bits > 64 )
		bits = 64;

	bitbuffer_write32( bitbuffer, (uint32_t)value, 32U );
	bitbuffer_write32( bitbuffer, (uint32_t)( value >> 32ULL ), bits - 32 );
}


void bitbuffer_write_float64( bitbuffer_t* bitbuffer, float64_t value )
{
#if !FOUNDATION_COMPILER_MSVC
	float64_cast_t conv = { .fval = value };
#else
	float64_cast_t conv; conv.fval = value;
#endif
	bitbuffer_write64( bitbuffer, conv.ival, 64U );
}


void bitbuffer_write_float32( bitbuffer_t* bitbuffer, float32_t value )
{
#if !FOUNDATION_COMPILER_MSVC
	float32_cast_t conv = { .fval = value };
#else
	float32_cast_t conv; conv.fval = value;
#endif
	bitbuffer_write32( bitbuffer, conv.ival, 32U );
}


void bitbuffer_write32( bitbuffer_t* bitbuffer, uint32_t value, unsigned int bits )
{
	unsigned int curbits;

	if( !bits )
		return;

	if( bits > 32 )
		bits = 32;

	curbits = 32 - bitbuffer->offset_write;
	if( bits < curbits )
		curbits = bits;

	bitbuffer->pending_write |= ( ( curbits == 32 ) ? value : ( value & ( ( 1U << curbits ) - 1 ) ) ) << bitbuffer->offset_write;
	bitbuffer->offset_write += curbits;

	bitbuffer->count_write += bits;

	if( bitbuffer->offset_write == 32 )
		_bitbuffer_put( bitbuffer );

	if( curbits == bits )
		return;

	FOUNDATION_ASSERT( bits && curbits );

	bitbuffer->pending_write = ( value >> curbits ) & ( ( 1U << ( bits - curbits ) ) - 1 );
	bitbuffer->offset_write  = bits - curbits;
}


void bitbuffer_align_read( bitbuffer_t* bitbuffer, bool force )
{
	if( !( bitbuffer->offset_read & 31 ) ) //0 or 32
	{
		if( !force )
			return;
		if( bitbuffer->offset_read )
			_bitbuffer_get( bitbuffer );
	}
	bitbuffer->count_read += 32 - bitbuffer->offset_read;
	bitbuffer->offset_read = 32;
}


void bitbuffer_align_write( bitbuffer_t* bitbuffer, bool force )
{
	if( !bitbuffer->offset_write && !force )
		return;
	bitbuffer->count_write += 32 - bitbuffer->offset_write;
	_bitbuffer_put( bitbuffer );
}


void bitbuffer_discard_read( bitbuffer_t* bitbuffer )
{
	if( bitbuffer->offset_read != 32 )
		bitbuffer->offset_read = 0;
}


void bitbuffer_discard_write( bitbuffer_t* bitbuffer )
{
	bitbuffer->offset_write = 0;
	bitbuffer->pending_write = 0;
}
