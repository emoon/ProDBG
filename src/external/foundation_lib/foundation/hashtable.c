/* hashtable.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


static FOUNDATION_FORCEINLINE uint32_t _hashtable32_hash( uint32_t key )
{
	key ^= key >> 16;
	key *= 0x85ebca6b;
	key ^= key >> 13;
	key *= 0xc2b2ae35;
	key ^= key >> 16;
	return key;
}


static FOUNDATION_FORCEINLINE uint64_t _hashtable64_hash( uint64_t key )
{
	key ^= key >> 33;
	key *= 0xff51afd7ed558ccd;
	key ^= key >> 33;
	key *= 0xc4ceb9fe1a85ec53;
	key ^= key >> 33;
	return key;
}


hashtable32_t* hashtable32_allocate( unsigned int buckets )
{
	hashtable32_t* table = (hashtable32_t*)memory_allocate( 0, sizeof( hashtable32_t ) + sizeof( hashtable32_entry_t ) * buckets, 8, MEMORY_PERSISTENT );

	hashtable32_initialize( table, buckets );

	return table;
}


void hashtable32_initialize( hashtable32_t* table, unsigned int buckets )
{
	memset( table, 0, sizeof( hashtable32_t ) + sizeof( hashtable32_entry_t ) * buckets );
	table->capacity = buckets;
}


void hashtable32_deallocate( hashtable32_t* table )
{
	hashtable32_finalize( table );
	memory_deallocate( table );
}


void hashtable32_finalize( hashtable32_t* table )
{
	FOUNDATION_UNUSED( table );
}


void hashtable32_set( hashtable32_t* table, uint32_t key, uint32_t value )
{
	uint32_t ie, eend;

	FOUNDATION_ASSERT( table );
	FOUNDATION_ASSERT( key );
	FOUNDATION_ASSERT( value );

	ie = eend = _hashtable32_hash( key ) % table->capacity;
	do
	{
		uint32_t current_key = atomic_load32( &table->entries[ie].key );

		if( current_key != key )
		{
			if( current_key || !atomic_cas32( &table->entries[ie].key, key, 0 ) )
			{
				ie = ( ie + 1 ) % table->capacity;
				if( ie == eend )
				{
					FOUNDATION_ASSERT( "Hashtable set looped, out-out-memory" );
					//Keep looping until slot frees up
					thread_yield();
				}
				continue;
			}
		}

		table->entries[ie].value = value;
		break;
	} while( true );
}


void hashtable32_erase( hashtable32_t* table, uint32_t key )
{
	uint32_t ie, eend;

	FOUNDATION_ASSERT( table );
	FOUNDATION_ASSERT( key );

	ie = eend = _hashtable32_hash( key ) % table->capacity;
	do
	{
		uint32_t current_key = atomic_load32( &table->entries[ie].key );

		if( current_key == key )
		{
			table->entries[ie].value = 0;
			return;
		}

		if( !current_key )
			return;

		ie = ( ie + 1 ) % table->capacity;
		if( ie == eend )
			return;
	} while( true );
}


uint32_t hashtable32_get( hashtable32_t* table, uint32_t key )
{
	uint32_t ie, eend;

	FOUNDATION_ASSERT( table );
	FOUNDATION_ASSERT( key );

	ie = eend = _hashtable32_hash( key ) % table->capacity;
	do
	{
		uint32_t current_key = atomic_load32( &table->entries[ie].key );

		if( current_key == key )
			return table->entries[ie].value;

		if( !current_key )
			return 0;

		ie = ( ie + 1 ) % table->capacity;
		if( ie == eend )
			return 0;
	} while( true );

	return 0;
}


uint32_t hashtable32_raw( hashtable32_t* table, uint32_t slot )
{
	if( !atomic_load32( &table->entries[slot].key ) )
		return 0;
	return table->entries[slot].value;
}


unsigned int hashtable32_size( hashtable32_t* table )
{
	unsigned int count = 0;
	unsigned int ie;
	for( ie = 0; ie < table->capacity; ++ie )
	{
		if( atomic_load32( &table->entries[ie].key ) && table->entries[ie].value )
			++count;
	}
	return count;
}


void hashtable32_clear( hashtable32_t* table )
{
	FOUNDATION_ASSERT( table );
	memset( table->entries, 0, sizeof( hashtable32_entry_t ) * table->capacity );
}



hashtable64_t* hashtable64_allocate( unsigned int buckets )
{
	hashtable64_t* table = (hashtable64_t*)memory_allocate( 0, sizeof( hashtable64_t ) + sizeof( hashtable64_entry_t ) * buckets, 8, MEMORY_PERSISTENT );

	hashtable64_initialize( table, buckets );

	return table;
}


void hashtable64_initialize( hashtable64_t* table, unsigned int buckets )
{
	memset( table, 0, sizeof( hashtable64_t ) + sizeof( hashtable64_entry_t ) * buckets );
	table->capacity = buckets;
}


void hashtable64_deallocate( hashtable64_t* table )
{
	hashtable64_finalize( table );
	memory_deallocate( table );
}


void hashtable64_finalize( hashtable64_t* table )
{
	FOUNDATION_UNUSED( table );
}


void hashtable64_set( hashtable64_t* table, uint64_t key, uint64_t value )
{
	uint64_t ie, eend;

	FOUNDATION_ASSERT( table );
	FOUNDATION_ASSERT( key );
	FOUNDATION_ASSERT( value );

	ie = eend = _hashtable64_hash( key ) % table->capacity;
	do
	{
		uint64_t current_key = atomic_load64( &table->entries[ie].key );

		if( current_key != key )
		{
			if( current_key || !atomic_cas64( &table->entries[ie].key, key, 0 ) )
			{
				ie = ( ie + 1 ) % table->capacity;
				if( ie == eend )
				{
					FOUNDATION_ASSERT( "Hashtable set looped, out-out-memory" );
					//Keep looping until slot frees up
					thread_yield();
				}
				continue;
			}
		}

		table->entries[ie].value = value;
		break;
	} while( true );
}


void hashtable64_erase( hashtable64_t* table, uint64_t key )
{
	uint64_t ie, eend;

	FOUNDATION_ASSERT( table );
	FOUNDATION_ASSERT( key );

	ie = eend = _hashtable64_hash( key ) % table->capacity;
	do
	{
		uint64_t current_key = atomic_load64( &table->entries[ie].key );

		if( current_key == key )
		{
			table->entries[ie].value = 0;
			return;
		}

		if( !current_key )
			return;

		ie = ( ie + 1 ) % table->capacity;
		if( ie == eend )
			return;
	} while( true );
}


uint64_t hashtable64_get( hashtable64_t* table, uint64_t key )
{
	uint64_t ie, eend;

	FOUNDATION_ASSERT( table );
	FOUNDATION_ASSERT( key );

	ie = eend = _hashtable64_hash( key ) % table->capacity;
	do
	{
		uint64_t current_key = atomic_load64( &table->entries[ie].key );

		if( current_key == key )
			return table->entries[ie].value;

		if( !current_key )
			return 0;

		ie = ( ie + 1 ) % table->capacity;
		if( ie == eend )
			return 0;
	} while( true );

	return 0;
}


uint64_t hashtable64_raw( hashtable64_t* table, uint64_t slot )
{
	if( !atomic_load64( &table->entries[slot].key ) )
		return 0;
	return table->entries[slot].value;
}


unsigned int hashtable64_size( hashtable64_t* table )
{
	unsigned int count = 0;
	unsigned int ie;
	for( ie = 0; ie < table->capacity; ++ie )
	{
		if( atomic_load64( &table->entries[ie].key ) && table->entries[ie].value )
			++count;
	}
	return count;
}


void hashtable64_clear( hashtable64_t* table )
{
	FOUNDATION_ASSERT( table );
	memset( table->entries, 0, sizeof( hashtable64_entry_t ) * (size_t)table->capacity );
}
