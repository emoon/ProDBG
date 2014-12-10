/* foundation.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


//Make artificial reference to main entry point
#if FOUNDATION_PLATFORM_ANDROID
struct android_app;
extern void android_main( struct android_app* );
#else
extern int main( int, char** );
#endif


static bool _foundation_initialized = false;


int foundation_initialize( const memory_system_t memory, const application_t application )
{
	if( _foundation_initialized )
		return 0;
	
	if( _atomic_initialize() < 0 )
		return -1;
	
	if( _memory_initialize( memory ) < 0 )
		return -1;

	_static_hash_initialize();
	
	if( _log_initialize() < 0 )
		return -1;

	if( _time_initialize() < 0 )
		return -1;

	if( _thread_initialize() < 0 )
		return -1;

	if( _random_initialize() < 0 )
		return -1;

	if( _fs_initialize() < 0 )
		return -1;

	if( _environment_initialize( application ) < 0 )
		return -1;

	if( _library_initialize() < 0 )
		return -1;

	if( _system_initialize() < 0 )
		return -1;

	if( _config_initialize() < 0 )
		return -1;

	//Parse built-in command line options
	{
		const char* const* cmdline = environment_command_line();
		unsigned int iarg, argsize;
		for( iarg = 0, argsize = array_size( cmdline ); iarg < argsize; ++iarg )
		{
			if( string_equal( cmdline[iarg], "--log-debug" ) )
				log_set_suppress( 0, ERRORLEVEL_NONE );
			else if( string_equal( cmdline[iarg], "--log-info" ) )
				log_set_suppress( 0, ERRORLEVEL_DEBUG );
		}

		config_parse_commandline( cmdline, array_size( cmdline ) );
	}

	//Artificial references
#if FOUNDATION_PLATFORM_ANDROID
	android_main( 0 );
#else
	if( (uintptr_t)main < 1 )
		return -1;
#endif

	_foundation_initialized = true;
	
	return 0;
}


void foundation_startup( void )
{
	_memory_preallocate();
}


void foundation_shutdown( void )
{
	_foundation_initialized = false;

	profile_shutdown();
	
	_config_shutdown();
	_fs_shutdown();
	_system_shutdown();
	_library_shutdown();
	_environment_shutdown();
	_random_shutdown();
	_thread_shutdown();
	_time_shutdown();
	_log_shutdown();
	_static_hash_shutdown();
	_memory_shutdown();
	_atomic_shutdown();
}


bool foundation_is_initialized( void )
{
	return _foundation_initialized;
}


version_t foundation_version( void )
{
	return version_make( 1, 1, 0, 0, 0 );
}

