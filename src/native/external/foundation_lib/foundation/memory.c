/* memory.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#if FOUNDATION_PLATFORM_WINDOWS
#  include <foundation/windows.h>
#endif
#if FOUNDATION_PLATFORM_POSIX
#  include <foundation/posix.h>
#  include <sys/mman.h>
#endif

#if FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_ANDROID
#  include <malloc.h>
#endif

#if FOUNDATION_PLATFORM_PNACL
#  include <stdlib.h>
#endif

#define MEMORY_GUARD_VALUE 0xDEADBEEF


static memory_system_t _memory_system;
static memory_tracker_t _memory_no_tracker;

typedef FOUNDATION_ALIGN(8) struct
{
	void*               storage;
	void*               end;
	atomicptr_t         head;
	void*               tail;
	uint64_t            size;
	uint64_t            maxchunk;
} atomic_linear_memory_t;

static atomic_linear_memory_t _memory_temporary;


#if BUILD_ENABLE_MEMORY_TRACKER
static memory_tracker_t _memory_tracker;
static void _memory_track( void* addr, uint64_t size );
static void _memory_untrack( void* addr );
#else
#define _memory_track( addr, size ) do { (void)sizeof( (addr) ); (void)sizeof( (size) ); } while(0)
#define _memory_untrack( addr ) do { (void)sizeof( (addr) ); } while(0)
#endif


#if FOUNDATION_PLATFORM_ANDROID
#  define FOUNDATION_MAX_ALIGN  8
#else
#  define FOUNDATION_MAX_ALIGN  16
#endif


static void _atomic_allocate_initialize( uint64_t storagesize )
{
	if( storagesize < 1024 )
		storagesize = BUILD_SIZE_TEMPORARY_MEMORY;
	_memory_temporary.storage   = memory_allocate( 0, storagesize, 16, MEMORY_PERSISTENT );
	_memory_temporary.end       = pointer_offset( _memory_temporary.storage, storagesize );
	_memory_temporary.size      = storagesize;
	_memory_temporary.maxchunk  = ( storagesize / 8 );
	atomic_storeptr( &_memory_temporary.head, _memory_temporary.storage );
}


static void _atomic_allocate_shutdown( void )
{
	if( _memory_temporary.storage )
		memory_deallocate( _memory_temporary.storage );
	memset( &_memory_temporary, 0, sizeof( _memory_temporary ) );
}


static void* _atomic_allocate_linear( uint64_t chunksize )
{
	void* old_head;
	void* new_head;
	void* return_pointer = 0;

	do
	{
		old_head = atomic_loadptr( &_memory_temporary.head );
		new_head = pointer_offset( old_head, chunksize );

		return_pointer = old_head;

		if( new_head > _memory_temporary.end )
		{
			new_head = pointer_offset( _memory_temporary.storage, chunksize );
			return_pointer = _memory_temporary.storage;
		}
	} while( !atomic_cas_ptr( &_memory_temporary.head, new_head, old_head ) );

	return return_pointer;
}


static FOUNDATION_CONSTCALL FOUNDATION_FORCEINLINE unsigned int _memory_get_align( unsigned int align )
{
	//All alignment in memory code is built around higher alignments
	//being multiples of lower alignments (powers of two).
	//4, 8, 16, ...
#if FOUNDATION_PLATFORM_ANDROID
	return align ? FOUNDATION_MAX_ALIGN : 0;
#elif FOUNDATION_PLATFORM_WINDOWS
	if( align < FOUNDATION_SIZE_POINTER )
		return FOUNDATION_SIZE_POINTER;
	align = math_align_poweroftwo( align );
	return ( align < FOUNDATION_MAX_ALIGN ) ? align : FOUNDATION_MAX_ALIGN;
#else
	if( align < FOUNDATION_SIZE_POINTER )
		return align ? FOUNDATION_SIZE_POINTER : 0;
	align = math_align_poweroftwo( align );
	return ( align < FOUNDATION_MAX_ALIGN ) ? align : FOUNDATION_MAX_ALIGN;
#endif
}


static FOUNDATION_CONSTCALL void* _memory_align_pointer( void* p, unsigned int align )
{
	uintptr_t address;
	if( !p || !align )
		return p;

	address = (uintptr_t)p;
	if( address % align )
	{
		address = address + ( align - ( address % align ) );
		p = (void*)address;
	}

	return p;
}


int _memory_initialize( const memory_system_t memory )
{
	_memory_system = memory;
	return _memory_system.initialize();
}


void _memory_preallocate( void )
{
	hash_t tracker;

	if( !_memory_temporary.storage )
		_atomic_allocate_initialize( config_int( HASH_FOUNDATION, HASH_TEMPORARY_MEMORY ) );

	tracker = config_string_hash( HASH_FOUNDATION, HASH_MEMORY_TRACKER );
	if( tracker == HASH_LOCAL )
		memory_set_tracker( memory_tracker_local() );
}


void _memory_shutdown( void )
{
	memory_set_tracker( _memory_no_tracker );

	_atomic_allocate_shutdown();
	_memory_system.shutdown();
}


#if BUILD_ENABLE_MEMORY_GUARD

static void* _memory_guard_initialize( void* memory, size_t size )
{
	int guard_loop;
	uint32_t* guard_header = pointer_offset( memory, FOUNDATION_MAX_ALIGN );
	uint32_t* guard_footer = pointer_offset( memory, size + FOUNDATION_MAX_ALIGN * 2 );
	*(uint64_t*)memory = size;
	for( guard_loop = 0; guard_loop < FOUNDATION_MAX_ALIGN / 4; ++guard_loop )
	{
		*guard_header++ = MEMORY_GUARD_VALUE;
		*guard_footer++ = MEMORY_GUARD_VALUE;
	}
	return pointer_offset( memory, FOUNDATION_MAX_ALIGN * 2 );
}


static void* _memory_guard_verify( void* memory )
{
	int guard_loop;
	uint64_t  size = *(uint64_t*)pointer_offset( memory, -FOUNDATION_MAX_ALIGN * 2 );
	uint32_t* guard_header = pointer_offset( memory, -FOUNDATION_MAX_ALIGN );
	uint32_t* guard_footer = pointer_offset( memory, size );
	for( guard_loop = 0; guard_loop < FOUNDATION_MAX_ALIGN / 4; ++guard_loop )
	{
		if( *guard_header != MEMORY_GUARD_VALUE )
			FOUNDATION_ASSERT_MSG( *guard_header == MEMORY_GUARD_VALUE, "Memory underwrite" );
		if( *guard_footer != MEMORY_GUARD_VALUE )
			FOUNDATION_ASSERT_MSG( *guard_footer == MEMORY_GUARD_VALUE, "Memory overwrite" );
		guard_header++;
		guard_footer++;
	}
	return pointer_offset( memory, -FOUNDATION_MAX_ALIGN * 2 );
}

#endif


void* memory_allocate( uint64_t context, uint64_t size, unsigned int align, int hint )
{
	void* p;
	if( ( hint & MEMORY_TEMPORARY ) && _memory_temporary.storage && ( size + align < _memory_temporary.maxchunk ) )
	{
		align = _memory_get_align( align );
		if( align < FOUNDATION_SIZE_POINTER )
			align = FOUNDATION_SIZE_POINTER;
		p = _memory_align_pointer( _atomic_allocate_linear( size + align ), align );
		FOUNDATION_ASSERT( !( (uintptr_t)p & 1 ) );
		memset( p, 0, (size_t)size );
	}
	else
	{
		p = _memory_system.allocate( context ? context : memory_context(), size, align, hint );
	}
	_memory_track( p, size );
	return p;
}


void* memory_reallocate( void* p, uint64_t size, unsigned int align, uint64_t oldsize )
{
	FOUNDATION_ASSERT_MSG( ( p < _memory_temporary.storage ) || ( p >= _memory_temporary.end ), "Trying to reallocate temporary memory" );
	_memory_untrack( p );
	p = _memory_system.reallocate( p, size, align, oldsize );
	_memory_track( p, size );
	return p;
}


void memory_deallocate( void* p )
{
	if( ( p < _memory_temporary.storage ) || ( p >=_memory_temporary.end ) )
		_memory_system.deallocate( p );
	_memory_untrack( p );
}


#if BUILD_ENABLE_MEMORY_CONTEXT


FOUNDATION_DECLARE_THREAD_LOCAL( memory_context_t*, memory_context, 0 )

void memory_context_push( uint64_t context_id )
{
	memory_context_t* context = get_thread_memory_context();
	if( !context )
	{
		context = memory_allocate( 0, sizeof( memory_context_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
		set_thread_memory_context( context );
	}
	FOUNDATION_ASSERT_MSG( context->depth < BUILD_SIZE_MEMORY_CONTEXT_DEPTH, "Memory context stack overflow" );
	context->context[ context->depth ] = context_id;
	if( context->depth < BUILD_SIZE_MEMORY_CONTEXT_DEPTH-1 )
		++context->depth;
}


void memory_context_pop( void )
{
	memory_context_t* context = get_thread_memory_context();
	if( context )
	{
		FOUNDATION_ASSERT_MSG( context->depth, "Memory context stack underflow" );
		--context->depth;
	}
}


uint64_t memory_context( void )
{
	memory_context_t* context = get_thread_memory_context();
	return ( context && ( context->depth > 0 ) ) ? context->context[ context->depth - 1 ] : 0;
}


void memory_context_thread_deallocate( void )
{
	memory_context_t* context = get_thread_memory_context();
	if( context )
		memory_deallocate( context );
	set_thread_memory_context( 0 );
}


#endif


#if FOUNDATION_PLATFORM_WINDOWS

typedef long (*NtAllocateVirtualMemoryFn)( HANDLE, void**, ULONG, size_t*, ULONG, ULONG);
static NtAllocateVirtualMemoryFn NtAllocateVirtualMemory = 0;

#endif


static void* _memory_allocate_malloc_raw( uint64_t size, unsigned int align, int hint )
{
	FOUNDATION_UNUSED( hint );

	//If we align manually, we must be able to retrieve the original pointer for passing to free()
	//Thus all allocations need to go through that path

#if FOUNDATION_PLATFORM_WINDOWS

#  if FOUNDATION_SIZE_POINTER == 4
#    if BUILD_ENABLE_MEMORY_GUARD
	char* memory = _aligned_malloc( (size_t)size + FOUNDATION_MAX_ALIGN * 3, align );
	if( memory )
		memory = _memory_guard_initialize( memory, (size_t)size );
	return memory;
#    else
	return _aligned_malloc( (size_t)size, align );
#    endif
#  else
	unsigned int padding, extra_padding = 0;
	size_t allocate_size;
	char* raw_memory;
	void* memory;
	long vmres;

	if( !( hint & MEMORY_32BIT_ADDRESS ) )
	{
		padding = ( align > FOUNDATION_SIZE_POINTER ? align : FOUNDATION_SIZE_POINTER );
#if BUILD_ENABLE_MEMORY_GUARD
		extra_padding = FOUNDATION_MAX_ALIGN * 3;
#endif
		raw_memory = _aligned_malloc( (size_t)size + padding + extra_padding, align );
		if( raw_memory )
		{
			memory = raw_memory + padding; //Will be aligned since padding is multiple of alignment (minimum align/pad is pointer size)
			*( (void**)memory - 1 ) = raw_memory;
			FOUNDATION_ASSERT( !( (uintptr_t)raw_memory & 1 ) );
			FOUNDATION_ASSERT( !( (uintptr_t)memory & 1 ) );
#if BUILD_ENABLE_MEMORY_GUARD
			memory = _memory_guard_initialize( memory, size );
			FOUNDATION_ASSERT( !( (uintptr_t)memory & 1 ) );
#endif
			return memory;
		}
		log_errorf( HASH_MEMORY, ERROR_OUT_OF_MEMORY, "Unable to allocate %llu bytes of memory", size );
		return 0;
	}

#    if BUILD_ENABLE_MEMORY_GUARD
	extra_padding = FOUNDATION_MAX_ALIGN * 3;
#    endif

	allocate_size = size + FOUNDATION_SIZE_POINTER + extra_padding + align;
	raw_memory = 0;

	vmres = NtAllocateVirtualMemory( INVALID_HANDLE_VALUE, &raw_memory, 1, &allocate_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
	if( vmres != 0 )
	{
		log_errorf( HASH_MEMORY, ERROR_OUT_OF_MEMORY, "Unable to allocate %llu bytes of memory in low 32bit address space", size );
		return 0;
	}

	memory = _memory_align_pointer( raw_memory + FOUNDATION_SIZE_POINTER, align );
	*( (void**)memory - 1 ) = (void*)( (uintptr_t)raw_memory | 1 );
#    if BUILD_ENABLE_MEMORY_GUARD
	memory = _memory_guard_initialize( memory, size );
#    endif
	FOUNDATION_ASSERT( !( (uintptr_t)raw_memory & 1 ) );
	FOUNDATION_ASSERT( !( (uintptr_t)memory & 1 ) );
	return memory;
#  endif

#else

#  if FOUNDATION_SIZE_POINTER > 4
	if( !( hint & MEMORY_32BIT_ADDRESS ) )
#  endif
	{
#if BUILD_ENABLE_MEMORY_GUARD
		unsigned int extra_padding = FOUNDATION_MAX_ALIGN * 3;
#else
		unsigned int extra_padding = 0;
#endif
		size_t allocate_size = (size_t)size + align + FOUNDATION_SIZE_POINTER + extra_padding;
		char* raw_memory = malloc( allocate_size );
		if( raw_memory )
		{
			void* memory = _memory_align_pointer( raw_memory + FOUNDATION_SIZE_POINTER, align );
			*( (void**)memory - 1 ) = raw_memory;
			FOUNDATION_ASSERT( !( (uintptr_t)raw_memory & 1 ) );
			FOUNDATION_ASSERT( !( (uintptr_t)memory & 1 ) );
#if BUILD_ENABLE_MEMORY_GUARD
			memory = _memory_guard_initialize( memory, size );
			FOUNDATION_ASSERT( !( (uintptr_t)memory & 1 ) );
#endif
			return memory;
		}
		log_errorf( HASH_MEMORY, ERROR_OUT_OF_MEMORY, "Unable to allocate %llu bytes of memory (%llu requested)", (uint64_t)size, (uint64_t)allocate_size );
		return 0;
	}

#  if FOUNDATION_SIZE_POINTER > 4

	size_t allocate_size;
	char* raw_memory;
	void* memory;

#    if BUILD_ENABLE_MEMORY_GUARD
	unsigned int extra_padding = FOUNDATION_MAX_ALIGN * 3;
#else
	unsigned int extra_padding = 0;
#    endif

	allocate_size = size + align + FOUNDATION_SIZE_POINTER*2 + extra_padding;

	#ifndef MAP_UNINITIALIZED
	#define MAP_UNINITIALIZED 0
	#endif

	#ifndef MAP_ANONYMOUS
	#define MAP_ANONYMOUS MAP_ANON
	#endif

#    ifndef MAP_32BIT
    //On MacOSX app needs to be linked with -pagezero_size 10000 -image_base 100000000 to
    // 1) Free up low 4Gb address range by reducing page zero size
    // 2) Move executable base address above 4Gb to free up more memory address space
    #define MMAP_REGION_START ((uintptr_t)0x10000)
    #define MMAP_REGION_END   ((uintptr_t)0x80000000)
    static atomicptr_t baseaddr = { (void*)MMAP_REGION_START };
    bool retried = false;
    do
    {
        raw_memory = mmap( atomic_loadptr( &baseaddr ), allocate_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED, -1, 0 );
        if( ( (uintptr_t)raw_memory >= MMAP_REGION_START ) && (uintptr_t)( raw_memory + allocate_size ) < MMAP_REGION_END )
        {
            atomic_storeptr( &baseaddr, pointer_offset( raw_memory, allocate_size ) );
            break;
        }
        if( raw_memory )
            munmap( raw_memory, allocate_size );
        raw_memory = 0;
        if( retried )
            break;
        retried = true;
        atomic_storeptr( &baseaddr, (void*)MMAP_REGION_START );
    } while( true );
#    else
	raw_memory = mmap( 0, allocate_size, PROT_READ | PROT_WRITE, MAP_32BIT | MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED, -1, 0 );
#    endif
	if( !raw_memory )
	{
		log_errorf( HASH_MEMORY, ERROR_OUT_OF_MEMORY, "Unable to allocate %llu bytes of memory in low 32bit address space", (uint64_t)size );
		return 0;
	}

	memory = _memory_align_pointer( raw_memory + FOUNDATION_SIZE_POINTER*2, align );
	*( (uintptr_t*)memory - 1 ) = ( (uintptr_t)raw_memory | 1 );
	*( (uintptr_t*)memory - 2 ) = allocate_size;
	FOUNDATION_ASSERT( !( (uintptr_t)raw_memory & 1 ) );
	FOUNDATION_ASSERT( !( (uintptr_t)memory & 1 ) );
#    if BUILD_ENABLE_MEMORY_GUARD
	memory = _memory_guard_initialize( memory, size );
	FOUNDATION_ASSERT( !( (uintptr_t)memory & 1 ) );
#    endif

	return memory;

#  endif

#endif
}


static void* _memory_allocate_malloc( uint64_t context, uint64_t size, unsigned int align, int hint )
{
	void* block;
	FOUNDATION_UNUSED( context );
	align = _memory_get_align( align );
	block = _memory_allocate_malloc_raw( size, align, hint );
	if( block && ( hint & MEMORY_ZERO_INITIALIZED ) )
		memset( block, 0, (size_t)size );
	return block;
}


static void _memory_deallocate_malloc( void* p )
{
#if FOUNDATION_SIZE_POINTER == 4
	if( !p )
		return;
#  if BUILD_ENABLE_MEMORY_GUARD
	p = _memory_guard_verify( p );
#  endif
#  if FOUNDATION_PLATFORM_WINDOWS
	_aligned_free( p );
#  else
	free( *( (void**)p - 1 ) );
#  endif

#else

	uintptr_t raw_ptr;

	if( !p )
		return;

#  if BUILD_ENABLE_MEMORY_GUARD
	p = _memory_guard_verify( p );
#  endif
	raw_ptr = *( (uintptr_t*)p - 1 );
	if( raw_ptr & 1 )
	{
#  if FOUNDATION_PLATFORM_WINDOWS
		VirtualFree( (void*)raw_ptr, 0, MEM_RELEASE );
#  else
		uintptr_t raw_size = *( (uintptr_t*)p - 2 );
		munmap( (void*)raw_ptr, raw_size );
#  endif
	}
	else
	{
#  if FOUNDATION_PLATFORM_WINDOWS
		_aligned_free( (void*)raw_ptr );
#  else
		free( (void*)raw_ptr );
#  endif
	}

#endif
}


static void* _memory_reallocate_malloc( void* p, uint64_t size, unsigned int align, uint64_t oldsize )
{
#if ( FOUNDATION_SIZE_POINTER == 4 ) && FOUNDATION_PLATFORM_WINDOWS
	align = _memory_get_align( align );
#  if BUILD_ENABLE_MEMORY_GUARD
	if( p )
	{
		p = _memory_guard_verify( p );
		p = _aligned_realloc( p, (size_t)size + FOUNDATION_MAX_ALIGN * 3, align );
	}
	else
	{
		p = _aligned_malloc( (size_t)size + FOUNDATION_MAX_ALIGN * 3, align );
	}
	if( p )
		p = _memory_guard_initialize( p, (size_t)size );
	return p;
#  else
	return _aligned_realloc( p, (size_t)size, align );
#  endif
#else
	void* memory;
	void* raw_p FOUNDATION_ATTRIBUTE(unused);

	align = _memory_get_align( align );

	memory = p;
#  if BUILD_ENABLE_MEMORY_GUARD
	if( memory )
		memory = _memory_guard_verify( memory );
#  endif
	raw_p = memory ? *( (void**)memory - 1 ) : 0;
	memory = 0;

#if FOUNDATION_PLATFORM_WINDOWS
	if( raw_p && !( (uintptr_t)raw_p & 1 ) )
	{
		unsigned int padding = ( align > FOUNDATION_SIZE_POINTER ? align : FOUNDATION_SIZE_POINTER );
#  if BUILD_ENABLE_MEMORY_GUARD
		unsigned int extra_padding = FOUNDATION_MAX_ALIGN * 3;
#  else
		unsigned int extra_padding = 0;
#  endif
		void* raw_memory = _aligned_realloc( raw_p, size + padding + extra_padding, align ? align : 8 );
		if( raw_memory )
		{
			memory = pointer_offset( raw_memory, padding );
			*( (void**)memory - 1 ) = raw_memory;
		}
#  if BUILD_ENABLE_MEMORY_GUARD
		memory = _memory_guard_initialize( memory, size );
#  endif
	}
	else
	{
#  if FOUNDATION_SIZE_POINTER == 4
		memory = _memory_allocate_malloc_raw( size, align, 0U );
#  else
		memory = _memory_allocate_malloc_raw( size, align, ( raw_p && ( (uintptr_t)raw_p < 0xFFFFFFFFULL) ) ? MEMORY_32BIT_ADDRESS : 0U );
#  endif
		if( p && memory && oldsize )
			memcpy( memory, p, ( size < oldsize ) ? size : oldsize );
		_memory_deallocate_malloc( p );
	}

#else //!FOUNDATION_PLATFORM_WINDOWS

//If we're on ARM the realloc can return a 16-bit aligned address, causing raw pointer store to SIGILL
//Realigning does not work since the realloc memory copy preserve cannot be done properly. Revert to normal alloc-and-copy
//Same with alignment, since we cannot guarantee that the returned memory block offset from start of actual memory block
//is the same in the reallocated block as the original block, we need to alloc-and-copy to get alignment
//Memory guard introduces implicit alignments as well so alloc-and-copy for that
#if !FOUNDATION_ARCH_ARM && !FOUNDATION_ARCH_ARM_64 && !BUILD_ENABLE_MEMORY_GUARD
	if( !align && raw_p && !( (uintptr_t)raw_p & 1 ) )
	{
		void* raw_memory = realloc( raw_p, (size_t)size + FOUNDATION_SIZE_POINTER );
		if( raw_memory )
		{
			*(void**)raw_memory = raw_memory;
			memory = pointer_offset( raw_memory, FOUNDATION_SIZE_POINTER );
		}
	}
	else
#endif
	{
#  if FOUNDATION_SIZE_POINTER == 4
		memory = _memory_allocate_malloc_raw( size, align, 0U );
#  else
		memory = _memory_allocate_malloc_raw( size, align, ( raw_p && ( (uintptr_t)raw_p < 0xFFFFFFFFULL) ) ? MEMORY_32BIT_ADDRESS : 0U );
#  endif
		if( p && memory && oldsize )
			memcpy( memory, p, ( size < oldsize ) ? (size_t)size : (size_t)oldsize );
		_memory_deallocate_malloc( p );
	}

#endif

	if( !memory )
	{
		log_panicf( HASH_MEMORY, ERROR_OUT_OF_MEMORY, "Unable to reallocate memory: %s (%" PRIfixPTR ", raw %" PRIfixPTR ")", system_error_message( 0 ), p, raw_p );
	}
	return memory;
#endif
}


static int _memory_initialize_malloc( void )
{
#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_SIZE_POINTER > 4 )
	NtAllocateVirtualMemory = (NtAllocateVirtualMemoryFn)GetProcAddress( GetModuleHandleA( "ntdll.dll" ), "NtAllocateVirtualMemory" );
#endif
	return 0;
}


static void _memory_shutdown_malloc( void )
{
}


memory_system_t memory_system_malloc( void )
{
	memory_system_t memsystem;
	memsystem.allocate = _memory_allocate_malloc;
	memsystem.reallocate = _memory_reallocate_malloc;
	memsystem.deallocate = _memory_deallocate_malloc;
	memsystem.initialize = _memory_initialize_malloc;
	memsystem.shutdown = _memory_shutdown_malloc;
	return memsystem;
}


#if BUILD_ENABLE_MEMORY_TRACKER


void memory_set_tracker( memory_tracker_t tracker )
{
	memory_tracker_t old_tracker = _memory_tracker;

	if( ( old_tracker.track == tracker.track ) && ( old_tracker.untrack == tracker.untrack ) )
		return;

	_memory_tracker = _memory_no_tracker;

	if( old_tracker.shutdown )
		old_tracker.shutdown();

	if( tracker.initialize )
		tracker.initialize();

	_memory_tracker = tracker;
}


#if BUILD_ENABLE_MEMORY_TRACKER

static void _memory_track( void* addr, uint64_t size )
{
	if( _memory_tracker.track )
		_memory_tracker.track( addr, size );
}


static void _memory_untrack( void* addr )
{
	if( _memory_tracker.untrack )
		_memory_tracker.untrack( addr );
}

#endif


struct memory_tag_t
{
	atomicptr_t   address;
	uintptr_t     size;
	void*         trace[14];
};
typedef FOUNDATION_ALIGN(8) struct memory_tag_t memory_tag_t;


static hashtable_t*       _memory_table;
static memory_tag_t*      _memory_tags;
static atomic32_t         _memory_tag_next;


static int _memory_tracker_initialize( void )
{
	log_debug( HASH_MEMORY, "Initializing local memory tracker" );
	if( !_memory_tags )
		_memory_tags = memory_allocate( 0, sizeof( memory_tag_t ) * BUILD_SIZE_MEMORY_TRACKER_MAX_ALLOCATIONS, 16, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	if( !_memory_table )
		_memory_table = hashtable_allocate( BUILD_SIZE_MEMORY_TRACKER_MAX_ALLOCATIONS );
	return 0;
}


static void _memory_tracker_shutdown( void )
{
	if( _memory_table )
		hashtable_deallocate( _memory_table );
	if( _memory_tags )
	{
		unsigned int it;
		bool got_leaks = false;

		log_debug( HASH_MEMORY, "Checking for memory leaks" );
		for( it = 0; it < BUILD_SIZE_MEMORY_TRACKER_MAX_ALLOCATIONS; ++it )
		{
			memory_tag_t* tag = _memory_tags + it;
			if( atomic_loadptr( &tag->address ) )
			{
				char* trace = stacktrace_resolve( tag->trace, 14, 0 );
				log_warnf( HASH_MEMORY, WARNING_MEMORY, "Memory leak: %d bytes @ 0x%" PRIfixPTR " : tag %d\n%s", (unsigned int)tag->size, atomic_loadptr( &tag->address ), it, trace );
				string_deallocate( trace );
				got_leaks = true;
			}
		}
		memory_deallocate( _memory_tags );

		if( !got_leaks )
			log_debug( HASH_MEMORY, "No memory leaks detected" );
	}
}


static void _memory_tracker_track( void* addr, uint64_t size )
{
	if( addr ) do
	{
		int32_t tag = atomic_exchange_and_add32( &_memory_tag_next, 1 );
		if( tag >= BUILD_SIZE_MEMORY_TRACKER_MAX_ALLOCATIONS )
		{
			int32_t newtag = tag % BUILD_SIZE_MEMORY_TRACKER_MAX_ALLOCATIONS;
			atomic_cas32( &_memory_tag_next, newtag, tag + 1 );
			tag = newtag;
		}
		if( !atomic_loadptr( &_memory_tags[ tag ].address ) && atomic_cas_ptr( &_memory_tags[ tag ].address, addr, 0 ) )
		{
			_memory_tags[ tag ].size = (uintptr_t)size;
			stacktrace_capture( _memory_tags[ tag ].trace, 14, 3 );
			hashtable_set( _memory_table, (uintptr_t)addr, (uintptr_t)( tag + 1 ) );
			return;
		}
	} while( true );
}


static void _memory_tracker_untrack( void* addr )
{
	uintptr_t tag = addr ? hashtable_get( _memory_table, (uintptr_t)addr ) : 0;
	if( tag )
	{
		--tag;
		atomic_storeptr( &_memory_tags[ tag ].address, 0 );
	}
	//else if( addr )
	//	log_warnf( HASH_MEMORY, WARNING_SUSPICIOUS, "Untracked deallocation: " PRIfixPTR, addr );
}


#endif


memory_tracker_t memory_tracker_local( void )
{
	memory_tracker_t tracker = _memory_no_tracker;
#if BUILD_ENABLE_MEMORY_TRACKER
	tracker.track = _memory_tracker_track;
	tracker.untrack = _memory_tracker_untrack;
	tracker.initialize = _memory_tracker_initialize;
	tracker.shutdown = _memory_tracker_shutdown;
#endif
	return tracker;
}
