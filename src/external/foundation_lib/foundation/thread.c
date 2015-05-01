/* thread.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
typedef DWORD (WINAPI* GetCurrentProcessorNumberFn)(VOID);
DWORD WINAPI GetCurrentProcessorNumberFallback(VOID) { return 0; }
GetCurrentProcessorNumberFn _fnGetCurrentProcessorNumber = GetCurrentProcessorNumberFallback;
#endif

#if FOUNDATION_PLATFORM_POSIX
#  if !FOUNDATION_PLATFORM_APPLE && !FOUNDATION_PLATFORM_BSD
#    include <sys/prctl.h>
#  endif
#  include <pthread.h>
#endif

#if FOUNDATION_PLATFORM_PNACL
#  include <foundation/pnacl.h>
#endif

#if FOUNDATION_PLATFORM_ANDROID
#  include <foundation/android.h>
#endif

#if FOUNDATION_PLATFORM_BSD
#  include <pthread_np.h>
#endif

#if FOUNDATION_PLATFORM_APPLE || FOUNDATION_PLATFORM_ANDROID || ( FOUNDATION_PLATFORM_WINDOWS && FOUNDATION_COMPILER_CLANG )

struct thread_local_block_t
{
	uint64_t     thread;
	atomicptr_t  block;
};
typedef struct thread_local_block_t thread_local_block_t;

//TODO: Ugly hack, improve this shit
static thread_local_block_t _thread_local_blocks[1024];

void* _allocate_thread_local_block( unsigned int size )
{
	void* block = memory_allocate( 0, size, 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );

	for( int i = 0; i < 1024; ++i )
	{
		if( !atomic_loadptr( &_thread_local_blocks[i].block ) )
		{
			if( atomic_cas_ptr( &_thread_local_blocks[i].block, block, 0 ) )
			{
				_thread_local_blocks[i].thread = thread_id();
				return block;
			}
		}
	}

	log_warnf( 0, WARNING_MEMORY, "Unable to locate thread local memory block slot, will leak %d bytes", size );
	return block;
}

#endif


FOUNDATION_ALIGNED_STRUCT( thread_t, 16 )
{
	FOUNDATION_DECLARE_OBJECT;

	atomic32_t            started; //Aligned to 16 bytes for atomic
	atomic32_t            running;
	atomic32_t            terminate;
	uint32_t              stacksize;
	thread_fn             fn;
	char                  name[32];
	thread_priority_t     priority;
	void*                 arg;
	void*                 result;
	uint64_t              osid;

#if FOUNDATION_PLATFORM_WINDOWS
	HANDLE                handle;
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	pthread_t             thread;
#else
#  error Not implemented
#endif
};
typedef FOUNDATION_ALIGNED_STRUCT( thread_t, 16 ) thread_t;

static uint64_t     _thread_main_id;
static objectmap_t* _thread_map;

#define GET_THREAD( obj ) objectmap_lookup( _thread_map, obj )

FOUNDATION_DECLARE_THREAD_LOCAL( const char*, name, 0 )
FOUNDATION_DECLARE_THREAD_LOCAL( thread_t*, self, 0 )


int _thread_initialize( void )
{
#if FOUNDATION_PLATFORM_WINDOWS
	//TODO: look into GetCurrentProcessorNumberEx for 64+ core support
	GetCurrentProcessorNumberFn getprocidfn = (GetCurrentProcessorNumberFn)GetProcAddress( GetModuleHandleA( "kernel32" ), "GetCurrentProcessorNumber" );
	if( getprocidfn )
		_fnGetCurrentProcessorNumber = getprocidfn;
#endif

	_thread_map = objectmap_allocate( BUILD_SIZE_THREAD_MAP );

	return 0;
}


void _thread_shutdown( void )
{
	objectmap_deallocate( _thread_map );

#if FOUNDATION_PLATFORM_APPLE || FOUNDATION_PLATFORM_ANDROID || ( FOUNDATION_PLATFORM_WINDOWS && FOUNDATION_COMPILER_CLANG )
	for( int i = 0; i < 1024; ++i )
	{
		if( atomic_loadptr( &_thread_local_blocks[i].block ) )
		{
			void* block = atomic_loadptr( &_thread_local_blocks[i].block );
			_thread_local_blocks[i].thread = 0;
			atomic_storeptr( &_thread_local_blocks[i].block, 0 );
			memory_deallocate( block );
		}
	}
#endif

	thread_finalize();
}


static void _thread_destroy( object_t id, void* thread_raw )
{
	thread_t* thread = thread_raw;
	FOUNDATION_UNUSED( id );
	if( !thread )
		return;
	if( thread_is_running( thread->id ) )
	{
		unsigned int spin_count = 0;
		FOUNDATION_ASSERT_FAIL( "Destroying running thread" );
		thread_terminate( thread->id );
		while( thread_is_running( thread->id ) && ( ++spin_count < 50 ) )
			thread_yield();
	}
	objectmap_free( _thread_map, thread->id );
	memory_deallocate( thread );
}


static FOUNDATION_FORCEINLINE void _thread_unref( thread_t* thread )
{
	if( thread )
		objectmap_lookup_unref( _thread_map, thread->id, _thread_destroy );
}


static int _thread_guard_wrapper( void* data )
{
	thread_t* thread = data;
	FOUNDATION_ASSERT( atomic_load32( &thread->running ) == 1 );
	thread->result = thread->fn( thread->id, thread->arg );
	return 0;
}


object_t thread_create( thread_fn fn, const char* name, thread_priority_t priority, unsigned int stacksize )
{
	thread_t* thread;
	uint64_t id = objectmap_reserve( _thread_map );
	if( !id )
	{
		log_error( 0, ERROR_OUT_OF_MEMORY, "Unable to allocate new thread, map full" );
		return 0;
	}
	thread = memory_allocate( 0, sizeof( thread_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	_object_initialize( (object_base_t*)thread, id );
	thread->fn = fn;
	string_copy( thread->name, name, 32 );
	thread->priority = priority;
	thread->stacksize = stacksize;
	objectmap_set( _thread_map, id, thread );
	return thread->id;
}


object_t thread_ref( object_t id )
{
	return _object_ref( GET_THREAD( id ) );
}


void thread_destroy( object_t id )
{
	_thread_unref( GET_THREAD( id ) );
}


bool thread_is_started( object_t id )
{
	thread_t* thread = GET_THREAD( id );
	return ( thread && ( atomic_load32( &thread->started ) > 0 ) );
}


bool thread_is_running( object_t id )
{
	thread_t* thread = GET_THREAD( id );
	return ( thread && ( atomic_load32( &thread->running ) > 0 ) );
}


bool thread_is_thread( object_t id )
{
	thread_t* thread = GET_THREAD( id );
	return ( thread != 0 );
}


void* thread_result( object_t id )
{
	thread_t* thread = GET_THREAD( id );
	return ( !thread || !atomic_load32( &thread->started ) || atomic_load32( &thread->running ) ) ? 0 : thread->result;
}


const char* thread_name( void )
{
	return get_thread_name();
}


#if FOUNDATION_PLATFORM_WINDOWS && !BUILD_DEPLOY

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

#if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
LONG WINAPI _thread_set_name_exception_filter( LPEXCEPTION_POINTERS pointers )
{
	return EXCEPTION_CONTINUE_EXECUTION;
}
#endif

static void FOUNDATION_NOINLINE _set_thread_name( const char* threadname )
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadname;
	info.dwThreadID = -1;
	info.dwFlags = 0;

#if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	LPTOP_LEVEL_EXCEPTION_FILTER prev_filter = SetUnhandledExceptionFilter( _thread_set_name_exception_filter );
#else
	__try
#endif
	{
		RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
	}
#if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	SetUnhandledExceptionFilter( prev_filter );
#else
	__except( EXCEPTION_CONTINUE_EXECUTION ) //Does EXCEPTION_EXECUTE_HANDLER require a debugger present?
	{
		atomic_thread_fence_release();
	}
#endif
}

#endif

void thread_set_name( const char* name )
{
	thread_t* self;

#if !BUILD_DEPLOY
#  if FOUNDATION_PLATFORM_WINDOWS
	_set_thread_name( name );
#  elif FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_ANDROID
	if( !thread_is_main() ) //Don't set main thread (i.e process) name
		prctl( PR_SET_NAME, name, 0, 0, 0 );
#  elif FOUNDATION_PLATFORM_MACOSX || FOUNDATION_PLATFORM_IOS
	if( !thread_is_main() ) //Don't set main thread (i.e process) name
		pthread_setname_np( name );
#  elif FOUNDATION_PLATFORM_BSD
	if( !thread_is_main() ) //Don't set main thread (i.e process) name
		pthread_set_name_np( pthread_self(), name );
#  endif
#endif

	set_thread_name( name );

	self = get_thread_self();
	if( self )
	{
#if !BUILD_DEPLOY
		thread_t* check_self = GET_THREAD( self->id );
		FOUNDATION_ASSERT( self == check_self );
#endif
		string_copy( self->name, name, 32 );
	}
}


object_t thread_self( void )
{
	thread_t* self = get_thread_self();
	return self ? self->id : 0;
}


#if FOUNDATION_PLATFORM_WINDOWS

typedef DWORD thread_return_t;
typedef void* thread_arg_t;
#define FOUNDATION_THREADCALL WINAPI
#define GET_THREAD_PTR(x) ((thread_t*)(x))

#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL

typedef void* thread_return_t;
typedef void* thread_arg_t;
#define FOUNDATION_THREADCALL
#define GET_THREAD_PTR(x) ((thread_t*)(x))

#else
#  error Not implemented
#endif

static thread_return_t FOUNDATION_THREADCALL _thread_entry( thread_arg_t data )
{
	uint64_t thr_osid;
	uint64_t thr_id;
	thread_t* thread = GET_THREAD_PTR( data );

	if( !_object_ref( (object_base_t*)thread ) )
	{
		log_warnf( 0, WARNING_SUSPICIOUS, "Unable to enter thread, invalid thread object %" PRIfixPTR, thread );
		return 0;
	}

	atomic_cas32( &thread->started, 1, 0 );
	if( !atomic_cas32( &thread->running, 1, 0 ) )
	{
		log_warnf( 0, WARNING_SUSPICIOUS, "Unable to enter thread %llx, already running", thread->id );
		_thread_unref( thread );
		return 0;
	}

	thread->osid = thread_id();

#if FOUNDATION_PLATFORM_WINDOWS && !BUILD_DEPLOY
	if( thread->name[0] )
		_set_thread_name( thread->name );
#elif ( FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_ANDROID ) && !BUILD_DEPLOY
	prctl( PR_SET_NAME, thread->name, 0, 0, 0 );
#elif FOUNDATION_PLATFORM_BSD && !BUILD_DEPLOY
	pthread_set_name_np( pthread_self(), thread->name );
#endif
	atomic_store32( &thread->terminate, 0 );

	set_thread_self( thread );

	FOUNDATION_ASSERT( atomic_load32( &thread->running ) == 1 );

	log_debugf( 0, "Started thread '%s' (%llx) ID %llx%s", thread->name, thread->osid, thread->id, crash_guard_callback() ? " (guarded)" : "" );

	if( system_debugger_attached() )
	{
		thread->result = thread->fn( thread->id, thread->arg );
	}
	else
	{
		int crash_result = crash_guard( _thread_guard_wrapper, thread, crash_guard_callback(), crash_guard_name() );
		if( crash_result == FOUNDATION_CRASH_DUMP_GENERATED )
		{
			thread->result = (void*)((uintptr_t)FOUNDATION_CRASH_DUMP_GENERATED);
			log_warnf( 0, WARNING_SUSPICIOUS, "Thread '%s' (%llx) ID %llx crashed", thread->name, thread->osid, thread->id );
		}
	}

	thr_osid = thread->osid;
	thr_id = thread->id;
	log_debugf( 0, "Terminated thread '%s' (%llx) ID %llx with %d refs", thread->name, thr_osid, thr_id, thread->ref );

	thread->osid  = 0;

	set_thread_self( 0 );
	thread_finalize();

	if( !atomic_cas32( &thread->running, 0, 1 ) )
	{
		FOUNDATION_ASSERT_FAIL( "Unable to reset running flag" );
		atomic_store32( &thread->running, 0 );
	}

	log_debugf( 0, "Exiting thread '%s' (%llx) ID %llx with %d refs", thread->name, thr_osid, thr_id, thread->ref );

	_thread_unref( thread );

	FOUNDATION_UNUSED(thr_osid);
	FOUNDATION_UNUSED(thr_id);

	return 0;
}


bool thread_start( object_t id, void* data )
{
#if FOUNDATION_PLATFORM_WINDOWS
	unsigned long osid = 0;
#endif
	thread_t* thread = GET_THREAD( id );
	if( !thread )
	{
		log_errorf( 0, ERROR_INVALID_VALUE, "Unable to start thread %llx, invalid id", id );
		return false; //Old/invalid id
	}

	if( atomic_load32( &thread->running ) > 0 )
	{
		log_warnf( 0, WARNING_SUSPICIOUS, "Unable to start thread %llx, already running", id );
		return false; //Thread already running
	}

	atomic_cas32( &thread->started, 0, 1 );

	thread->arg = data;

	if( !thread->stacksize )
		thread->stacksize = BUILD_SIZE_DEFAULT_THREAD_STACK;

#if FOUNDATION_PLATFORM_WINDOWS
	thread->handle = CreateThread( 0, thread->stacksize, _thread_entry, thread, 0, &osid );
	if( !thread->handle )
	{
		log_errorf( 0, ERROR_OUT_OF_MEMORY, "Unable to create thread: CreateThread failed: %s", system_error_message( GetLastError() ) );
		return false;
	}
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	int err = pthread_create( &thread->thread, 0, _thread_entry, thread );
	if( err )
	{
		log_errorf( 0, ERROR_OUT_OF_MEMORY, "Unable to create thread: pthread_create failed: %s", system_error_message( err ) );
		return false;
	}
#else
#  error Not implemented
#endif

	return true;
}


void thread_terminate( object_t id )
{
	thread_t* thread = GET_THREAD( id );
	if( thread )
		atomic_store32( &thread->terminate, 1 );
}


bool thread_should_terminate( object_t id )
{
	thread_t* thread = GET_THREAD( id );
	if( thread )
		return atomic_load32( &thread->terminate )  > 0;
	return true;
}


void thread_sleep( int milliseconds )
{
#if FOUNDATION_PLATFORM_WINDOWS
	SleepEx( milliseconds, 1 );
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	struct timespec ts;
	ts.tv_sec  = milliseconds / 1000;
	ts.tv_nsec = (long)( milliseconds % 1000 ) * 1000000L;
	nanosleep( &ts, 0 );
#else
#  error Not implemented
#endif
}


void thread_yield( void )
{
#if FOUNDATION_PLATFORM_WINDOWS
	Sleep(0);
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	sched_yield();
#else
#  error Not implemented
#endif
}


uint64_t thread_id( void )
{
#if FOUNDATION_PLATFORM_WINDOWS
	return GetCurrentThreadId();
#elif FOUNDATION_PLATFORM_APPLE
	return pthread_mach_thread_np( pthread_self() );
#elif FOUNDATION_PLATFORM_BSD
	return pthread_getthreadid_np();
#elif FOUNDATION_PLATFORM_POSIX
	if( sizeof( pthread_t ) < 8 )
		return (uint64_t)pthread_self() & 0x00000000FFFFFFFFULL;
	else
		return pthread_self();
#elif FOUNDATION_PLATFORM_PNACL
	return (uintptr_t)pthread_self();
#else
#  error Not implemented
#endif
}


unsigned int thread_hardware( void )
{
#if FOUNDATION_PLATFORM_WINDOWS
	return _fnGetCurrentProcessorNumber();
#elif FOUNDATION_PLATFORM_LINUX
	return sched_getcpu();
#else
	//TODO: Implement for other platforms
	return 0;
#endif
}


void thread_set_hardware( uint64_t mask )
{
	//TODO: Implement
	FOUNDATION_UNUSED( mask );
}


void thread_set_main( void )
{
	_thread_main_id = thread_id();
}


bool thread_is_main( void )
{
	return thread_id() == _thread_main_id;
}


void thread_finalize( void )
{
	_profile_thread_finalize();

	system_thread_deallocate();
	random_thread_deallocate();

#if FOUNDATION_PLATFORM_ANDROID
	thread_detach_jvm();
#endif

	error_context_thread_deallocate();
	memory_context_thread_deallocate();

#if FOUNDATION_PLATFORM_MACOSX || FOUNDATION_PLATFORM_IOS
	uint64_t curid = thread_id();
	for( int i = 0; i < 1024; ++i )
	{
		if( ( _thread_local_blocks[i].thread == curid ) && atomic_loadptr( &_thread_local_blocks[i].block ) )
		{
			void* block = atomic_loadptr( &_thread_local_blocks[i].block );
			_thread_local_blocks[i].thread = 0;
			atomic_storeptr( &_thread_local_blocks[i].block, 0 );
			memory_deallocate( block );
		}
	}
#endif
}


#if FOUNDATION_PLATFORM_ANDROID

#include <android/native_activity.h>


void* thread_attach_jvm( void )
{
	JavaVMAttachArgs attach_args;
	struct android_app* app = android_app();
	void* env = 0;

	(*app->activity->vm)->GetEnv( app->activity->vm, &env, JNI_VERSION_1_6 );
	if( env )
		return env;

	attach_args.version = JNI_VERSION_1_6;
	attach_args.name = "NativeThread";
	attach_args.group = 0;

	// Attaches the current thread to the JVM
	// TODO: According to the native activity, the java env can only be used in the main thread (calling ANativeActivityCallbacks)
	jint result = (*app->activity->vm)->AttachCurrentThread( app->activity->vm, (const struct JNINativeInterface ***)&env, &attach_args );
	if( result < 0 )
		log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to attach thread to Java VM (%d)", result );

	return env;
}


void thread_detach_jvm( void )
{
	JavaVM* java_vm = android_app()->activity->vm;
	(*java_vm)->DetachCurrentThread( java_vm );
}

#endif
