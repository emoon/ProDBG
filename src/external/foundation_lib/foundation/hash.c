/* hash.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


#if FOUNDATION_COMPILER_MSVC
#  include <stdlib.h>
#elif FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
#  define _rotl64(a, bits) (((a) << (uint64_t)(bits)) | ((a) >> (64ULL - (uint64_t)(bits))))
#endif

#define HASH_SEED 0xbaadf00d


static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL uint64_t fmix64( uint64_t k );


//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

static FOUNDATION_FORCEINLINE uint64_t getblock( const uint64_t* FOUNDATION_RESTRICT p, const unsigned int i )
{
#if FOUNDATION_ARCH_ENDIAN_LITTLE
	return p[i];
#else
	return byteorder_swap64( p[i] );
#endif
}

#if FOUNDATION_ARCH_ARM || FOUNDATION_ARCH_ARM_64
static FOUNDATION_FORCEINLINE uint64_t getblock_nonaligned( const char* FOUNDATION_RESTRICT p, const unsigned int i )
{
	uint64_t ret;
	memcpy( &ret, p + i*8, 8 );
#if FOUNDATION_ARCH_ENDIAN_LITTLE
	return ret;
#else
	return byteorder_swap64( ret );
#endif
}
#endif


//----------
// Block mix - combine the key bits with the hash bits and scramble everything
#define bmix64( h1, h2, k1, k2, c1, c2 ) \
	k1 *= c1; \
	k1  = _rotl64(k1,23); \
	k1 *= c2; \
	h1 ^= k1; \
	h1 += h2; \
	h2 = _rotl64(h2,41); \
	k2 *= c2; \
	k2  = _rotl64(k2,23); \
	k2 *= c1; \
	h2 ^= k2; \
	h2 += h1; \
	h1 = h1*3+0x52dce729; \
	h2 = h2*3+0x38495ab5; \
	c1 = c1*5+0x7b7d159c; \
	c2 = c2*5+0x6bce6396;

//----------
// Finalization mix - avalanches all bits to within 0.05% bias
static FOUNDATION_FORCEINLINE uint64_t fmix64( uint64_t k )
{
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccd;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53;
	k ^= k >> 33;

	return k;
}


hash_t hash( const void* key, const unsigned int len )
{
	const unsigned int nblocks = len / 16;
	const uint64_t* blocks;
	unsigned int i;
	const uint8_t* tail;
	uint64_t k1;
	uint64_t k2;

	uint64_t h1 = 0x9368e53c2f6af274ULL ^ HASH_SEED;
	uint64_t h2 = 0x586dcd208f7cd3fdULL ^ HASH_SEED;

    /*lint -esym(438,c1,c2) Last value of c1 and c2 not used */
	uint64_t c1 = 0x87c37b91114253d5ULL;
	uint64_t c2 = 0x4cf5ad432745937fULL;

	//----------
	// body

	blocks = (const uint64_t*)key; /*lint !e826 Ok, loop below will not access data outside scope*/

#if FOUNDATION_ARCH_ARM || FOUNDATION_ARCH_ARM_64
	if( (uintptr_t)key % 8 )
	for( i = 0; i < nblocks; ++i )
	{
		k1 = getblock_nonaligned(key,i*2);
		k2 = getblock_nonaligned(key,i*2+1);

		bmix64(h1,h2,k1,k2,c1,c2);
	}
	else
#endif
	for( i = 0; i < nblocks; ++i )
	{
		k1 = getblock(blocks,i*2);
		k2 = getblock(blocks,i*2+1);

		bmix64(h1,h2,k1,k2,c1,c2);
	}

	//----------
	// tail

	tail = pointer_offset_const( key, nblocks * 16 );

	k1 = 0;
	k2 = 0;

	switch(len & 15) /*lint -save -e616 -e825 -e744 */
	{
	case 15: k2 ^= ((uint64_t)tail[14]) << 48;
	case 14: k2 ^= ((uint64_t)tail[13]) << 40;
	case 13: k2 ^= ((uint64_t)tail[12]) << 32;
	case 12: k2 ^= ((uint64_t)tail[11]) << 24;
	case 11: k2 ^= ((uint64_t)tail[10]) << 16;
	case 10: k2 ^= ((uint64_t)tail[ 9]) << 8;
	case  9: k2 ^= ((uint64_t)tail[ 8]);

	case  8: k1 ^= ((uint64_t)tail[ 7]) << 56;
	case  7: k1 ^= ((uint64_t)tail[ 6]) << 48;
	case  6: k1 ^= ((uint64_t)tail[ 5]) << 40;
	case  5: k1 ^= ((uint64_t)tail[ 4]) << 32;
	case  4: k1 ^= ((uint64_t)tail[ 3]) << 24;
	case  3: k1 ^= ((uint64_t)tail[ 2]) << 16;
	case  2: k1 ^= ((uint64_t)tail[ 1]) << 8;
	case  1: k1 ^= ((uint64_t)tail[ 0]);
	         bmix64(h1,h2,k1,k2,c1,c2);
	}; /*lint -restore */

	//----------
	// finalization

	h2 ^= len;

	h1 += h2;
	h2 += h1;

	h1 = fmix64(h1);
	h2 = fmix64(h2);

	h1 += h2;
	/* We only need 64-bit part
	h2 += h1;

	((uint64_t*)out)[0] = h1;
	((uint64_t*)out)[1] = h2;*/
	return h1;
}


#if BUILD_ENABLE_STATIC_HASH_DEBUG


static hashtable64_t* _hash_lookup;


int _static_hash_initialize( void )
{
	if( !_hash_lookup )
		_hash_lookup = hashtable64_allocate( BUILD_SIZE_STATIC_HASH_STORE + 1 );
	return 0;
}


void _static_hash_shutdown( void )
{
	unsigned int slot;
	for( slot = 0; slot < BUILD_SIZE_STATIC_HASH_STORE + 1; ++slot )
	{
		char* str = (char*)((uintptr_t)hashtable64_raw( _hash_lookup, slot ));
		if( str )
			string_deallocate( str );
	}

	hashtable64_deallocate( _hash_lookup );
	_hash_lookup = 0;
}


void _static_hash_store( const void* key, const unsigned int len, const hash_t value )
{
	char* stored;

	if( !_hash_lookup )
		return;

	stored = (char*)((uintptr_t)hashtable64_get( _hash_lookup, value ));
	if( stored )
	{
		FOUNDATION_ASSERT_MSG( string_equal_substr( stored, key, len ), "Static hash collision" );
		FOUNDATION_ASSERT_MSG( string_length( stored ) == len, "Static hash collision" );
		return;
	}

	stored = memory_allocate( 0, len + 1, 0, MEMORY_PERSISTENT );
	memcpy( stored, key, len );
	stored[len] = 0;

	hashtable64_set( _hash_lookup, value, (uint64_t)(uintptr_t)stored );
}


const char* hash_to_string( const hash_t value )
{
	if( !_hash_lookup )
		return 0;

	return (const char*)(uintptr_t)hashtable64_get( _hash_lookup, value );
}


#else


int _static_hash_initialize( void )
{
	return 0;
}


void _static_hash_shutdown( void )
{
}


#endif
