/* process.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <sys/time.h>
#elif FOUNDATION_PLATFORM_PNACL
#  include <unistd.h>
#endif

#if FOUNDATION_PLATFORM_MACOSX
#  include <foundation/apple.h>
#  include <sys/event.h>
#endif


static int _process_exit_code;


process_t* process_allocate()
{
	process_t* proc = memory_allocate( 0, sizeof( process_t ), 0, MEMORY_PERSISTENT );

	process_initialize( proc );

	return proc;
}


void process_initialize( process_t* proc )
{
	memset( proc, 0, sizeof( process_t ) );
	proc->flags = PROCESS_ATTACHED;
}


void process_deallocate( process_t* proc )
{
	if( !proc )
		return;
	process_finalize( proc );
	memory_deallocate( proc );
}


void process_finalize( process_t* proc )
{
	if( !( proc->flags & PROCESS_DETACHED ) )
		process_wait( proc );

	stream_deallocate( proc->pipeout );
	stream_deallocate( proc->pipein );
	string_deallocate( proc->wd );
	string_deallocate( proc->path );
	string_array_deallocate( proc->args );
#if FOUNDATION_PLATFORM_WINDOWS
	string_deallocate( proc->verb );
#endif

	proc->pipeout = 0;
	proc->pipein = 0;
	proc->wd = 0;
	proc->path = 0;
#if FOUNDATION_PLATFORM_WINDOWS
	proc->verb = 0;
#endif
}


void process_set_working_directory( process_t* proc, const char* path )
{
	if( !proc )
		return;
	string_deallocate( proc->wd );
	proc->wd = string_clone( path );
}


void process_set_executable_path( process_t* proc, const char* path )
{
	if( !proc )
		return;
	string_deallocate( proc->path );
	proc->path = string_clone( path );
}


void process_set_arguments( process_t* proc, const char** args, unsigned int num )
{
	unsigned int ia;
	if( !proc )
		return;
	string_array_deallocate( proc->args );
	for( ia = 0; ia < num; ++ia )
		array_push( proc->args, string_clone( args[ia] ) );
}


void process_set_flags( process_t* proc, unsigned int flags )
{
	if( !proc )
		return;
	proc->flags = flags;
}


void process_set_verb( process_t* proc, const char* verb )
{
	if( !proc )
		return;
#if FOUNDATION_PLATFORM_WINDOWS
	string_deallocate( proc->verb );
	proc->verb = string_clone( verb );
#else
	FOUNDATION_UNUSED( verb );
#endif
}


int process_spawn( process_t* proc )
{
	static char unescaped[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.:/\\";
	int i, num_args;
	int size;
#if FOUNDATION_PLATFORM_WINDOWS
	wchar_t* wcmdline;
	wchar_t* wwd;
	char* cmdline = 0;
#endif

	if( !proc )
		return PROCESS_INVALID_ARGS;

	proc->code = PROCESS_INVALID_ARGS;

	if( !string_length( proc->path ) )
		return proc->code;

	//Always escape path on Windows platforms
#if FOUNDATION_PLATFORM_POSIX
	if( string_find_first_not_of( proc->path, unescaped, 0 ) != STRING_NPOS )
#endif
	{
		if( proc->path[0] != '"' )
			proc->path = string_prepend( proc->path, "\"" );
		if( proc->path[ string_length( proc->path ) - 1 ] != '"' )
			proc->path = string_append( proc->path, "\"" );
	}

	size = array_size( proc->args );
	for( i = 0, num_args = 0; i < size; ++i )
	{
		char* arg = proc->args[i];

		if( !string_length( arg ) )
			continue;

		++num_args;

#if !FOUNDATION_PLATFORM_POSIX
		if( string_find_first_not_of( arg, unescaped, 0 ) != STRING_NPOS )
		{
			if( arg[0] != '"' )
			{
				//Check if we need to escape " characters
				unsigned int pos = string_find( arg, '"', 0 );
				while( pos != STRING_NPOS )
				{
					if( arg[ pos - 1 ] != '\\' )
					{
						char* escarg = string_substr( arg, 0, pos );
						char* left = string_substr( arg, pos, STRING_NPOS );
						escarg = string_append( escarg, "\\" );
						escarg = string_append( escarg, left );
						string_deallocate( left );
						string_deallocate( arg );
						arg = escarg;
					}
					pos = string_find( arg, '"', pos + 2 );
				}
				arg = string_prepend( arg, "\"" );
				arg = string_append( arg, "\"" );

				proc->args[i] = arg;
			}
		}
#endif
	}

#if FOUNDATION_PLATFORM_WINDOWS

#  ifndef SEE_MASK_NOASYNC
#    define SEE_MASK_NOASYNC           0x00000100
#  endif

	if( !( proc->flags & PROCESS_WINDOWS_USE_SHELLEXECUTE ) ) //Don't prepend exe path to parameters if using ShellExecute
		cmdline = string_clone( proc->path );

	//Build command line string
	for( i = 0; i < size; ++i )
	{
		char* arg = proc->args[i];

		if( !string_length( arg ) )
			continue;

		if( cmdline )
			cmdline = string_append( cmdline, " " );
		cmdline = string_append( cmdline, arg );
	}

	if( !string_length( proc->wd ) )
		proc->wd = string_clone( environment_current_working_directory() );

	wcmdline = wstring_allocate_from_string( cmdline, 0 );
	wwd = wstring_allocate_from_string( proc->wd, 0 );

	if( proc->flags & PROCESS_WINDOWS_USE_SHELLEXECUTE )
	{
		SHELLEXECUTEINFOW sei;
		wchar_t* wverb;
		wchar_t* wpath;

		wverb = ( proc->verb && string_length( proc->verb ) ) ? wstring_allocate_from_string( proc->verb, 0 ) : 0;
		wpath = wstring_allocate_from_string( proc->path, 0 );

		ZeroMemory( &sei, sizeof( sei ) );

		sei.cbSize          = sizeof(SHELLEXECUTEINFOW);
		sei.hwnd            = 0;
		sei.fMask           = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
		sei.lpVerb          = wverb;
		sei.lpFile          = wpath;
		sei.lpParameters    = wcmdline;
		sei.lpDirectory     = wwd;
		sei.nShow           = SW_SHOWNORMAL;

		if( !( proc->flags & PROCESS_CONSOLE ) )
			sei.fMask      |= SEE_MASK_NO_CONSOLE;

		if( proc->flags & PROCESS_STDSTREAMS )
			log_warn( 0, WARNING_UNSUPPORTED, "Unable to redirect standard in/out through pipes when using ShellExecute for process spawning" );

		log_debugf( 0, "Spawn process (ShellExecute): %s %s", proc->path, cmdline );

		if( !ShellExecuteExW( &sei ) )
		{
			log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to spawn process (ShellExecute) for executable '%s': %s", proc->path, system_error_message( GetLastError() ) );
		}
		else
		{
			proc->hp   = sei.hProcess;
			proc->ht   = 0;
			proc->code = 0;
		}

		wstring_deallocate( wverb );
		wstring_deallocate( wpath );
	}
	else
	{
		STARTUPINFOW si;
		PROCESS_INFORMATION pi;
		BOOL inherit_handles = FALSE;

		memset( &si, 0, sizeof( si ) );
		memset( &pi, 0, sizeof( pi ) );
		si.cb = sizeof( si );

		if( proc->flags & PROCESS_STDSTREAMS )
		{
			proc->pipeout = pipe_allocate();
			proc->pipein = pipe_allocate();

			si.dwFlags |= STARTF_USESTDHANDLES;
			si.hStdOutput = pipe_write_handle( proc->pipeout );
			si.hStdInput = pipe_read_handle( proc->pipein );
			si.hStdError = GetStdHandle( STD_ERROR_HANDLE );

			//Don't inherit wrong ends of pipes
			SetHandleInformation( pipe_read_handle( proc->pipeout ), HANDLE_FLAG_INHERIT, 0 );
			SetHandleInformation( pipe_write_handle( proc->pipein ), HANDLE_FLAG_INHERIT, 0 );

			inherit_handles = TRUE;
		}

		log_debugf( 0, "Spawn process (CreateProcess): %s %s", proc->path, cmdline );

		if( !CreateProcessW( 0, wcmdline, 0, 0, inherit_handles, ( proc->flags & PROCESS_CONSOLE ) ? CREATE_NEW_CONSOLE : 0, 0, wwd, &si, &pi ) )
		{
			log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to spawn process (CreateProcess) for executable '%s': %s", proc->path, system_error_message( GetLastError() ) );

			stream_deallocate( proc->pipeout );
			stream_deallocate( proc->pipein );

			proc->pipeout = 0;
			proc->pipein = 0;
		}
		else
		{
			proc->hp = pi.hProcess;
			proc->ht = pi.hThread;
			proc->code = 0;
		}

		if( proc->pipeout )
			pipe_close_write( proc->pipeout );
		if( proc->pipein )
			pipe_close_read( proc->pipein );
	}

	wstring_deallocate( wcmdline );
	wstring_deallocate( wwd );
	string_deallocate( cmdline );

	if( proc->code < 0 )
		return proc->code; //Error

#endif

#if FOUNDATION_PLATFORM_MACOSX

	if( proc->flags & PROCESS_MACOSX_USE_OPENAPPLICATION )
	{
		proc->pid = 0;

		LSApplicationParameters params;
		ProcessSerialNumber psn;
		FSRef* fsref = memory_allocate( 0, sizeof( FSRef ), 0, MEMORY_TEMPORARY | MEMORY_ZERO_INITIALIZED );

		memset( &params, 0, sizeof( LSApplicationParameters ) );
		memset( &psn, 0, sizeof( ProcessSerialNumber ) );

		char* pathstripped = string_strip( string_clone( proc->path ), "\"" );

		OSStatus status = 0;
		status = FSPathMakeRef( (uint8_t*)pathstripped, fsref, 0 );
		if( status < 0 )
		{
			pathstripped = string_append( pathstripped, ".app" );
			status = FSPathMakeRef( (uint8_t*)pathstripped, fsref, 0 );
		}

		CFStringRef* args = 0;
		for( i = 0, size = array_size( proc->args ); i < size; ++i ) //App gets executable path automatically, don't include
			array_push( args, CFStringCreateWithCString( 0, proc->args[i], kCFStringEncodingUTF8 ) );

		CFArrayRef argvref = CFArrayCreate( 0, (const void**)args, (CFIndex)array_size( args ), 0 );

		params.flags = kLSLaunchDefaults;
		params.application = fsref;
		params.argv = argvref;

		log_debugf( 0, "Spawn process (LSOpenApplication): %s", pathstripped );

		status = LSOpenApplication( &params, &psn );
		if( status != 0 )
		{
			proc->code = status;
			log_warnf( 0, WARNING_BAD_DATA, "Unable to spawn process for executable '%s': %s", proc->path, system_error_message( status ) );
		}

		CFRelease( argvref );
		for( i = 0, size = array_size( args ); i < size; ++i )
			CFRelease( args[i] );
		array_deallocate( args );

		memory_deallocate( fsref );
		string_deallocate( pathstripped );

		if( status == 0 )
		{
			pid_t pid = 0;
			GetProcessPID( &psn, &pid );

			proc->pid = pid;

			//Always "detached" with LSOpenApplication, not a child process at all
			//Setup a kqueue to watch when process terminates so we can emulate a wait
			proc->kq = kqueue();
			if( proc->kq < 0 )
			{
				log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to create kqueue for process watch: %s (%d)", proc->kq, system_error_message( proc->kq ) );
				proc->kq = 0;
			}
			else
			{
				struct kevent changes;
				EV_SET( &changes, (pid_t)pid, EVFILT_PROC, EV_ADD | EV_RECEIPT, NOTE_EXIT, 0, 0 );
				int ret = kevent( proc->kq, &changes, 1, &changes, 1, 0 );
				if( ret != 1 )
				{
					log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to setup kqueue for process watch, failed to add event to kqueue (%d)", ret );
					close( proc->kq );
					proc->kq = 0;
				}
			}
		}

		goto exit;
	}
#endif

#if FOUNDATION_PLATFORM_POSIX

	//Insert executable arg at start and null ptr at end
	int argc = array_size( proc->args ) + 1;
	array_grow( proc->args, 2 );
	for( int arg = argc - 1; arg > 0; --arg )
		proc->args[arg] = proc->args[arg-1];
	proc->args[0] = string_clone( proc->path );
	proc->args[argc] = 0;

	if( proc->flags & PROCESS_STDSTREAMS )
	{
		proc->pipeout = pipe_allocate();
		proc->pipein = pipe_allocate();
	}

	proc->pid = 0;
	pid_t pid = fork();

	if( pid == 0 )
	{
		//Child
		if( string_length( proc->wd ) )
		{
			log_debugf( 0, "Spawned child process, setting working directory to %s", proc->wd );
			environment_set_current_working_directory( proc->wd );
		}

		log_debugf( 0, "Child process executing: %s", proc->path );

		if( proc->flags & PROCESS_STDSTREAMS )
		{
			pipe_close_read( proc->pipeout );
			dup2( pipe_write_fd( proc->pipeout ), STDOUT_FILENO );

			pipe_close_write( proc->pipein );
			dup2( pipe_read_fd( proc->pipein ), STDIN_FILENO );
		}

		int code = execv( proc->path, proc->args );
		if( code < 0 ) //Will always be true since this point will never be reached if execve() is successful
			log_warnf( 0, WARNING_BAD_DATA, "Child process failed execve() : %s : %s", proc->path, system_error_message( errno ) );

		//Error
		process_exit( -1 );
	}

	if( pid > 0 )
	{
		log_debugf( 0, "Child process forked, pid %d", pid );

		proc->pid = pid;

		if( proc->pipeout )
			pipe_close_write( proc->pipeout );
		if( proc->pipein )
			pipe_close_read( proc->pipein );

		/*if( proc->flags & PROCESS_DETACHED )
		{
			int cstatus = 0;
			pid_t err = waitpid( pid, &cstatus, WNOHANG );
			if( err == 0 )
			{
				//TODO: Ugly wait to make sure process spawned correctly
				thread_sleep( 500 );
				err = waitpid( pid, &cstatus, WNOHANG );
			}
			if( err > 0 )
			{
				//Process exited, check code
				proc->pid = 0;
				proc->code = (int)((char)WEXITSTATUS( cstatus ));
				log_debugf( 0, "Child process returned: %d", proc->code );
				return proc->code;
			}
		}*/
	}
	else
	{
		//Error
		proc->code = errno;
		log_warnf( 0, WARNING_BAD_DATA, "Unable to spawn process: %s : %s", proc->path, system_error_message( proc->code ) );

		if( proc->pipeout )
			stream_deallocate( proc->pipeout );
		if( proc->pipein )
			stream_deallocate( proc->pipein );

		proc->pipeout = 0;
		proc->pipein = 0;

		return proc->code;
	}

