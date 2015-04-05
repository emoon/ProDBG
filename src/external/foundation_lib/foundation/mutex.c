/* mutex.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#if FOUNDATION_PLATFORM_WINDOWS
#  include <foundation/windows.h>
#elif FOUNDATION_PLATFORM_POSIX
#  include <foundation/posix.h>
#elif FOUNDATION_PLATFORM_PNACL
#  include <foundation/pnacl.h>
#endif


struct FOUNDATION_ALIGN(16) mutex_t
{
	//! Mutex name
	char                   name[32];

#if FOUNDATION_PLATFORM_WINDOWS

	//! Critical section
	unsigned char          csection[32];

	//! Event handle
	void*                  event;

	//! Wait count
	atomic32_t             waiting;

#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL

	//! Mutex object
	pthread_mutex_t        mutex;

	//! Condition object
	pthread_cond_t         cond;

	//! Pending signal
	volatile bool          pending;

#else
#  error Not implemented
#endif

	//! Enter count
	volatile int           lockcount;

	//! Owner thread
	uint64_t               lockedthread;
};


static void _mutex_initialize( mutex_t* mutex, const char* name )
{
	string_copy( mutex->name, name, 32 );

#if FOUNDATION_PLATFORM_WINDOWS
	InitializeCriticalSectionAndSpinCount( (CRITICAL_SECTION*)mutex->csection, 4000 );
	mutex->event = CreateEvent( 0, TRUE, FALSE, 0 );
	atomic_store32( &mutex->waiting, 0 );
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	mutex->pending = false;

	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

	pthread_cond_init( &mutex->cond, 0 );
	pthread_mutex_init( &mutex->mutex, &attr );

	pthread_mutexattr_destroy( &attr );
#else
#  error _mutex_initialize not implemented
#endif

	mutex->lockcount = 0;
	mutex->lockedthread = 0;
}


static void _mutex_shutdown( mutex_t* mutex )
{
	FOUNDATION_ASSERT( !mutex->lockcount );
#if FOUNDATION_PLATFORM_WINDOWS
	CloseHandle( mutex->event );
	DeleteCriticalSection( (CRITICAL_SECTION*)mutex->csection );
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	pthread_mutex_destroy( &mutex->mutex );
	pthread_cond_destroy( &mutex->cond );
#else
#  error mutex_deallocate not implemented
#endif
}


mutex_t* mutex_allocate( const char* name )
{
	mutex_t* mutex = memory_allocate( 0, sizeof( mutex_t ), 16, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );

	_mutex_initialize( mutex, name );

	return mutex;
}


void mutex_deallocate( mutex_t* mutex )
{
	if( !mutex )
		return;

	_mutex_shutdown( mutex );

	memory_deallocate( mutex );
}


const char* mutex_name( mutex_t* mutex )
{
	FOUNDATION_ASSERT( mutex );
	return mutex->name;
}


bool mutex_try_lock( mutex_t* mutex )
{
	bool was_locked = false;
	FOUNDATION_ASSERT( mutex );

#if !BUILD_DEPLOY
	profile_trylock( mutex->name );
#endif

#if FOUNDATION_PLATFORM_WINDOWS
	was_locked = TryEnterCriticalSection( (CRITICAL_SECTION*)mutex->csection );
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	was_locked = ( pthread_mutex_trylock( &mutex->mutex ) == 0 );
#else
#  error mutex_try_lock not implemented
#endif
#if !BUILD_DEPLOY
	if( was_locked )
		profile_lock( mutex->name );
#endif
	if( was_locked )
	{
		FOUNDATION_ASSERT( !mutex->lockcount || ( thread_id() == mutex->lockedthread ) );
		if( !mutex->lockcount )
			mutex->lockedthread = thread_id();
		++mutex->lockcount;
	}
	return was_locked;
}


bool mutex_lock( mutex_t* mutex )
{
	FOUNDATION_ASSERT( mutex );

#if !BUILD_DEPLOY
	profile_trylock( mutex->name );
#endif

#if FOUNDATION_PLATFORM_WINDOWS
	EnterCriticalSection( (CRITICAL_SECTION*)mutex->csection );
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	if( pthread_mutex_lock( &mutex->mutex ) != 0 )
	{
		FOUNDATION_ASSERT_FAILFORMAT( "unable to lock mutex %s", mutex->name );
		return false;
	}
#else
#  error mutex_lock not implemented
#endif
#if !BUILD_DEPLOY
	profile_lock( mutex->name );
#endif

	FOUNDATION_ASSERT_MSGFORMAT( !mutex->lockcount || ( thread_id() == mutex->lockedthread ), "Mutex lock acquired with lockcount > 0 (%d) and locked thread not self (%llx != %llx)", mutex->lockcount, mutex->lockedthread, thread_id() );
	if( !mutex->lockcount )
		mutex->lockedthread = thread_id();
	++mutex->lockcount;

	return true;
}


bool mutex_unlock( mutex_t* mutex )
{
	FOUNDATION_ASSERT( mutex );

	if( !mutex->lockcount )
	{
		log_warnf( 0, WARNING_SUSPICIOUS, "Unable to unlock unlocked mutex %s", mutex->name );
		return false;
	}

	FOUNDATION_ASSERT( mutex->lockedthread == thread_id() );
	--mutex->lockcount;

#if !BUILD_DEPLOY
	profile_unlock( mutex->name );
#endif

#if FOUNDATION_PLATFORM_WINDOWS
	LeaveCriticalSection( (CRITICAL_SECTION*)mutex->csection );
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	if( pthread_mutex_unlock( &mutex->mutex ) != 0 )
	{
		FOUNDATION_ASSERT_FAILFORMAT( "unable to unlock mutex %s", mutex->name );
		return false;
	}
#else
#  error mutex_unlock not implemented
#endif
	return true;
}


bool mutex_wait( mutex_t* mutex, unsigned int timeout )
{
#if FOUNDATION_PLATFORM_WINDOWS
	DWORD ret;
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	struct timeval now;
	struct timespec then;
#endif
	FOUNDATION_ASSERT( mutex );
#if FOUNDATION_PLATFORM_WINDOWS

#if !BUILD_DEPLOY
	profile_wait( mutex->name );
#endif

	atomic_incr32( &mutex->waiting );

	ret = WaitForSingleObject( mutex->event, ( timeout == 0 ) ? INFINITE : timeout );

	if( ret == WAIT_OBJECT_0 )
		mutex_lock( mutex );

	if( atomic_decr32( &mutex->waiting ) == 0 )
		ResetEvent( mutex->event );

	return ret == WAIT_OBJECT_0;

#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL

	mutex_lock( mutex );

	if( mutex->pending )
	{
		mutex->pending = false;
		return true;
	}

	--mutex->lockcount;

	bool was_signal = false;
	if( !timeout )
	{
		int ret = pthread_cond_wait( &mutex->cond, &mutex->mutex );
		if( ret == 0 )
		{
			was_signal = true;
		}
		else
		{
			log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to wait on mutex '%s': %s (%d)", mutex->name, system_error_message( ret ), ret );
		}
	}
	else
	{
		int ret;
		gettimeofday( &now, 0 );
		then.tv_sec  = now.tv_sec + ( timeout / 1000 );
		then.tv_nsec = ( now.tv_usec * 1000 ) + (long)( timeout % 1000 ) * 1000000L;
		while( then.tv_nsec > 999999999 )
		{
			++then.tv_sec;
			then.tv_nsec -= 1000000000L;
		}
		ret = pthread_cond_timedwait( &mutex->cond, &mutex->mutex, &then );
		if( ret == 0 )
		{
			was_signal = true;
		}
		else if( ret != ETIMEDOUT )
		{
			log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to wait (timed) on mutex '%s': %s (%d)", mutex->name, system_error_message( ret ), ret );
		}
	}

	++mutex->lockcount;
	mutex->lockedthread = thread_id();

	if( was_signal )
		mutex->pending = false;
	else
		mutex_unlock( mutex );

	return was_signal;

#else
#  error mutex_wait not implemented
#endif
}


void mutex_signal( mutex_t* mutex )
{
	FOUNDATION_ASSERT( mutex );

#if !BUILD_DEPLOY
	profile_signal( mutex->name );
#endif

#if FOUNDATION_PLATFORM_WINDOWS

	SetEvent( mutex->event );

#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL

	mutex_lock( mutex );
	mutex->pending = true;

	int ret = pthread_cond_broadcast( &mutex->cond );
	if( ret != 0 )
		log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to signal mutex '%s': %s (%d)", mutex->name, system_error_message( ret ), ret );

	mutex_unlock( mutex );

#else
#  error mutex_signal not implemented
#endif
}


#if FOUNDATION_PLATFORM_WINDOWS

void* mutex_event_object( mutex_t* mutex )
{
	FOUNDATION_ASSERT( mutex );
	return mutex->event;
}

#endif

