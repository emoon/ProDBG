/* atomic.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#pragma once

#include <foundation/platform.h>
#include <foundation/types.h>


#if FOUNDATION_PLATFORM_APPLE
#  include <libkern/OSAtomic.h>
#endif

#if FOUNDATION_ARCH_MIPS || ( FOUNDATION_PLATFORM_LINUX_RASPBERRYPI && FOUNDATION_COMPILER_GCC && ( __GNUC__ <=4 || ( __GNUC__ == 4 && __GNUC_MINOR__ < 8 ) ) )
#  define FOUNDATION_MUTEX_64BIT_ATOMIC 1
FOUNDATION_API uint64_t __foundation_sync_fetch_and_add_8( uint64_t* val, uint64_t add );
FOUNDATION_API uint64_t __foundation_sync_add_and_fetch_8( uint64_t* val, uint64_t add );
FOUNDATION_API bool     __foundation_sync_bool_compare_and_swap_8( uint64_t* val, uint64_t oldval, uint64_t newval );
#else
#  define FOUNDATION_MUTEX_64BIT_ATOMIC 0
#endif


static FOUNDATION_FORCEINLINE int32_t      atomic_load32( atomic32_t* src );
static FOUNDATION_FORCEINLINE int64_t      atomic_load64( atomic64_t* src );
static FOUNDATION_FORCEINLINE void*        atomic_loadptr( atomicptr_t* src );

static FOUNDATION_FORCEINLINE void         atomic_store32( atomic32_t* dst, int32_t val );
static FOUNDATION_FORCEINLINE void         atomic_store64( atomic64_t* dst, int64_t val );
static FOUNDATION_FORCEINLINE void         atomic_storeptr( atomicptr_t* dst, void* val );

static FOUNDATION_FORCEINLINE int32_t      atomic_exchange_and_add32( atomic32_t* val, int32_t add );
static FOUNDATION_FORCEINLINE int64_t      atomic_exchange_and_add64( atomic64_t* val, int64_t add );

static FOUNDATION_FORCEINLINE int32_t      atomic_add32( atomic32_t* val, int32_t add );
static FOUNDATION_FORCEINLINE int64_t      atomic_add64( atomic64_t* val, int64_t add );

static FOUNDATION_FORCEINLINE int32_t      atomic_incr32( atomic32_t* val );
static FOUNDATION_FORCEINLINE int64_t      atomic_incr64( atomic64_t* val );

static FOUNDATION_FORCEINLINE int32_t      atomic_decr32( atomic32_t* val );
static FOUNDATION_FORCEINLINE int64_t      atomic_decr64( atomic64_t* val );

static FOUNDATION_FORCEINLINE bool         atomic_cas32( atomic32_t* dst, int32_t val, int32_t ref );
static FOUNDATION_FORCEINLINE bool         atomic_cas64( atomic64_t* dst, int64_t val, int64_t ref );
static FOUNDATION_FORCEINLINE bool         atomic_cas_ptr( atomicptr_t* dst, void* val, void* ref );

static FOUNDATION_FORCEINLINE void         atomic_signal_fence_acquire( void );
static FOUNDATION_FORCEINLINE void         atomic_signal_fence_release( void );
static FOUNDATION_FORCEINLINE void         atomic_signal_fence_sequentially_consistent( void );

static FOUNDATION_FORCEINLINE void         atomic_thread_fence_acquire( void );
static FOUNDATION_FORCEINLINE void         atomic_thread_fence_release( void );
static FOUNDATION_FORCEINLINE void         atomic_thread_fence_sequentially_consistent( void );


// Implementations

static FOUNDATION_FORCEINLINE int32_t atomic_load32( atomic32_t* val )
{
	return val->nonatomic;
}


static FOUNDATION_FORCEINLINE int64_t atomic_load64( atomic64_t* val )
{
#if FOUNDATION_ARCH_X86
	int64_t result;
#  if FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL
    __asm
	{
		mov esi, val;
		mov ebx, eax;
		mov ecx, edx;
		lock cmpxchg8b [esi];
		mov dword ptr result, eax;
		mov dword ptr result[4], edx;
	}
#  else
	__asm volatile(
		"movl %%ebx, %%eax\n"
		"movl %%ecx, %%edx\n"
		"lock; cmpxchg8b %1"
		: "=&A"(result)
		: "m"(val->nonatomic));
#  endif
	return result;
#else
	return val->nonatomic;
#endif
}


static FOUNDATION_FORCEINLINE void* atomic_loadptr( atomicptr_t* val )
{
	return val->nonatomic;
}


static FOUNDATION_FORCEINLINE void atomic_store32( atomic32_t* dst, int32_t val )
{
	dst->nonatomic = val;
}


static FOUNDATION_FORCEINLINE void atomic_store64( atomic64_t* dst, int64_t val )
{
#if FOUNDATION_ARCH_X86
#  if FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL
	__asm
	{
		mov esi, dst;
		mov ebx, dword ptr val;
		mov ecx, dword ptr val[4];
	retry:
		cmpxchg8b [esi];
		jne retry;
	}
#  else
	uint64_t expected = dst->nonatomic;
#    if FOUNDATION_COMPILER_GCC
  __sync_bool_compare_and_swap( &dst->nonatomic, expected, val );
#    else
	__asm volatile(
		"1:    cmpxchg8b %0\n"
		"      jne 1b"
		: "=m"(dst->nonatomic)
		: "b"((uint32_t)val), "c"((uint32_t)(val >> 32)), "A"(expected));
#    endif
#  endif
#else
	dst->nonatomic = val;
#endif
}


static FOUNDATION_FORCEINLINE void atomic_storeptr( atomicptr_t* dst, void* val )
{
	dst->nonatomic = val;
}


static FOUNDATION_FORCEINLINE int32_t atomic_exchange_and_add32( atomic32_t* val, int32_t add )
{
#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL )
	return _InterlockedExchangeAdd( (volatile long*)&val->nonatomic, add );
#elif FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	return __sync_fetch_and_add( &val->nonatomic, add );
#else
#  error Not implemented
#endif
}


static FOUNDATION_FORCEINLINE int atomic_add32( atomic32_t* val, int32_t add )
{
#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL )
	int32_t old = (int32_t)_InterlockedExchangeAdd( (volatile long*)&val->nonatomic, add );
	return ( old + add );
#elif FOUNDATION_PLATFORM_APPLE
	return OSAtomicAdd32( add, (int*)&val->nonatomic );
#elif FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	return __sync_add_and_fetch( &val->nonatomic, add );
#else
#  error Not implemented
#endif
}

static FOUNDATION_FORCEINLINE int atomic_incr32( atomic32_t* val ) { return atomic_add32( val, 1 ); }
static FOUNDATION_FORCEINLINE int atomic_decr32( atomic32_t* val ) { return atomic_add32( val, -1 ); }


static FOUNDATION_FORCEINLINE int64_t atomic_exchange_and_add64( atomic64_t* val, int64_t add )
{
#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL )
#  if FOUNDATION_ARCH_X86
	long long ref;
	do { ref = val->nonatomic; } while( _InterlockedCompareExchange64( (volatile long long*)&val->nonatomic, ref + add, ref ) != ref );
	return ref;
#  else //X86_64
	return _InterlockedExchangeAdd64( &val->nonatomic, add );
#  endif
#elif FOUNDATION_PLATFORM_APPLE
	int64_t ref;
	do { ref = (int64_t)val->nonatomic; } while( !OSAtomicCompareAndSwap64( ref, ref + add, (int64_t*)&val->nonatomic ) );
	return ref;
#elif FOUNDATION_MUTEX_64BIT_ATOMIC
	return __foundation_sync_fetch_and_add_8( &val->nonatomic, add );
#elif FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	return __sync_fetch_and_add( &val->nonatomic, add );
#else
#  error Not implemented
#endif
}


static FOUNDATION_FORCEINLINE int64_t atomic_add64( atomic64_t* val, int64_t add )
{
#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL )
#  if FOUNDATION_ARCH_X86
	return atomic_exchange_and_add64( val, add ) + add;
#  else
	return _InterlockedExchangeAdd64( &val->nonatomic, add ) + add;
#endif
#elif FOUNDATION_PLATFORM_APPLE
	return OSAtomicAdd64( add, (int64_t*)&val->nonatomic );
#elif FOUNDATION_MUTEX_64BIT_ATOMIC
	return __foundation_sync_add_and_fetch_8( &val->nonatomic, add );
#elif FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	return __sync_add_and_fetch( &val->nonatomic, add );
#else
#  error Not implemented
#endif
}

static FOUNDATION_FORCEINLINE int64_t atomic_incr64( atomic64_t* val ) { return atomic_add64( val, 1LL ); }
static FOUNDATION_FORCEINLINE int64_t atomic_decr64( atomic64_t* val ) { return atomic_add64( val, -1LL ); }


static FOUNDATION_FORCEINLINE bool atomic_cas32( atomic32_t* dst, int32_t val, int32_t ref )
{
#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL )
	return ( _InterlockedCompareExchange( (volatile long*)&dst->nonatomic, val, ref ) == ref ) ? true : false;
#elif FOUNDATION_PLATFORM_APPLE
	return OSAtomicCompareAndSwap32( ref, val, (int32_t*)&dst->nonatomic );
#elif FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	return __sync_bool_compare_and_swap( &dst->nonatomic, ref, val );
#else
#  error Not implemented
#endif
}


static FOUNDATION_FORCEINLINE bool atomic_cas64( atomic64_t* dst, int64_t val, int64_t ref )
{
#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL )
	return ( _InterlockedCompareExchange64( (volatile long long*)&dst->nonatomic, val, ref ) == ref ) ? true : false;
#elif FOUNDATION_PLATFORM_APPLE
	return OSAtomicCompareAndSwap64( ref, val, (int64_t*)&dst->nonatomic );
#elif FOUNDATION_MUTEX_64BIT_ATOMIC
	return __foundation_sync_bool_compare_and_swap_8( &dst->nonatomic, ref, val );
#elif FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	return __sync_bool_compare_and_swap( &dst->nonatomic, ref, val );
#else
#  error Not implemented
#endif
}


static FOUNDATION_FORCEINLINE bool atomic_cas_ptr( atomicptr_t* dst, void* val, void* ref )
{
#  if FOUNDATION_SIZE_POINTER == 8
	return atomic_cas64( (atomic64_t*)dst, (int64_t)(uintptr_t)val, (int64_t)(uintptr_t)ref );
#  elif FOUNDATION_SIZE_POINTER == 4
	return atomic_cas32( (atomic32_t*)dst, (int32_t)(uintptr_t)val, (int32_t)(uintptr_t)ref );
#  else
#    error Unknown architecture (pointer size)
#  endif
}


static FOUNDATION_FORCEINLINE void atomic_signal_fence_acquire( void ) {}
static FOUNDATION_FORCEINLINE void atomic_signal_fence_release( void ) {}
static FOUNDATION_FORCEINLINE void atomic_signal_fence_sequentially_consistent( void ) {}

#if !FOUNDATION_ARCH_ARM || !FOUNDATION_ARCH_THUMB
static FOUNDATION_FORCEINLINE void atomic_thread_fence_acquire( void ) {}
static FOUNDATION_FORCEINLINE void atomic_thread_fence_release( void ) {}
static FOUNDATION_FORCEINLINE void atomic_thread_fence_sequentially_consistent( void ) {}
#endif


#if FOUNDATION_PLATFORM_WINDOWS

#define atomic_signal_fence_acquire() _ReadWriteBarrier()
#define atomic_signal_fence_release() _ReadWriteBarrier()
#define atomic_signal_fence_sequentially_consistent() _ReadWriteBarrier()
#define atomic_thread_fence_acquire() _ReadWriteBarrier()
#define atomic_thread_fence_release() _ReadWriteBarrier()
#define atomic_thread_fence_sequentially_consistent() MemoryBarrier()

#else

#define atomic_signal_fence_acquire() __asm volatile("" ::: "memory")
#define atomic_signal_fence_release() __asm volatile("" ::: "memory")
#define atomic_signal_fence_sequentially_consistent() __asm volatile("" ::: "memory")

#  if FOUNDATION_ARCH_ARM5 || FOUNDATION_ARCH_ARM6

// Fences compiled as standalone functions

#  elif FOUNDATION_ARCH_ARM

#define atomic_thread_fence_acquire() __asm volatile("dmb sy" ::: "memory")
#define atomic_thread_fence_release() __asm volatile("dmb st" ::: "memory")
#define atomic_thread_fence_sequentially_consistent() __asm volatile("dmb sy" ::: "memory")

#  else

#define atomic_thread_fence_acquire() __asm volatile("" ::: "memory")
#define atomic_thread_fence_release() __asm volatile("" ::: "memory")

#    if FOUNDATION_ARCH_MIPS
#define atomic_thread_fence_sequentially_consistent() __asm volatile("sync" ::: "memory")
#    elif FOUNDATION_ARCH_X86_64
#define atomic_thread_fence_sequentially_consistent() __asm volatile("lock; orl $0, (%%rsp)" ::: "memory")
#    elif FOUNDATION_ARCH_X86
#define atomic_thread_fence_sequentially_consistent() __asm volatile("lock; orl $0, (%%esp)" ::: "memory")
#    elif FOUNDATION_PLATFORM_PNACL
#define atomic_thread_fence_sequentially_consistent() __asm volatile("sync" ::: "memory")
#    else
#error atomic_thread_fence_sequentially_consistent not implemented for architecture
#    endif

#  endif

#endif
