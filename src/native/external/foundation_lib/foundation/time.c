/* time.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#elif FOUNDATION_PLATFORM_APPLE
#  include <foundation/apple.h>
#  include <mach/mach_time.h>
static mach_timebase_info_data_t _time_info;
static void absolutetime_to_nanoseconds (uint64_t mach_time, uint64_t* clock ) { *clock = mach_time * _time_info.numer / _time_info.denom; }
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
#  include <unistd.h>
#  include <time.h>
#  include <string.h>
#else
#  error Not implemented on this platform!
#endif

static tick_t _time_freq;
static double _time_oofreq;
static tick_t _time_startup;


int _time_initialize( void )
{
#if FOUNDATION_PLATFORM_WINDOWS
	tick_t unused;
	if( !QueryPerformanceFrequency( (LARGE_INTEGER*)&_time_freq ) ||
	    !QueryPerformanceCounter( (LARGE_INTEGER*)&unused ) )
		return -1;
#elif FOUNDATION_PLATFORM_APPLE
	if( mach_timebase_info( &_time_info ) )
		return -1;
	_time_freq = 1000000000ULL;
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	if( clock_gettime( CLOCK_MONOTONIC, &ts ) )
		return -1;
	_time_freq = 1000000000ULL;
#else
#  error Not implemented
#endif

	_time_oofreq  = 1.0 / (double)_time_freq;
	_time_startup = time_current();

	return 0;
}


void _time_shutdown( void )
{
}


tick_t time_current( void )
{
#if FOUNDATION_PLATFORM_WINDOWS

	tick_t curclock;
	QueryPerformanceCounter( (LARGE_INTEGER*)&curclock );
	return curclock;

#elif FOUNDATION_PLATFORM_APPLE

	tick_t curclock = 0;
	absolutetime_to_nanoseconds( mach_absolute_time(), &curclock );
	return curclock;

#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL

	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	clock_gettime( CLOCK_MONOTONIC, &ts );
	return ( (uint64_t)ts.tv_sec * 1000000000ULL ) + ts.tv_nsec;

#else
#  error Not implemented
#endif
}


tick_t time_startup( void )
{
	return _time_startup;
}


tick_t time_ticks_per_second( void )
{
	return _time_freq;
}


tick_t time_diff( const tick_t from, const tick_t to )
{
	if( to <= from )
		return 0;
	return ( to - from );
}


deltatime_t time_elapsed( const tick_t t )
{
	return (deltatime_t)( (double)time_elapsed_ticks( t ) * _time_oofreq );
}


tick_t time_elapsed_ticks( const tick_t t )
{
	tick_t dt = 0;

#if FOUNDATION_PLATFORM_WINDOWS

	tick_t curclock = t;
	QueryPerformanceCounter( (LARGE_INTEGER*)&curclock );
	dt = curclock - t;

#elif FOUNDATION_PLATFORM_APPLE

	tick_t curclock = t;
	absolutetime_to_nanoseconds( mach_absolute_time(), &curclock );
	dt = curclock - t;

#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL

	tick_t curclock;
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	clock_gettime( CLOCK_MONOTONIC, &ts );

	curclock = ( (tick_t)ts.tv_sec * 1000000000ULL ) + ts.tv_nsec;
	dt = curclock - t;

#else
#  error Not implemented
#endif

	return dt;
}


deltatime_t time_ticks_to_seconds( const tick_t dt )
{
	return (deltatime_t)( (double)dt * _time_oofreq );
}


#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL )
struct __timeb64 {
	__time64_t time;
	unsigned short millitm;
	short timezone;
	short dstflag;
	};
_CRTIMP errno_t __cdecl _ftime64_s(_Out_ struct __timeb64 * _Time);
#endif

tick_t time_system( void )
{
#if FOUNDATION_PLATFORM_WINDOWS

	struct __timeb64 tb;
	_ftime64_s( &tb );
	return ( (tick_t)tb.time * 1000ULL ) + (tick_t)tb.millitm;

#elif FOUNDATION_PLATFORM_APPLE

	tick_t curclock = 0;
	absolutetime_to_nanoseconds( mach_absolute_time(), &curclock );
	return ( curclock / 1000000ULL );

#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL

	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	clock_gettime( CLOCK_REALTIME, &ts );
	return ( (uint64_t)ts.tv_sec * 1000ULL ) + ( ts.tv_nsec / 1000000ULL );

#else
#  error Not implemented
#endif
}
