/* crash.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


static crash_dump_callback_fn  _crash_dump_callback;
static const char*             _crash_dump_name;

#if FOUNDATION_PLATFORM_WINDOWS || ( FOUNDATION_PLATFORM_POSIX /*&& !FOUNDATION_PLATFORM_APPLE*/ )
static char                    _crash_dump_file[FOUNDATION_MAX_PATHLEN+128];
#endif


void crash_guard_set( crash_dump_callback_fn callback, const char* name )
{
	_crash_dump_callback = callback;
	_crash_dump_name     = name;
}


const char* crash_guard_name( void )
{
	return _crash_dump_name;
}


crash_dump_callback_fn crash_guard_callback( void )
{
	return _crash_dump_callback;
}


#if FOUNDATION_PLATFORM_WINDOWS

#  include <foundation/windows.h>
#  include <stdio.h>
#  include <stdarg.h>

typedef BOOL ( STDCALL *MiniDumpWriteDumpFn )( HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, CONST PMINIDUMP_EXCEPTION_INFORMATION, CONST PMINIDUMP_USER_STREAM_INFORMATION, CONST PMINIDUMP_CALLBACK_INFORMATION );

static void _crash_create_mini_dump( EXCEPTION_POINTERS* pointers, const char* name, char* dump_file )
{
	MINIDUMP_EXCEPTION_INFORMATION info;

	HANDLE     file;
	SYSTEMTIME local_time;

	GetLocalTime( &local_time );

	dump_file[0] = 0;
	if( !name )
		name = environment_application()->short_name;
	string_format_buffer( dump_file, FOUNDATION_MAX_PATHLEN + 128, "%s/%s%s%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
		environment_temporary_directory(), name ? name : "", name ? "-" : "",
		string_from_uuid_static( environment_application()->instance ),
		local_time.wYear, local_time.wMonth, local_time.wDay,
		local_time.wHour, local_time.wMinute, local_time.wSecond,
		GetCurrentProcessId(), GetCurrentThreadId());
	fs_make_directory( environment_temporary_directory() );
	file = CreateFileA( dump_file, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0 );

	if( file && ( file != INVALID_HANDLE_VALUE ) )
	{
		BOOL    success = FALSE;
		HMODULE lib     = LoadLibraryA( "dbghelp.dll" );

		if( lib )
		{
			MiniDumpWriteDumpFn CallMiniDumpWriteDump = (MiniDumpWriteDumpFn)GetProcAddress( lib, "MiniDumpWriteDump" );
			if( CallMiniDumpWriteDump )
			{
				info.ThreadId          = GetCurrentThreadId();
				info.ExceptionPointers = pointers;
				info.ClientPointers    = TRUE;

				success = CallMiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpWithDataSegs | MiniDumpWithProcessThreadData | MiniDumpWithThreadInfo, &info, 0, 0 );
				if( !success )
					dump_file[0] = 0;
			}

			FreeLibrary( lib );
		}
		if( success )
		{
			FlushFileBuffers( file );
		}

		CloseHandle( file );
	}
}

#  if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG

struct crash_exception_closure_t
{
	crash_dump_callback_fn      callback;
	const char*                 name;
};
typedef struct crash_exception_closure_t crash_exception_closure_t;

//TODO: Set per-thread
crash_exception_closure_t _crash_exception_closure;

LONG WINAPI _crash_exception_filter( LPEXCEPTION_POINTERS pointers )
{
	_crash_create_mini_dump( pointers, _crash_exception_closure.name, _crash_dump_file );
	if( _crash_exception_closure.callback )
		_crash_exception_closure.callback( _crash_dump_file );
	else
		log_errorf( 0, ERROR_EXCEPTION, "Exception occurred! Minidump written to: %ls", _crash_dump_file );
	return EXCEPTION_EXECUTE_HANDLER;
}

#  endif

#endif

#if FOUNDATION_PLATFORM_POSIX //&& !FOUNDATION_PLATFORM_APPLE

#include <signal.h>
#include <setjmp.h>
//#include <ucontext.h>

FOUNDATION_DECLARE_THREAD_LOCAL( crash_dump_callback_fn, crash_callback, 0 )
FOUNDATION_DECLARE_THREAD_LOCAL( const char*, crash_callback_name, 0 )

#if FOUNDATION_PLATFORM_ANDROID
#  define crash_env_t long int*
#elif FOUNDATION_PLATFORM_APPLE
#  define crash_env_t int*
#elif FOUNDATION_PLATFORM_BSD
#  define crash_env_t struct _sigjmp_buf*
#else
#  define crash_env_t struct __jmp_buf_tag*
#endif
FOUNDATION_DECLARE_THREAD_LOCAL( crash_env_t, crash_env, 0 )


