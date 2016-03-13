/* md5.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


#define F1( x, y, z ) ((z) ^ ((x) & ((y) ^ (z))))
#define F2( x, y, z ) ((y) ^ ((z) & ((x) ^ (y))))
#define F3( x, y, z ) ((x) ^ (y) ^ (z))
#define F4( x, y, z ) ((y) ^ ((x) | ~(z)))

#define MD5_STEP( f, a, b, c, d, t, s ) (a) += f((b), (c), (d)) + (t); (a) = (((a) << (s)) | ((a) >> (32 - (s)))); (a) += (b);


static void md5_encode( unsigned char* dest, const uint32_t* src, uint32_t length )
{
	unsigned int i, j;
	for( i = 0, j = 0; j < length; ++i, j += 4 )
	{
		dest[j]   = (unsigned char)(   src[i]         & 0xff );
		dest[j+1] = (unsigned char)( ( src[i] >> 8  ) & 0xff );
		dest[j+2] = (unsigned char)( ( src[i] >> 16 ) & 0xff );
		dest[j+3] = (unsigned char)( ( src[i] >> 24 ) & 0xff );
	}
}


static void md5_decode( uint32_t* dest, const unsigned char* src, uint32_t length )
{
	unsigned int i, j;
	for( i = 0, j = 0; j < length; ++i, j += 4 )
		dest[i] = (uint32_t)src[j] | ( (uint32_t)src[j+1] << 8 ) | ( (uint32_t)src[j+2] << 16 ) | ( (uint32_t)src[j+3] << 24 );
}


static void md5_transform( md5_t* digest, const unsigned char* buffer )
{
	unsigned int a = digest->state[0], b = digest->state[1], c = digest->state[2], d = digest->state[3];
	unsigned int x[16];

	md5_decode( x, buffer, 64 );

	MD5_STEP( F1, a, b, c, d, x[ 0] + 0xd76aa478, 7  );
	MD5_STEP( F1, d, a, b, c, x[ 1] + 0xe8c7b756, 12 );
	MD5_STEP( F1, c, d, a, b, x[ 2] + 0x242070db, 17 );
	MD5_STEP( F1, b, c, d, a, x[ 3] + 0xc1bdceee, 22 );
	MD5_STEP( F1, a, b, c, d, x[ 4] + 0xf57c0faf, 7  );
	MD5_STEP( F1, d, a, b, c, x[ 5] + 0x4787c62a, 12 );
	MD5_STEP( F1, c, d, a, b, x[ 6] + 0xa8304613, 17 );
	MD5_STEP( F1, b, c, d, a, x[ 7] + 0xfd469501, 22 );
	MD5_STEP( F1, a, b, c, d, x[ 8] + 0x698098d8, 7  );
	MD5_STEP( F1, d, a, b, c, x[ 9] + 0x8b44f7af, 12 );
	MD5_STEP( F1, c, d, a, b, x[10] + 0xffff5bb1, 17 );
	MD5_STEP( F1, b, c, d, a, x[11] + 0x895cd7be, 22 );
	MD5_STEP( F1, a, b, c, d, x[12] + 0x6b901122, 7  );
	MD5_STEP( F1, d, a, b, c, x[13] + 0xfd987193, 12 );
	MD5_STEP( F1, c, d, a, b, x[14] + 0xa679438e, 17 );
	MD5_STEP( F1, b, c, d, a, x[15] + 0x49b40821, 22 );

	MD5_STEP( F2, a, b, c, d, x[ 1] + 0xf61e2562, 5  );
	MD5_STEP( F2, d, a, b, c, x[ 6] + 0xc040b340, 9  );
	MD5_STEP( F2, c, d, a, b, x[11] + 0x265e5a51, 14 );
	MD5_STEP( F2, b, c, d, a, x[ 0] + 0xe9b6c7aa, 20 );
	MD5_STEP( F2, a, b, c, d, x[ 5] + 0xd62f105d, 5  );
	MD5_STEP( F2, d, a, b, c, x[10] + 0x02441453, 9  );
	MD5_STEP( F2, c, d, a, b, x[15] + 0xd8a1e681, 14 );
	MD5_STEP( F2, b, c, d, a, x[ 4] + 0xe7d3fbc8, 20 );
	MD5_STEP( F2, a, b, c, d, x[ 9] + 0x21e1cde6, 5  );
	MD5_STEP( F2, d, a, b, c, x[14] + 0xc33707d6, 9  );
	MD5_STEP( F2, c, d, a, b, x[ 3] + 0xf4d50d87, 14 );
	MD5_STEP( F2, b, c, d, a, x[ 8] + 0x455a14ed, 20 );
	MD5_STEP( F2, a, b, c, d, x[13] + 0xa9e3e905, 5  );
	MD5_STEP( F2, d, a, b, c, x[ 2] + 0xfcefa3f8, 9  );
	MD5_STEP( F2, c, d, a, b, x[ 7] + 0x676f02d9, 14 );
	MD5_STEP( F2, b, c, d, a, x[12] + 0x8d2a4c8a, 20 );

	MD5_STEP( F3, a, b, c, d, x[ 5] + 0xfffa3942, 4  );
	MD5_STEP( F3, d, a, b, c, x[ 8] + 0x8771f681, 11 );
	MD5_STEP( F3, c, d, a, b, x[11] + 0x6d9d6122, 16 );
	MD5_STEP( F3, b, c, d, a, x[14] + 0xfde5380c, 23 );
	MD5_STEP( F3, a, b, c, d, x[ 1] + 0xa4beea44, 4  );
	MD5_STEP( F3, d, a, b, c, x[ 4] + 0x4bdecfa9, 11 );
	MD5_STEP( F3, c, d, a, b, x[ 7] + 0xf6bb4b60, 16 );
	MD5_STEP( F3, b, c, d, a, x[10] + 0xbebfbc70, 23 );
	MD5_STEP( F3, a, b, c, d, x[13] + 0x289b7ec6, 4  );
	MD5_STEP( F3, d, a, b, c, x[ 0] + 0xeaa127fa, 11 );
	MD5_STEP( F3, c, d, a, b, x[ 3] + 0xd4ef3085, 16 );
	MD5_STEP( F3, b, c, d, a, x[ 6] + 0x04881d05, 23 );
	MD5_STEP( F3, a, b, c, d, x[ 9] + 0xd9d4d039, 4  );
	MD5_STEP( F3, d, a, b, c, x[12] + 0xe6db99e5, 11 );
	MD5_STEP( F3, c, d, a, b, x[15] + 0x1fa27cf8, 16 );
	MD5_STEP( F3, b, c, d, a, x[ 2] + 0xc4ac5665, 23 );

	MD5_STEP( F4, a, b, c, d, x[ 0] + 0xf4292244, 6  );
	MD5_STEP( F4, d, a, b, c, x[ 7] + 0x432aff97, 10 );
	MD5_STEP( F4, c, d, a, b, x[14] + 0xab9423a7, 15 );
	MD5_STEP( F4, b, c, d, a, x[ 5] + 0xfc93a039, 21 );
	MD5_STEP( F4, a, b, c, d, x[12] + 0x655b59c3, 6  );
	MD5_STEP( F4, d, a, b, c, x[ 3] + 0x8f0ccc92, 10 );
	MD5_STEP( F4, c, d, a, b, x[10] + 0xffeff47d, 15 );
	MD5_STEP( F4, b, c, d, a, x[ 1] + 0x85845dd1, 21 );
	MD5_STEP( F4, a, b, c, d, x[ 8] + 0x6fa87e4f, 6  );
	MD5_STEP( F4, d, a, b, c, x[15] + 0xfe2ce6e0, 10 );
	MD5_STEP( F4, c, d, a, b, x[ 6] + 0xa3014314, 15 );
	MD5_STEP( F4, b, c, d, a, x[13] + 0x4e0811a1, 21 );
	MD5_STEP( F4, a, b, c, d, x[ 4] + 0xf7537e82, 6  );
	MD5_STEP( F4, d, a, b, c, x[11] + 0xbd3af235, 10 );
	MD5_STEP( F4, c, d, a, b, x[ 2] + 0x2ad7d2bb, 15 );
	MD5_STEP( F4, b, c, d, a, x[ 9] + 0xeb86d391, 21 );

	digest->state[0] += a;
	digest->state[1] += b;
	digest->state[2] += c;
	digest->state[3] += d;
}


md5_t* md5_allocate( void )
{
	md5_t* digest = memory_allocate( 0, sizeof( md5_t ), 0, MEMORY_PERSISTENT );

	md5_initialize( digest );

	return digest;
}


void md5_deallocate( md5_t* digest )
{
	md5_finalize( digest );
	memory_deallocate( digest );
}


void md5_initialize( md5_t* digest )
{
	digest->init = false;
	digest->state[0] = 0x67452301;
	digest->state[1] = 0xefcdab89;
	digest->state[2] = 0x98badcfe;
	digest->state[3] = 0x10325476;
	digest->count[0] = 0;
	digest->count[1] = 0;

	memset( digest->buffer, 0, 64 );
}


void md5_finalize( md5_t* md5 )
{
	FOUNDATION_UNUSED( md5 );
}


md5_t* md5_digest( md5_t* digest, const char* msg )
{
	return md5_digest_raw( digest, msg, string_length( msg ) );
}


md5_t* md5_digest_raw( md5_t* digest, const void* buffer, size_t size )
{
	uint64_t index_in, index_buf;
	uint64_t space_buf;

	if( !digest )
		digest = md5_allocate();
	if( digest->init )
		md5_initialize( digest );

	index_buf = ( ( digest->count[0] >> 3 ) & 0x3F );

	digest->count[0] += ( (uint32_t)size << 3 );

	if( digest->count[0] < ( (uint32_t)size << 3 ) )
		++digest->count[1];

	digest->count[1] += ( (uint32_t)size >> 29 );

	space_buf = 64 - index_buf;

	if( size >= space_buf )
	{
		memcpy( digest->buffer + index_buf, buffer, (size_t)space_buf );
		md5_transform( digest, digest->buffer );

		for( index_in = space_buf; index_in + 63 < size; index_in += 64 )
			md5_transform( digest, (const unsigned char*)buffer + index_in );

		index_buf = 0;
	}
	else
		index_in = 0;

	memcpy( digest->buffer + index_buf, (const char*)buffer + index_in, (size_t)( size - index_in ) );

	return digest;
}


void md5_digest_finalize( md5_t* digest )
{
	unsigned char bits[8];
	unsigned int idx, size_pad;
	static const unsigned char padding[64] = {
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	if( !digest )
		return;

	md5_encode( bits, digest->count, 8 );

	idx      = ( ( digest->count[0] >> 3 ) & 0x3f );
	size_pad = ( idx < 56 ) ? ( 56 - idx ) : ( 120 - idx );

	md5_digest_raw( digest, padding, size_pad );
	md5_digest_raw( digest, bits, 8 );

	md5_encode( digest->digest, digest->state, 16 );

	memset( digest->buffer, 0, 64 );

	digest->init = true;
}


uint128_t md5_get_digest_raw( const md5_t* digest )
{
	uint128_t val = {{0,0}};
	if( digest )
		memcpy( &val, digest->digest, sizeof( uint128_t ) );
	return val;
}


char* md5_get_digest( const md5_t* digest )
{
	const char trn[17] = "0123456789ABCDEF";
	int i, j;
	char* str;

	if( !digest )
		return 0;

	str = string_allocate( 32 );
	for( i = 0, j = 0; i < 16; ++i, j += 2 )
	{
		str[j]   = trn[ digest->digest[i] / 16 ];
		str[j+1] = trn[ digest->digest[i] % 16 ];
	}
	str[32] = 0;

	return str;
}

