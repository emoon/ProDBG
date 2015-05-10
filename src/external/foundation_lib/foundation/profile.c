/* profile.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#if BUILD_ENABLE_PROFILE

#define PROFILE_ENABLE_SANITY_CHECKS 0

typedef struct _profile_block_data   profile_block_data_t;
typedef struct _profile_block        profile_block_t;
typedef struct _profile_root         profile_root_t;

#define MAX_MESSAGE_LENGTH 25

#pragma pack(push)
#pragma pack(1)

struct _profile_block_data
{
	uint32_t              id;
	uint32_t              parentid;
	uint32_t              processor;
	uint32_t              thread;
	uint64_t              start;
	uint64_t              end;
	char                  name[ MAX_MESSAGE_LENGTH + 1 ];
}; //sizeof( profile_block_data ) == 58
FOUNDATION_STATIC_ASSERT( sizeof( profile_block_data_t ) == 58, "profile_block_data_t size" );

#pragma pack(pop)

struct _profile_block
{
	profile_block_data_t  data;
	uint16_t              previous;
	uint16_t              sibling;
	uint16_t              child;
}; //sizeof( profile_block ) == 64
FOUNDATION_STATIC_ASSERT( sizeof( profile_block_t ) == 64, "profile_block_t size" );

#define PROFILE_ID_ENDOFSTREAM      0
#define PROFILE_ID_SYSTEMINFO       1
#define PROFILE_ID_LOGMESSAGE       2
#define PROFILE_ID_LOGCONTINUE      3
#define PROFILE_ID_ENDFRAME         4
#define PROFILE_ID_TRYLOCK          5
#define PROFILE_ID_TRYLOCKCONTINUE  6
#define PROFILE_ID_LOCK             7
#define PROFILE_ID_LOCKCONTINUE     8
#define PROFILE_ID_UNLOCK           9
#define PROFILE_ID_UNLOCKCONTINUE   10
#define PROFILE_ID_WAIT             11
#define PROFILE_ID_SIGNAL           12

#define GET_BLOCK( index )          ( _profile_blocks + (index) )
#define BLOCK_INDEX( block )        (uint16_t)((uintptr_t)( (block) - _profile_blocks ))

static const char*                  _profile_identifier;
static atomic32_t                   _profile_counter;
static atomic32_t                   _profile_loopid;
static atomic32_t                   _profile_free;
static atomic32_t                   _profile_root;
static profile_block_t*             _profile_blocks;
static uint64_t                     _profile_ground_time;
static int                          _profile_enable;
static profile_write_fn             _profile_write;
static uint64_t                     _profile_num_blocks;
static int                          _profile_wait = 100;
static object_t                     _profile_io_thread;

FOUNDATION_DECLARE_THREAD_LOCAL( uint32_t, profile_block, 0 )


static profile_block_t* _profile_allocate_block( void )
{
	//Grab block from free list, avoiding ABA issues by
	//using high 16 bit as a loop counter
	profile_block_t* block;
	uint32_t free_block_tag, free_block, next_block_tag;
	do
	{
		free_block_tag = atomic_load32( &_profile_free );
		free_block = free_block_tag & 0xffff;

		next_block_tag = GET_BLOCK( free_block )->child;
		next_block_tag |= ( atomic_incr32( &_profile_loopid ) & 0xffff ) << 16;
	} while( free_block && !atomic_cas32( &_profile_free, next_block_tag, free_block_tag ) );

	if( !free_block )
	{
		static atomic32_t has_warned = {0};
		if( atomic_cas32( &has_warned, 1, 0 ) )
			log_error( 0, ERROR_OUT_OF_MEMORY, ( _profile_num_blocks < 65535 ) ? "Profile blocks exhausted, increase profile memory block size" : "Profile blocks exhausted, decrease profile output wait time" );
		return 0;
	}

	block = GET_BLOCK( free_block );
	memset( block, 0, sizeof( profile_block_t ) );
	return block;
}


static void _profile_free_block( uint32_t block, uint32_t leaf )
{
	uint32_t last_tag, block_tag;
	do
	{
		block_tag = block | ( ( atomic_incr32( &_profile_loopid ) & 0xffff ) << 16 );
		last_tag = atomic_load32( &_profile_free );
		GET_BLOCK( leaf )->child = last_tag & 0xffff;
	} while( !atomic_cas32( &_profile_free, block_tag, last_tag ) );
}


static void _profile_put_root_block( uint32_t block )
{
	uint32_t sibling;
	profile_block_t* self = GET_BLOCK( block );

#if PROFILE_ENABLE_SANITY_CHECKS
	FOUNDATION_ASSERT( self->sibling == 0 );
#endif
	while( !atomic_cas32( &_profile_root, block, 0 ) )
	{
		do
		{
			sibling = atomic_load32( &_profile_root );
		} while( sibling && !atomic_cas32( &_profile_root, 0, sibling ) );

		if( sibling )
		{
			if( self->sibling )
			{
				uint32_t leaf = self->sibling;
				while( GET_BLOCK( leaf )->sibling )
					leaf = GET_BLOCK( leaf )->sibling;
				GET_BLOCK( sibling )->previous = leaf;
				GET_BLOCK( leaf )->sibling = sibling;
			}
			else
			{
				self->sibling = sibling;
			}
		}
	}
}


static void _profile_put_simple_block( uint32_t block )
{
	//Add to current block, or if no current add to array
	uint32_t parent_block = get_thread_profile_block();
	if( parent_block )
	{
		profile_block_t* self = GET_BLOCK( block );
		profile_block_t* parent = GET_BLOCK( parent_block );
		uint32_t next_block = parent->child;
		self->previous = (uint16_t)parent_block;
		self->sibling = (uint16_t)next_block;
		if( next_block )
			GET_BLOCK( next_block )->previous = (uint16_t)block;
		parent->child = block;
	}
	else
	{
		_profile_put_root_block( block );
	}
}


static void _profile_put_message_block( uint32_t id, const char* message )
{
	profile_block_t* subblock = 0;
	int len = (int)string_length( message );

	//Allocate new master block
	profile_block_t* block = _profile_allocate_block();
	if( !block )
		return;
	block->data.id = id;
	block->data.processor = thread_hardware();
	block->data.thread = (uint32_t)thread_id();
	block->data.start  = time_current() - _profile_ground_time;
	block->data.end = atomic_add32( &_profile_counter, 1 );
	memcpy( block->data.name, message, ( len >= MAX_MESSAGE_LENGTH ) ? MAX_MESSAGE_LENGTH : len );

	len -= MAX_MESSAGE_LENGTH;
	message += MAX_MESSAGE_LENGTH;
	subblock = block;

	while( len > 0 )
	{
		//add subblock
		profile_block_t* cblock = _profile_allocate_block();
		uint16_t cblock_index;
		if( !cblock )
			return;
		cblock_index = BLOCK_INDEX( cblock );
		cblock->data.id = id + 1;
		cblock->data.parentid = (uint32_t)subblock->data.end;
		cblock->data.processor = block->data.processor;
		cblock->data.thread = block->data.thread;
		cblock->data.start  = block->data.start;
		cblock->data.end    = atomic_add32( &_profile_counter, 1 );
		memcpy( cblock->data.name, message, ( len >= MAX_MESSAGE_LENGTH ) ? MAX_MESSAGE_LENGTH : len );

		cblock->sibling = subblock->child;
		if( cblock->sibling )
			GET_BLOCK( cblock->sibling )->previous = cblock_index;
		subblock->child = cblock_index;
		cblock->previous = BLOCK_INDEX( subblock );
		subblock = cblock;

		len -= MAX_MESSAGE_LENGTH;
		message += MAX_MESSAGE_LENGTH;
	}

	_profile_put_simple_block( BLOCK_INDEX( block ) );
}


//Pass each block once, writing it to stream and adjusting child/sibling pointers to form a single-linked list through child pointer
//Potential drawback of this is that block access order will degenerate over time and result in random access over the whole
//profile memory area in the end
static profile_block_t* _profile_process_block( profile_block_t* block )
{
	profile_block_t* leaf = block;

	if( _profile_write )
		_profile_write( block, sizeof( profile_block_t ) );

	if( block->child )
	{
		leaf = _profile_process_block( GET_BLOCK( block->child ) );
		if( block->sibling )
		{
			profile_block_t* subleaf = _profile_process_block( GET_BLOCK( block->sibling ) );
			subleaf->child = block->child;
			block->child = block->sibling;
			block->sibling = 0;
		}
	}
	else if( block->sibling )
	{
		leaf = _profile_process_block( GET_BLOCK( block->sibling ) );
		block->child = block->sibling;
		block->sibling = 0;
	}
	return leaf;
}


static void _profile_process_root_block( void )
{
	uint32_t block;

	do
	{
		block = atomic_load32( &_profile_root );
	} while( block && !atomic_cas32( &_profile_root, 0, block ) );

	while( block )
	{
		profile_block_t* leaf;
		profile_block_t* current = GET_BLOCK( block );
		uint32_t next = current->sibling;

		current->sibling = 0;
		leaf = _profile_process_block( current );
		_profile_free_block( block, BLOCK_INDEX( leaf ) );

		block = next;
	}
}


static void* _profile_io( object_t thread, void* arg )
{
	unsigned int system_info_counter = 0;
	profile_block_t system_info;
	FOUNDATION_UNUSED( arg );
	memset( &system_info, 0, sizeof( profile_block_t ) );
	system_info.data.id = PROFILE_ID_SYSTEMINFO;
	system_info.data.start = time_ticks_per_second();
	string_copy( system_info.data.name, "sysinfo", 7 );

	while( !thread_should_terminate( thread ) )
	{
		thread_sleep( _profile_wait );

		if( !atomic_load32( &_profile_root ) )
			continue;

		profile_begin_block( "profile_io" );

		if( atomic_load32( &_profile_root ) )
		{
			profile_begin_block( "process" );

			//This is thread safe in the sense that only completely closed and ended
			//blocks will be put as children to root block, so no additional blocks
			//will ever be added to child subtrees while we process it here
			_profile_process_root_block();

			profile_end_block();
		}

		if( system_info_counter++ > 10 )
		{
			if( _profile_write )
				_profile_write( &system_info, sizeof( profile_block_t ) );
			system_info_counter = 0;
		}

		profile_end_block();
	}

	if( atomic_load32( &_profile_root ) )
		_profile_process_root_block();

	if( _profile_write )
	{
		profile_block_t terminate;
		memset( &terminate, 0, sizeof( profile_block_t ) );
		terminate.data.id = PROFILE_ID_ENDOFSTREAM;
		_profile_write( &terminate, sizeof( profile_block_t ) );
	}

	return 0;
}


void profile_initialize( const char* identifier, void* buffer, uint64_t size )
{
	profile_block_t* root  = buffer;
	profile_block_t* block = root;
	uint64_t num_blocks = size / sizeof( profile_block_t );
	uint32_t i;

	if( num_blocks > 65535 )
		num_blocks = 65535;

	for( i = 0; i < ( num_blocks - 1 ); ++i, ++block )
	{
		block->child = ( i + 1 );
		block->sibling = 0;
	}
	block->child = 0;
	block->sibling = 0;
	root->child = 0;

	atomic_store32( &_profile_root, 0 );

	_profile_num_blocks = num_blocks;
	_profile_identifier = identifier;
	_profile_blocks = root;
	atomic_store32( &_profile_free, 1 ); //TODO: Currently 0 is a no-block identifier, so we waste the first block
	atomic_store32( &_profile_counter, 128 );
	_profile_ground_time = time_current();
	set_thread_profile_block( 0 );

	log_debugf( 0, "Initialize profiling system with %llu blocks (%lluKiB)", num_blocks, size / 1024 );
}


void profile_shutdown( void )
{
	profile_enable( 0 );

	while( thread_is_thread( _profile_io_thread ) )
		thread_sleep( 1 );
	_profile_io_thread = 0;

	//Discard and free up blocks remaining in queue
	_profile_thread_finalize();
	if( atomic_load32( &_profile_root ) )
		_profile_process_root_block();

	//Sanity checks
	{
		uint64_t num_blocks = 0;
		uint32_t free_block = atomic_load32( &_profile_free ) & 0xffff;

		if( atomic_load32( &_profile_root ) )
			log_error( 0, ERROR_INTERNAL_FAILURE, "Profile module state inconsistent on shutdown, at least one root block still allocated/active" );

		while( free_block )
		{
			profile_block_t* block = GET_BLOCK( free_block );
			if( block->sibling )
			       log_errorf( 0, ERROR_INTERNAL_FAILURE, "Profile module state inconsistent on shutdown, block %d has sibling set", free_block );
			++num_blocks;
			free_block = GET_BLOCK( free_block )->child;
		}
		if( _profile_num_blocks )
			++num_blocks; //Include the wasted block 0

		if( num_blocks != _profile_num_blocks )
		{
			//If profile output function (user) crashed, this will probably trigger since at least one block will be lost in space
			log_errorf( 0, ERROR_INTERNAL_FAILURE, "Profile module state inconsistent on shutdown, lost blocks (found %llu of %llu)", num_blocks, _profile_num_blocks );
		}
	}

	atomic_store32( &_profile_root, 0 );
	atomic_store32( &_profile_free, 0 );

	_profile_num_blocks = 0;
	_profile_identifier = 0;
}


void profile_set_output( profile_write_fn writer )
{
	_profile_write = writer;
}


void profile_set_output_wait( int ms )
{
	_profile_wait = ( ms > 1 ) ? ms : 1;
}


void profile_enable( bool enable )
{
	bool was_enabled = ( _profile_enable > 0 );
	bool is_enabled = enable;

	if( is_enabled && !was_enabled )
	{
		_profile_enable = 1;

		//Start output thread
		_profile_io_thread = thread_create( _profile_io, "profile_io", THREAD_PRIORITY_BELOWNORMAL, 0 );
		thread_start( _profile_io_thread, 0 );

		while( !thread_is_running( _profile_io_thread ) )
			thread_yield();
	}
	else if( !is_enabled && was_enabled )
	{
		//Stop output thread
		thread_terminate( _profile_io_thread );
		thread_destroy( _profile_io_thread );

		while( thread_is_running( _profile_io_thread ) )
			thread_yield();

		_profile_enable = 0;
	}
}


void profile_end_frame( uint64_t counter )
{
	profile_block_t* block;
	if( !_profile_enable )
		return;

	//Allocate new master block
	block = _profile_allocate_block();
	if( !block )
		return;
	block->data.id = PROFILE_ID_ENDFRAME;
	block->data.processor = thread_hardware();
	block->data.thread = (uint32_t)thread_id();
	block->data.start  = time_current() - _profile_ground_time;
	block->data.end = counter;

	_profile_put_simple_block( BLOCK_INDEX( block ) );
}


void profile_begin_block( const char* message )
{
	uint32_t parent;
	if( !_profile_enable )
		return;

	parent = get_thread_profile_block();
	if( !parent )
	{
		//Allocate new master block
		profile_block_t* block = _profile_allocate_block();
		uint32_t blockindex;
		if( !block )
			return;
		blockindex = BLOCK_INDEX( block );
		block->data.id = atomic_add32( &_profile_counter, 1 );
		string_copy( block->data.name, message, MAX_MESSAGE_LENGTH );
		block->data.processor = thread_hardware();
		block->data.thread = (uint32_t)thread_id();
		block->data.start  = time_current() - _profile_ground_time;
		set_thread_profile_block( blockindex );
	}
	else
	{
		//Allocate new child block
		profile_block_t* parentblock;
		profile_block_t* subblock = _profile_allocate_block();
		uint32_t subindex;
		if( !subblock )
			return;
		subindex = BLOCK_INDEX( subblock );
		parentblock = GET_BLOCK( parent );
		subblock->data.id = atomic_add32( &_profile_counter, 1 );
		subblock->data.parentid = parentblock->data.id;
		string_copy( subblock->data.name, message, MAX_MESSAGE_LENGTH );
		subblock->data.processor = thread_hardware();
		subblock->data.thread = (uint32_t)thread_id();
		subblock->data.start  = time_current() - _profile_ground_time;
		subblock->previous = parent;
		subblock->sibling = parentblock->child;
		if( parentblock->child )
			GET_BLOCK( parentblock->child )->previous = subindex;
		parentblock->child = subindex;
		set_thread_profile_block( subindex );
	}
}


void profile_update_block( void )
{
	char* message;
	unsigned int processor;
	uint32_t block_index = get_thread_profile_block();
	profile_block_t* block;
	if( !_profile_enable || !block_index )
		return;

	block = GET_BLOCK( block_index );
	message = block->data.name;
	processor = thread_hardware();
	if( block->data.processor == processor )
		return;

	//Thread migrated to another core, split into new block
	profile_end_block();
	profile_begin_block( message );
}


void profile_end_block( void )
{
	uint32_t block_index = get_thread_profile_block();
	profile_block_t* block;
	if( !_profile_enable || !block_index )
		return;

	block = GET_BLOCK( block_index );
	block->data.end = time_current() - _profile_ground_time;

	if( block->previous )
	{
		unsigned int processor;
		profile_block_t* current = block;
		profile_block_t* previous = GET_BLOCK( block->previous );
		profile_block_t* parent;
		unsigned int current_index = block_index;
		unsigned int parent_index;
		while( previous->child != current_index )
		{
			current_index = current->previous; //Walk sibling list backwards
			current = GET_BLOCK( current_index );
			previous = GET_BLOCK( current->previous );
#if PROFILE_ENABLE_SANITY_CHECKS
			FOUNDATION_ASSERT( current_index != 0 );
			FOUNDATION_ASSERT( current->previous != 0 );
#endif
		}
		parent_index = current->previous; //Previous now points to parent
		parent = GET_BLOCK( parent_index );
#if PROFILE_ENABLE_SANITY_CHECKS
		FOUNDATION_ASSERT( parent_index != block_index );
#endif
		set_thread_profile_block( parent_index );

		processor = thread_hardware();
		if( parent->data.processor != processor )
		{
			const char* message = parent->data.name;
			//Thread migrated, split into new block
			profile_end_block();
			profile_begin_block( message );
		}
	}
	else
	{
		_profile_put_root_block( block_index );
		set_thread_profile_block( 0 );
	}
}


void profile_log( const char* message )
{
	if( !_profile_enable )
		return;

	_profile_put_message_block( PROFILE_ID_LOGMESSAGE, message );
}


void profile_trylock( const char* name )
{
	if( !_profile_enable )
		return;

	_profile_put_message_block( PROFILE_ID_TRYLOCK, name );
}


void profile_lock( const char* name )
{
	if( !_profile_enable )
		return;

	_profile_put_message_block( PROFILE_ID_LOCK, name );
}


void profile_unlock( const char* name )
{
	if( !_profile_enable )
		return;

	_profile_put_message_block( PROFILE_ID_UNLOCK, name );
}


void profile_wait( const char* name )
{
	if( !_profile_enable )
		return;

	_profile_put_message_block( PROFILE_ID_WAIT, name );
}


void profile_signal( const char* name )
{
	if( !_profile_enable )
		return;

	_profile_put_message_block( PROFILE_ID_SIGNAL, name );
}


#endif


void _profile_thread_finalize( void )
{
#if BUILD_ENABLE_PROFILE
	uint32_t block_index, last_block = 0;
	while( ( block_index = get_thread_profile_block() ) )
	{
		log_warnf( 0, WARNING_SUSPICIOUS, "Profile thread cleanup, free block %u", block_index );
		if( last_block == block_index )
		{
			log_warnf( 0, WARNING_SUSPICIOUS, "Unrecoverable error, self reference in block %u", block_index );
			break;
		}
		profile_end_block();
	}
#endif
}
