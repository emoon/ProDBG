/* random.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#include <foundation/internal.h>


// C implementation of the "Maximally equidistributed pseudorandom number generators via linear output transformations" from http://www.sciencedirect.com/science/article/pii/S0378475408002358
// Put state array in thread-local storage for thread safety

#define RANDOM_STATE_SIZE   1391

#define RANDOM_BITS         32
#define RANDOM_BITS_SAVE    15
#define RANDOM_MASK_UPPER   ( 0xFFFFFFFFU >> ( RANDOM_BITS - RANDOM_BITS_SAVE ) )
#define RANDOM_MASK_LOWER   ( ~RANDOM_MASK_UPPER )

#define RANDOM_BITMASK      0x48000000

#define RANDOM_LOW_LIMIT    23
#define RANDOM_MID_LIMIT    229
#define RANDOM_HIGH_LIMIT   481

//Some helper macros to make code a bit more condensed
#define RANDOM_XOR_AND_LEFTSHIFT( bits, val )  ( (val) ^ ( (val) << (bits) ) )
#define RANDOM_XOR_AND_RIGHTSHIFT( bits, val ) ( (val) ^ ( (val) >> (bits) ) )
#define RANDOM_TRANSFORM( bits, key, mask, test, val ) \
	( ( (val) & (test) ) ? ( ( ( ( (val) << (bits) ) ^ ( (val) >> ( RANDOM_BITS - (bits) ) ) ) & (mask) ) ^ (key) ) : \
	                         ( ( ( (val) << (bits) ) ^ ( (val) >> ( RANDOM_BITS - (bits) ) ) ) & (mask) ) )


FOUNDATION_DECLARE_THREAD_LOCAL( unsigned int*, state, 0 )

static mutex_t*       _random_mutex;
static unsigned int** _random_state;
static unsigned int** _random_available_state;


static void _random_seed_buffer( unsigned int* buffer )
{
	int i;
	uint64_t base = time_system();
	for( i = 0; i < RANDOM_STATE_SIZE; ++i )
		buffer[i] ^= ( base + time_current() + ( i * RANDOM_HIGH_LIMIT * RANDOM_LOW_LIMIT ) ) & 0xFFFFFFFFULL;
}


static unsigned int* _random_allocate_buffer( void )
{
	unsigned int* buffer = memory_allocate( 0, sizeof( unsigned int ) * ( RANDOM_STATE_SIZE + 1 ), 0, MEMORY_PERSISTENT );
	_random_seed_buffer( buffer );
	buffer[RANDOM_STATE_SIZE] = 0;
	array_push( _random_state, buffer );
	return buffer;
}


int _random_initialize( void )
{
	if( !_random_mutex )
	{
		int i;
		_random_mutex = mutex_allocate( "random" );

		//Allocate and seed a number of state buffers
		array_reserve( _random_state, 64 );
		array_reserve( _random_available_state, 64 );
		for( i = 0; i < 16; ++i )
		{
			unsigned int* buffer = _random_allocate_buffer();
			array_push( _random_available_state, buffer );
		}
	}
	return 0;
}


void _random_shutdown( void )
{
	int i, size;

	if( _random_mutex )
		mutex_lock( _random_mutex );

	for( i = 0, size = array_size( _random_state ); i < size; ++i )
		memory_deallocate( _random_state[i] );
	array_deallocate( _random_available_state );
	array_deallocate( _random_state );

	set_thread_state( 0 );

	if( _random_mutex )
	{
		mutex_unlock( _random_mutex );
		mutex_deallocate( _random_mutex );
	}
	_random_mutex = 0;
}


static unsigned int* _random_thread_allocate( void )
{
	unsigned int* buffer;

	mutex_lock( _random_mutex );

	//Grab a free state buffer or allocate if none available
	if( !array_size( _random_available_state ) )
	{
		buffer = _random_allocate_buffer();
		array_push( _random_available_state, buffer );
	}
	else
	{
		buffer = _random_available_state[ array_size( _random_available_state ) - 1 ];
		array_pop( _random_available_state );
	}

	mutex_unlock( _random_mutex );

	set_thread_state( buffer );

	return buffer;
}


void random_thread_deallocate( void )
{
	if( !get_thread_state() )
		return;

	mutex_lock( _random_mutex );
	array_push( _random_available_state, get_thread_state() );
	mutex_unlock( _random_mutex );

	set_thread_state( 0 );
}


static FOUNDATION_FORCEINLINE unsigned int random_from_state( unsigned int* FOUNDATION_RESTRICT state )
{
	unsigned int state_index = state[ RANDOM_STATE_SIZE ];
	unsigned int bits0, bits1, bits2;
	if( state_index == 0 )
	{
		bits0 = ( state[ state_index + RANDOM_STATE_SIZE - 1 ] & RANDOM_MASK_LOWER ) | ( state[ state_index + RANDOM_STATE_SIZE - 2 ] & RANDOM_MASK_UPPER );
		bits1 = RANDOM_XOR_AND_LEFTSHIFT( 24, state[ state_index ] ) ^ RANDOM_XOR_AND_RIGHTSHIFT( 30, state[ state_index + RANDOM_LOW_LIMIT ] );
		bits2 = RANDOM_XOR_AND_LEFTSHIFT( 10, state[ state_index + RANDOM_HIGH_LIMIT ] ) ^ ( 26 << state[ state_index + RANDOM_MID_LIMIT ] );
		state[ state_index ] = bits1 ^ bits2;
		state[ state_index - 1 + RANDOM_STATE_SIZE ] = bits0 ^ RANDOM_XOR_AND_RIGHTSHIFT( 20, bits1 ) ^ RANDOM_TRANSFORM( 9, 0xb729fcecU, 0xfbffffffU, 0x00020000U, bits2 ) ^ state[ state_index ];
		state[ RANDOM_STATE_SIZE ] = state_index = RANDOM_STATE_SIZE - 1;
		return ( state[ state_index ] ^ ( state[ state_index + RANDOM_HIGH_LIMIT - RANDOM_STATE_SIZE + 1 ] & RANDOM_BITMASK ) );
	}
	else if( state_index == 1 )
	{
		bits0 = ( state[ state_index - 1 ] & RANDOM_MASK_LOWER ) | ( state[ state_index + RANDOM_STATE_SIZE - 2 ] & RANDOM_MASK_UPPER );
		bits1 = RANDOM_XOR_AND_LEFTSHIFT( 24, state[ state_index ] ) ^ RANDOM_XOR_AND_RIGHTSHIFT( 30, state[ state_index + RANDOM_LOW_LIMIT ] );
		bits2 = RANDOM_XOR_AND_LEFTSHIFT( 10, state[ state_index + RANDOM_HIGH_LIMIT ] ) ^ ( 26 << state[ state_index + RANDOM_MID_LIMIT ] );
		state[ state_index ] = bits1 ^ bits2;
		state[ state_index - 1 ] = bits0 ^ RANDOM_XOR_AND_RIGHTSHIFT( 20, bits1 ) ^ RANDOM_TRANSFORM( 9, 0xb729fcecU, 0xfbffffffU, 0x00020000U, bits2 ) ^ state[ state_index ];
		state[ RANDOM_STATE_SIZE ] = state_index = 0;
		return ( state[ state_index ] ^ ( state[ state_index + RANDOM_HIGH_LIMIT + 1 ] & RANDOM_BITMASK ) );
	}
	else if( state_index + RANDOM_LOW_LIMIT >= RANDOM_STATE_SIZE )
	{
		bits0 = ( state[ state_index - 1 ] & RANDOM_MASK_LOWER ) | ( state[ state_index - 2 ] & RANDOM_MASK_UPPER );
		bits1 = RANDOM_XOR_AND_LEFTSHIFT( 24, state[ state_index ] ) ^ RANDOM_XOR_AND_RIGHTSHIFT( 30, state[ state_index + RANDOM_LOW_LIMIT - RANDOM_STATE_SIZE ] );
		bits2 = RANDOM_XOR_AND_LEFTSHIFT( 10, state[ state_index + RANDOM_HIGH_LIMIT - RANDOM_STATE_SIZE ] ) ^ ( 26 << state[ state_index + RANDOM_MID_LIMIT - RANDOM_STATE_SIZE ] );
		state[ state_index ] = bits1 ^ bits2;
		state[ state_index - 1 ] = bits0 ^ RANDOM_XOR_AND_RIGHTSHIFT( 20, bits1 ) ^ RANDOM_TRANSFORM( 9, 0xb729fcecU, 0xfbffffffU, 0x00020000U, bits2 ) ^ state[ state_index ];
		state[ RANDOM_STATE_SIZE ] = --state_index;
		return ( state[ state_index ] ^ ( state[ state_index + RANDOM_HIGH_LIMIT - RANDOM_STATE_SIZE + 1 ] & RANDOM_BITMASK ) );
	}
	else if( state_index + RANDOM_MID_LIMIT >= RANDOM_STATE_SIZE )
	{
		bits0 = ( state[ state_index - 1 ] & RANDOM_MASK_LOWER ) | ( state[ state_index - 2 ] & RANDOM_MASK_UPPER );
		bits1 = RANDOM_XOR_AND_LEFTSHIFT( 24, state[ state_index ] ) ^ RANDOM_XOR_AND_RIGHTSHIFT( 30, state[ state_index + RANDOM_LOW_LIMIT ] );
		bits2 = RANDOM_XOR_AND_LEFTSHIFT( 10, state[ state_index + RANDOM_HIGH_LIMIT - RANDOM_STATE_SIZE ] ) ^ ( 26 << state[ state_index + RANDOM_MID_LIMIT - RANDOM_STATE_SIZE ] );
		state[ state_index ] = bits1 ^ bits2;
		state[ state_index - 1 ] = bits0 ^ RANDOM_XOR_AND_RIGHTSHIFT( 20, bits1 ) ^ RANDOM_TRANSFORM( 9, 0xb729fcecU, 0xfbffffffU, 0x00020000U, bits2 ) ^ state[ state_index ];
		state[ RANDOM_STATE_SIZE ] = --state_index;
		return ( state[ state_index ] ^ ( state[ state_index + RANDOM_HIGH_LIMIT - RANDOM_STATE_SIZE + 1 ] & RANDOM_BITMASK ) );
	}
	else if( state_index + RANDOM_HIGH_LIMIT >= RANDOM_STATE_SIZE )
	{
		bits0 = ( state[ state_index - 1 ] & RANDOM_MASK_LOWER ) | ( state[ state_index - 2 ] & RANDOM_MASK_UPPER );
		bits1 = RANDOM_XOR_AND_LEFTSHIFT( 24, state[ state_index ] ) ^ RANDOM_XOR_AND_RIGHTSHIFT( 30, state[ state_index + RANDOM_LOW_LIMIT ] );
		bits2 = RANDOM_XOR_AND_LEFTSHIFT( 10, state[ state_index + RANDOM_HIGH_LIMIT - RANDOM_STATE_SIZE ] ) ^ ( 26 << state[ state_index + RANDOM_MID_LIMIT ] );
		state[ state_index ] = bits1 ^ bits2;
		state[ state_index - 1 ] = bits0 ^ RANDOM_XOR_AND_RIGHTSHIFT( 20, bits1 ) ^ RANDOM_TRANSFORM( 9, 0xb729fcecU, 0xfbffffffU, 0x00020000U, bits2 ) ^ state[ state_index ];
		state[ RANDOM_STATE_SIZE ] = --state_index;
		return ( state[ state_index ] ^ ( state[ state_index + RANDOM_HIGH_LIMIT - RANDOM_STATE_SIZE + 1 ] & RANDOM_BITMASK ) );
	}
	//else if( 2 <= state_index <= RANDOM_STATE_SIZE - RANDOM_HIGH_LIMIT - 1 )
	{
		bits0 = ( state[ state_index - 1 ] & RANDOM_MASK_LOWER ) | ( state[ state_index - 2 ] & RANDOM_MASK_UPPER );
		bits1 = RANDOM_XOR_AND_LEFTSHIFT( 24, state[ state_index ] ) ^ RANDOM_XOR_AND_RIGHTSHIFT( 30, state[ state_index + RANDOM_LOW_LIMIT ] );
		bits2 = RANDOM_XOR_AND_LEFTSHIFT( 10, state[ state_index + RANDOM_HIGH_LIMIT ] ) ^ ( 26 << state[ state_index + RANDOM_MID_LIMIT ] );
		state[ state_index ] = bits1 ^ bits2;
		state[ state_index - 1 ] = bits0 ^ RANDOM_XOR_AND_RIGHTSHIFT( 20, bits1 ) ^ RANDOM_TRANSFORM( 9, 0xb729fcecU, 0xfbffffffU, 0x00020000U, bits2 ) ^ state[ state_index ];
		state[ RANDOM_STATE_SIZE ] = --state_index;
		return ( state[ state_index ] ^ ( state[ state_index + RANDOM_HIGH_LIMIT + 1 ] & RANDOM_BITMASK ) );
	}
}


uint32_t random32( void )
{
	unsigned int* state = get_thread_state();
	if( !state )
		state = _random_thread_allocate();

	return random_from_state( state );
}


uint32_t random32_range( uint32_t low, uint32_t high )
{
	if( low > high )
	{
		uint32_t tmp = low;
		low = high;
		high = tmp;
	}
	if( high <= low + 1 )
		return low;
	return low + ( random32() % ( high - low ) );
}


uint64_t random64( void )
{
	uint32_t low, high;
	unsigned int* state = get_thread_state();
	if( !state )
		state = _random_thread_allocate();

	low = random_from_state( state );
	high = random_from_state( state );

	return ( (uint64_t)high << 32ULL ) | low;
}


uint64_t random64_range( uint64_t low, uint64_t high )
{
	if( low > high )
	{
		uint64_t tmp = low;
		low = high;
		high = tmp;
	}
	if( high <= low + 1 )
		return low;
	return low + ( random64() % ( high - low ) );
}


real random_normalized( void )
{
#if FOUNDATION_SIZE_REAL == 64
	const real result = (real)random64() * ( REAL_C( 1.0 ) / REAL_C( 18446744073709551616.0L ) );
#else
	const real result = (real)random32() * ( REAL_C( 1.0 ) / REAL_C( 4294967296.0 ) );
#endif
	//Deal with floating point roundoff issues
	if( result >= REAL_C( 1.0 ) )
		return math_realdec( REAL_C( 1.0 ), 1 );
	else if( result < 0 )
		return 0;
	return result;
}


real random_range( real low, real high )
{
	real result;
	if( low > high )
	{
		real tmp = low;
		low = high;
		high = tmp;
	}
	result = low + ( ( high - low ) * random_normalized() );
	//Deal with floating point roundoff issues
	if( result <= low )
		return low;
	else if( result >= high )
		return math_realdec( high, 1 );
	return result;
}


int32_t random32_gaussian_range( int32_t low, int32_t high )
{
	const uint64_t cubic = ( ( ( (uint64_t)random32() + (uint64_t)random32() ) + ( (uint64_t)random32() + (uint64_t)random32() ) + 2ULL ) >> 2ULL );
	if( low > high )
	{
		int32_t tmp = low;
		low = high;
		high = tmp;
	}
	return low + (int32_t)( ( cubic * (uint64_t)( high - low ) ) >> 32ULL );
}


real random_gaussian_range( real low, real high )
{
	real result;
	if( low > high )
	{
		real tmp = low;
		low = high;
		high = tmp;
	}
	result = low + ( ( high - low ) * REAL_C( 0.33333333333333333333333333333 ) * ( random_normalized() + random_normalized() + random_normalized() ) );
	if( result <= low )
		return low;
	else if( result >= high )
		return math_realdec( high, 1 );
	return result;
}


int32_t random32_triangle_range( int32_t low, int32_t high )
{
	const uint32_t t0 = random32();
	const uint32_t t1  = random32();
	const uint64_t tri = ( t0 >> 1 ) + ( t1 >> 1 ) + ( t0 & t1 & 1 );
	if( low > high )
	{
		int32_t tmp = low;
		low = high;
		high = tmp;
	}
	return low + (int32_t)( ( tri * (uint64_t)( high - low ) ) >> 32ULL );
}


real random_triangle_range( real low, real high )
{
	real result;
	if( low > high )
	{
		real tmp = low;
		low = high;
		high = tmp;
	}
	result = low + ( high - low ) * REAL_C( 0.5 ) * ( random_normalized() + random_normalized() );
	if( result <= low )
		return low;
	else if( result >= high )
		return math_realdec( high, 1 );
	return result;
}


uint32_t random32_weighted( uint32_t limit, const real* weights )
{
	uint32_t i;
	real sum;
	if( limit >= 2 )
	{
		sum = 0;
		for( i = 0; i < limit; ++i )
			sum += ( weights[i] > 0 ? weights[i] : 0 );

		if( sum > 0 )
		{
			real value = random_range( 0, sum );
			for( i = 0; i < limit; ++i )
			{
				if( weights[i] > 0 )
				{
					if( value < weights[i] )
						return i;
					value -= weights[i];
				}
			}
		}
		else
		{
			return random32_range( 0, limit );
		}
	}

	//Deal with floating point roundoff issues
	return limit - 1;
}