static void _crash_guard_minidump( void* context, const char* name, char* dump_file )
{
	if( !name )
		name = environment_application()->short_name;
	string_format_buffer( dump_file, FOUNDATION_MAX_PATHLEN + 128, "%s/%s%s%s-%x.dmp",
		environment_temporary_directory(), name ? name : "", name ? "-" : "",
		string_from_uuid_static( environment_application()->instance ), time_system() );
	fs_make_directory( environment_temporary_directory() );

	//TODO: Write dump file
	//ucontext_t* user_context = context;
	FOUNDATION_UNUSED( context );
}


static void _crash_guard_sigaction( int sig, siginfo_t* info, void* arg )
{
	FOUNDATION_UNUSED( sig );
	FOUNDATION_UNUSED( info );
	FOUNDATION_UNUSED( arg );

	log_warnf( 0, WARNING_SUSPICIOUS, "Caught crash guard signal: %d", sig );

	crash_dump_callback_fn callback = get_thread_crash_callback();
	if( callback )
	{
		_crash_guard_minidump( arg, get_thread_crash_callback_name(), _crash_dump_file );
		callback( _crash_dump_file );
	}

	error_context_clear();

	crash_env_t guard_env = get_thread_crash_env();
	if( guard_env )
		siglongjmp( guard_env, FOUNDATION_CRASH_DUMP_GENERATED );
	else
		log_warn( 0, WARNING_SUSPICIOUS, "No sigjmp_buf for thread" );
}

#endif


int crash_guard( crash_guard_fn fn, void* data, crash_dump_callback_fn callback, const char* name )
{
	//Make sure path is initialized
	environment_temporary_directory();

#if FOUNDATION_PLATFORM_WINDOWS

#  if FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL// || FOUNDATION_COMPILER_CLANG
	__try
	{
		return fn( data );
	}
	__except( _crash_create_mini_dump( GetExceptionInformation(), name, _crash_dump_file ), EXCEPTION_EXECUTE_HANDLER )
	{
		if( callback )
			callback( _crash_dump_file );

		error_context_clear();

		return FOUNDATION_CRASH_DUMP_GENERATED;
	}
#  else
	SetUnhandledExceptionFilter( _crash_exception_filter );
	_crash_exception_closure.callback = callback;
	_crash_exception_closure.name = name;
	return fn( data );
#  endif

#elif FOUNDATION_PLATFORM_POSIX
	sigjmp_buf guard_env;

	struct sigaction action;
	memset( &action, 0, sizeof( action ) );

	//Signals we process globally
	action.sa_sigaction = _crash_guard_sigaction;
	action.sa_flags = SA_SIGINFO;
	if( ( sigaction( SIGTRAP, &action, 0 ) < 0 ) ||
	    ( sigaction( SIGABRT, &action, 0 ) < 0 ) ||
	    ( sigaction( SIGFPE,  &action, 0 ) < 0 ) ||
	    ( sigaction( SIGSEGV, &action, 0 ) < 0 ) ||
	    ( sigaction( SIGBUS,  &action, 0 ) < 0 ) ||
	    ( sigaction( SIGILL,  &action, 0 ) < 0 ) ||
	    ( sigaction( SIGSYS,  &action, 0 ) < 0 ) )
	{
		log_warn( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to set crash guard signal actions" );
		return fn( data );
	}

	set_thread_crash_callback( callback );
	set_thread_crash_callback_name( name );

	memset( &guard_env, 0, sizeof( guard_env ) );
	int ret = sigsetjmp( guard_env, 1 );
	if( ret == 0 )
	{
		set_thread_crash_env( guard_env );
		return fn( data );
	}
	return ret;

#else

	FOUNDATION_UNUSED( callback );
	FOUNDATION_UNUSED( name );

	//No guard mechanism in place yet for this platform
	return fn( data );

#endif
}


#if FOUNDATION_PLATFORM_WINDOWS

extern __declspec(dllimport) void STDCALL DebugBreak(void);

#endif


void crash_debug_break( void )
{
#if FOUNDATION_PLATFORM_WINDOWS
	DebugBreak();
#elif FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	__builtin_trap();
#else
	(*(volatile int*)3 = 0);
#endif
}


void crash_dump( void )
{
#if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	__builtin_trap();
#else
	(*(volatile int*)3 = 0);
#endif
}
