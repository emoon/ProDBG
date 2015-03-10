/* main.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#if FOUNDATION_PLATFORM_PNACL
#  include <foundation/pnacl.h>
#  include <ppapi/c/ppp_instance.h>
#endif


#if FOUNDATION_PLATFORM_WINDOWS

#  include <foundation/windows.h>


BOOL STDCALL _main_console_handler( DWORD control_type )
{
	const char* control_name = "UNKNOWN";
	bool post_terminate = false;
	bool handled = true;

	switch( control_type )
	{
		case CTRL_C_EVENT:         control_name = "CTRL_C"; post_terminate = true; break;
		case CTRL_BREAK_EVENT:     control_name = "CTRL_BREAK"; break;
		case CTRL_CLOSE_EVENT:     control_name = "CTRL_CLOSE"; post_terminate = true; break;
		case CTRL_LOGOFF_EVENT:    control_name = "CTRL_LOGOFF"; post_terminate = !config_bool( HASH_APPLICATION, HASH_DAEMON ); break;
		case CTRL_SHUTDOWN_EVENT:  control_name = "CTRL_SHUTDOWN"; post_terminate = true; break;
		default:                   handled = false; break;
	}
	log_infof( 0, "Caught console control: %s (%d)", control_name, control_type );
	if( post_terminate )
	{
		unsigned long level = 0, flags = 0;

		system_post_event( FOUNDATIONEVENT_TERMINATE );

		GetProcessShutdownParameters( &level, &flags );
		SetProcessShutdownParameters( level, SHUTDOWN_NORETRY );

		thread_sleep( 1000 );
	}
	return handled;
}


/*! Win32 UI application entry point, credits to Microsoft for ignoring yet another standard... */
#  if FOUNDATION_COMPILER_MSVC
FOUNDATION_API int APIENTRY WinMain( HINSTANCE, HINSTANCE, LPSTR, int );
#  endif

int STDCALL WinMain( HINSTANCE instance, HINSTANCE previnst, LPSTR cline, int cmd_show )
{
	int ret = -1;

	if( main_initialize() < 0 )
		return -1;

	SetConsoleCtrlHandler( _main_console_handler, TRUE );

	thread_set_main();

	foundation_startup();

	system_post_event( FOUNDATIONEVENT_START );

#if BUILD_DEBUG
	ret = main_run( 0 );
#else
	{
		char* name = 0;
		const application_t* app = environment_application();
		{
			const char* aname = app->short_name;
			name = string_clone( aname ? aname : "unknown" );
			name = string_append( name, "-" );
			name = string_append( name, string_from_version_static( app->version ) );
		}

		if( app->dump_callback )
			crash_guard_set( app->dump_callback, name );

		ret = crash_guard( main_run, 0, app->dump_callback, name );

		string_deallocate( name );
	}
#endif

	main_shutdown();

	return ret;
}


#elif FOUNDATION_PLATFORM_ANDROID
#include <foundation/android.h>
#endif

#if FOUNDATION_PLATFORM_POSIX

#include <foundation/posix.h>

#include <signal.h>
#include <stdio.h>

#if FOUNDATION_PLATFORM_APPLE
#include <foundation/main.h>
#include <foundation/delegate.h>
#endif


static void sighandler( int sig )
{
#if BUILD_ENABLE_LOG
	const char* signame = "UNKNOWN";
	switch( sig )
	{
		case SIGKILL: signame = "SIGKILL"; break;
		case SIGTERM: signame = "SIGTERM"; break;
		case SIGQUIT: signame = "SIGQUIT"; break;
		case SIGINT:  signame = "SIGINT"; break;
		default: break;
	}
	log_infof( 0, "Caught signal: %s (%d)", signame, sig );
#else
	FOUNDATION_UNUSED( sig );
#endif
	system_post_event( FOUNDATIONEVENT_TERMINATE );
}

#endif