#endif

#if !FOUNDATION_PLATFORM_WINDOWS && !FOUNDATION_PLATFORM_POSIX
	FOUNDATION_ASSERT_FAIL( "Process spawning not supported on platform" );
#endif

#if FOUNDATION_PLATFORM_MACOSX
exit:
#endif

	if( proc->flags & PROCESS_DETACHED )
		return PROCESS_STILL_ACTIVE;

	return process_wait( proc );
}


stream_t* process_stdout( process_t* proc )
{
	return proc ? proc->pipeout : 0;
}


stream_t* process_stdin( process_t* proc )
{
	return proc ? proc->pipein : 0;
}


int process_wait( process_t* proc )
{
#if FOUNDATION_PLATFORM_POSIX
	int cstatus;
	pid_t err;
#endif

	if( !proc )
		return PROCESS_INVALID_ARGS;

#if FOUNDATION_PLATFORM_WINDOWS

	if( !proc->hp )
		return proc->code;

	while( GetExitCodeProcess( proc->hp, (LPDWORD)&proc->code ) )
	{
		if( ( proc->code != STILL_ACTIVE ) || ( proc->flags & PROCESS_DETACHED ) )
			break;
		thread_sleep( 50 );
		proc->code = -1;
	}

	if( ( proc->code == STILL_ACTIVE ) && ( proc->flags & PROCESS_DETACHED ) )
		return PROCESS_STILL_ACTIVE;

	if( proc->ht )
		CloseHandle( proc->ht );
	if( proc->hp )
		CloseHandle( proc->hp );

	proc->hp = 0;
	proc->ht = 0;

#elif FOUNDATION_PLATFORM_POSIX

	if( !proc->pid )
		return proc->code;

#  if FOUNDATION_PLATFORM_MACOSX
	if( proc->flags & PROCESS_MACOSX_USE_OPENAPPLICATION )
	{
		if( proc->kq )
		{
			struct kevent event;
			int ret = kevent( proc->kq, 0, 0, &event, 1, 0 );
			if( ret != 1 )
				log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to wait on process, failed to read event from kqueue (%d)", ret );

			close( proc->kq );
			proc->kq = 0;
		}
		else
		{
			log_warn( 0, WARNING_BAD_DATA, "Unable to wait on a process started with PROCESS_MACOSX_USE_OPENAPPLICATION and no kqueue" );
			return PROCESS_WAIT_FAILED;
		}
		proc->pid = 0;
		proc->code = 0;
		return proc->code;
	}
#  endif

	cstatus = 0;
	err = waitpid( proc->pid, &cstatus, ( proc->flags & PROCESS_DETACHED ) ? WNOHANG : 0 );
	if( err > 0 )
	{
		if( WIFEXITED( cstatus ) )
			proc->code = (int)((char)WEXITSTATUS( cstatus ));
		else if( WIFSIGNALED( cstatus ) )
		{
			proc->code = PROCESS_TERMINATED_SIGNAL;
#ifdef WCOREDUMP
			//if( WCOREDUMP( cstatus ) )
			//...
#endif
			//proc->signal = WTERMSIG( cstatus );
		}
		proc->pid = 0;
	}
	else
	{
		int cur_errno = errno;
		if( ( err == 0 ) && ( proc->flags & PROCESS_DETACHED ) )
			return PROCESS_STILL_ACTIVE;
		if( ( err < 0 ) && ( cur_errno == EINTR ) )
			return PROCESS_WAIT_INTERRUPTED;
		log_warnf( 0, WARNING_BAD_DATA, "waitpid(%d) failed: %s (%d) (returned %d)", proc->pid, system_error_message( cur_errno ), cur_errno, err );
		return PROCESS_WAIT_FAILED;
	}

#elif FOUNDATION_PLATFORM_PNACL
	//Not supported
#else
#error Not implemented
#endif

	return proc->code;
}


int process_exit_code( void )
{
	return _process_exit_code;
}


void process_set_exit_code( int code )
{
	_process_exit_code = code;
}


void FOUNDATION_ATTRIBUTE( noreturn ) process_exit( int code )
{
	_exit( code );
}

