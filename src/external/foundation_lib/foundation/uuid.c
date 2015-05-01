/* uuid.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


typedef struct
{
	uint32_t       data1;
	uint16_t       data2;
	uint16_t       data3;
	uint8_t        data4[8];
} uuid_raw_t;

typedef struct
{
	uint32_t       time_low;
	uint16_t       time_mid;
	uint16_t       time_hi_and_version;
	uint8_t        clock_seq_hi_and_reserved;
	uint8_t        clock_seq_low;
	uint8_t        node[6];
} uuid_time_t;

typedef union
{
	uuid_t         uuid;
	uuid_raw_t     raw;
	uint128_t      rnd;
} uuid_random_t;

typedef union
{
	uuid_raw_t     raw;
	uuid_time_t    time;
	uuid_random_t  random;
	uuid_t         uuid;
} uuid_convert_t;


static atomic32_t _uuid_last_counter;


//682EAE88-339A-41B6-B8E3-997DAA0466D4
const uuid_t UUID_DNS = { { 0x682EAE88339A41B6ULL, 0xB8E3997DAA0466D4ULL } };


uuid_t uuid_generate_random( void )
{
	uuid_random_t random_uuid;
	random_uuid.rnd = uint128_make( random64(), random64() );

	//Add variant and version
	random_uuid.raw.data3 &= 0x0FFF;
	random_uuid.raw.data3 |= 0x4000;
	random_uuid.raw.data4[0] &= 0x3F;
	random_uuid.raw.data4[0] |= 0x80;

	return random_uuid.uuid;
}


uuid_t uuid_generate_time( void )
{
	uuid_time_t time_uuid;
	uuid_convert_t convert;
	int64_t current_time;
	int32_t current_counter;
	tick_t current_tick;
	int in = 0;
	uint32_t clock_seq = 0;
	uint64_t host_id = 0;

	//Allows creation of 10000 unique timestamps per millisecond
	current_time = time_system();
	current_counter = atomic_incr32( &_uuid_last_counter ) % 10000;

	current_tick = ( (tick_t)current_time * 10000ULL ) + current_counter + 0x01B21DD213814000ULL; //Convert to 100ns since UUID UTC base time, October 15 1582, and add counter

	//We have no state so clock sequence is random
	clock_seq = random32();

	time_uuid.time_low = (uint32_t)( current_tick & 0xFFFFFFFFULL );
	time_uuid.time_mid = (uint16_t)( ( current_tick >> 32ULL ) & 0xFFFFULL );
	time_uuid.time_hi_and_version = (uint16_t)( current_tick >> 48ULL );
	time_uuid.clock_seq_low = ( clock_seq & 0xFF );
	time_uuid.clock_seq_hi_and_reserved = ( ( clock_seq & 0x3F00 ) >> 8 );

	//If hardware node ID is null, use random and set identifier (multicast) bit
	host_id = system_hostid();
	if( host_id )
	{
		for( in = 0; in < 6; ++in )
			time_uuid.node[5-in] = (uint8_t)( ( host_id >> ( 8ULL * in ) ) & 0xFF );
	}
	else
	{
		for( in = 0; in < 6; ++in )
			time_uuid.node[in] = (uint8_t)( random32() & 0xFF );
		time_uuid.node[0] |= 0x01;
	}

	//Add variant and version
	time_uuid.time_hi_and_version &= 0x0FFF;
	time_uuid.time_hi_and_version |= ( 1 << 12 );
	time_uuid.clock_seq_hi_and_reserved &= 0x3F;
	time_uuid.clock_seq_hi_and_reserved |= 0x80;

	convert.time = time_uuid;
	return convert.uuid;
}


uuid_t uuid_generate_name( const uuid_t ns, const char* name )
{
	//v3 uuid, namespace and md5
	md5_t md5;
	uuid_raw_t namespace_id;
	uuid_raw_t gen_uuid;
	uuid_convert_t convert;
	uint128_t digest;

	//Namespace in network byte order
	convert.uuid = ns;
	namespace_id = convert.raw;
	namespace_id.data1 = byteorder_bigendian32( namespace_id.data1 );
	namespace_id.data2 = byteorder_bigendian16( namespace_id.data2 );
	namespace_id.data3 = byteorder_bigendian16( namespace_id.data3 );

	md5_initialize( &md5 );
	md5_digest_raw( &md5, &namespace_id, sizeof( namespace_id ) );
	md5_digest( &md5, name );
	md5_digest_finalize( &md5 );

	//Convert to host order
	digest = md5_get_digest_raw( &md5 );
	memcpy( &gen_uuid, &digest, sizeof( uuid_raw_t ) );
	gen_uuid.data1 = byteorder_bigendian32( gen_uuid.data1 );
	gen_uuid.data2 = byteorder_bigendian16( gen_uuid.data2 );
	gen_uuid.data3 = byteorder_bigendian16( gen_uuid.data3 );

	//Add variant and version
	gen_uuid.data3 &= 0x0FFF;
	gen_uuid.data3 |= ( 3 << 12 ); //Variant 3 for MD5
	gen_uuid.data4[0] &= 0x3F;
	gen_uuid.data4[0] |= 0x80;

	md5_finalize( &md5 );

	convert.raw = gen_uuid;
	return convert.uuid;
}


#include <stdio.h>

char* string_from_uuid_buffer( char* buffer, const uuid_t val )
{
	uuid_convert_t convert;
	convert.uuid = val;
	/*unsigned int len = (unsigned int)*/sprintf( buffer, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", convert.raw.data1, convert.raw.data2, convert.raw.data3, convert.raw.data4[0], convert.raw.data4[1], convert.raw.data4[2], convert.raw.data4[3], convert.raw.data4[4], convert.raw.data4[5], convert.raw.data4[6], convert.raw.data4[7] );
	return buffer;
}


uuid_t string_to_uuid( const char* str )
{
	uuid_convert_t convert;
	unsigned int data[10];
	memset( data, 0, sizeof( data ) );
	convert.raw.data1 = 0;
	sscanf( str, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", &convert.raw.data1, &data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7], &data[8], &data[9] );
	convert.raw.data2 = data[0];
	convert.raw.data3 = data[1];
	convert.raw.data4[0] = data[2];
	convert.raw.data4[1] = data[3];
	convert.raw.data4[2] = data[4];
	convert.raw.data4[3] = data[5];
	convert.raw.data4[4] = data[6];
	convert.raw.data4[5] = data[7];
	convert.raw.data4[6] = data[8];
	convert.raw.data4[7] = data[9];
	return convert.uuid;
}