#if FOUNDATION_PLATFORM_ANDROID
/*! Aliased entry point */
int real_main( void )
#elif FOUNDATION_PLATFORM_PNACL
/*! Aliased entry point */
int real_main( PP_Instance instance )
#else
/*! Normal entry point for all platforms, including Windows console applications */
int main( int argc, char** argv )
#endif
{
	int ret = -1;

#if !FOUNDATION_PLATFORM_ANDROID && !FOUNDATION_PLATFORM_PNACL
	_environment_main_args( argc, (const char* const*)argv );
#elif FOUNDATION_PLATFORM_PNACL
	FOUNDATION_UNUSED( instance );
#endif

	if( ( ret = main_initialize() ) < 0 )
		return ret;

#if FOUNDATION_PLATFORM_POSIX

	//Set signal handlers
	{
		struct sigaction action;
		memset( &action, 0, sizeof( action ) );

		//Signals we process globally
		action.sa_handler = sighandler;
		sigaction( SIGKILL, &action, 0 );
		sigaction( SIGTERM, &action, 0 );
		sigaction( SIGQUIT, &action, 0 );
		sigaction( SIGINT,  &action, 0 );

		//Ignore sigpipe
		action.sa_handler = SIG_IGN;
		sigaction( SIGPIPE, &action, 0 );
	}

#endif

#if FOUNDATION_PLATFORM_ANDROID
	if( ( ret = android_initialize() ) < 0 )
		return ret;
#endif

#if FOUNDATION_PLATFORM_WINDOWS

	SetConsoleCtrlHandler( _main_console_handler, TRUE );

#endif

	thread_set_main();

	foundation_startup();

#if FOUNDATION_PLATFORM_WINDOWS || FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_PNACL
	system_post_event( FOUNDATIONEVENT_START );
#endif

#if FOUNDATION_PLATFORM_APPLE
#  if FOUNDATION_PLATFORM_MACOSX
	if( !( environment_application()->flags & APPLICATION_UTILITY ) )
	{
		delegate_start_main_ns_thread();

		extern int NSApplicationMain( int argc, const char *argv[] );
		ret = NSApplicationMain( argc, (const char**)argv );

#  elif FOUNDATION_PLATFORM_IOS
	{
		delegate_start_main_ns_thread();

		extern int UIApplicationMain( int argc, char *argv[], void *principalClassName, void *delegateClassName );
		ret = UIApplicationMain( argc, (char**)argv, 0, 0 );

#  endif
		//NSApplicationMain and UIApplicationMain never returns though
		return ret;
	}
#endif

	{
		char* name = 0;
		const application_t* app = environment_application();
		{
			const char* aname = app->short_name;
			name = string_clone( aname ? aname : "unknown" );
			name = string_append( name, "-" );
			name = string_append( name, string_from_version_static( app->version ) );
		}

		if( app->dump_callback )
			crash_guard_set( app->dump_callback, name );

		ret = crash_guard( main_run, 0, app->dump_callback, name );

		string_deallocate( name );
	}
//#endif

	main_shutdown();

#if FOUNDATION_PLATFORM_ANDROID
	android_shutdown();
#endif

	return ret;
}


#if FOUNDATION_PLATFORM_ANDROID

/*! Android native glue entry point */
void android_main( struct android_app* app )
{
	if( !app )
		return;
	android_entry( app );
	real_main();
}

#endif


#if FOUNDATION_PLATFORM_PNACL

/*! PNaCl glue entry points */

PP_EXPORT int32_t PPP_InitializeModule( PP_Module module_id, PPB_GetInterface get_browser )
{
	return pnacl_module_initialize( module_id, get_browser );
}


PP_EXPORT const void* PPP_GetInterface( const char* interface_name )
{
	return pnacl_module_interface( interface_name );
}


PP_EXPORT void PPP_ShutdownModule()
{
	pnacl_module_shutdown();
}

#endif
