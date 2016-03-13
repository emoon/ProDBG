/* atomic.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


#if FOUNDATION_ARCH_ARM5 || FOUNDATION_ARCH_ARM6

__asm__(
	"   .align 2\n"
	"   .globl _atomic_thread_fence_acquire\n"
	"   .globl _atomic_thread_fence_release\n"
	"   .globl _atomic_thread_fence_sequentially_consistent\n"
	"_atomic_thread_fence_acquire:\n"
	"_atomic_thread_fence_release:\n"
	"_atomic_thread_fence_sequentially_consistent:\n"
	"   mov r0, #0\n"
	"   mcr p15, 0, r0, c7, c10, 5\n"
	"   bx lr\n"
);

#endif


#if FOUNDATION_MUTEX_64BIT_ATOMIC

#include <foundation/posix.h>

static  pthread_mutex_t _atomic_mutex;


uint64_t __foundation_sync_fetch_and_add_8( uint64_t* val, uint64_t add )
{
	pthread_mutex_lock( &_atomic_mutex );
	uint64_t prev = *val;
	*val += add;
	pthread_mutex_unlock( &_atomic_mutex );
	return prev;
}


uint64_t __foundation_sync_add_and_fetch_8( uint64_t* val, uint64_t add )
{
	pthread_mutex_lock( &_atomic_mutex );
	uint64_t ret = ( *val += add );
	pthread_mutex_unlock( &_atomic_mutex );
	return ret;
}


bool __foundation_sync_bool_compare_and_swap_8( uint64_t* val, uint64_t oldval, uint64_t newval )
{
	bool res = false;
	pthread_mutex_lock( &_atomic_mutex );
	if( *val == oldval )
	{
		*val = newval;
		res = true;
	}
	pthread_mutex_unlock( &_atomic_mutex );
	return res;
}

#endif


int _atomic_initialize( void )
{
#if FOUNDATION_ARCH_MIPS
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutex_init( &_atomic_mutex, &attr );
	pthread_mutexattr_destroy( &attr );
#endif
	return 0;
}


void _atomic_shutdown( void )
{
#if FOUNDATION_ARCH_MIPS
	pthread_mutex_destroy( &_atomic_mutex );
#endif
}

