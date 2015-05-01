/* environment.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


static char    _environment_executable_name[FOUNDATION_MAX_PATHLEN];
static char    _environment_executable_dir[FOUNDATION_MAX_PATHLEN];
static char    _environment_executable_path[FOUNDATION_MAX_PATHLEN];
static char    _environment_initial_working_dir[FOUNDATION_MAX_PATHLEN];
static char    _environment_current_working_dir[FOUNDATION_MAX_PATHLEN];
static char    _environment_home_dir[FOUNDATION_MAX_PATHLEN];
static char    _environment_temp_dir[FOUNDATION_MAX_PATHLEN];
static bool    _environment_temp_dir_local;
#if FOUNDATION_PLATFORM_WINDOWS
static char*   _environment_var;
#  include <foundation/windows.h>
#elif FOUNDATION_PLATFORM_POSIX
#  include <foundation/posix.h>
#endif

#if FOUNDATION_PLATFORM_ANDROID
#  include <foundation/android.h>
#endif

#if FOUNDATION_PLATFORM_APPLE
#  include <foundation/apple.h>
extern void _environment_ns_command_line( char*** argv );
extern void _environment_ns_home_directory( char* );
extern void _environment_ns_temporary_directory( char* );
#endif

#if FOUNDATION_PLATFORM_BSD
#  include <sys/types.h>
#  include <sys/sysctl.h>
#endif

static application_t       _environment_app;

static char**              _environment_argv;
static int                 _environment_main_argc;
static const char* const*  _environment_main_argv;

static void _environment_clean_temporary_directory( bool recreate );


void _environment_main_args( int argc, const char* const* argv )
{
	_environment_main_argc = argc;
	_environment_main_argv = argv;
}

#if !FOUNDATION_PLATFORM_PNACL

static void _environment_set_executable_paths( const char* executable_path )
{
	unsigned int last_path = string_rfind( executable_path, '/', STRING_NPOS );
	if( last_path != STRING_NPOS )
	{
		if( !string_length( _environment_executable_dir ) )
			string_copy( _environment_executable_dir, executable_path, last_path + 1 );
		if( !string_length( _environment_executable_name ) )
		{
			string_copy( _environment_executable_name, executable_path + last_path + 1, FOUNDATION_MAX_PATHLEN );
		}
	}
	else
	{
		if( !string_length( _environment_executable_dir ) )
			_environment_executable_dir[0] = 0;
		if( !string_length( _environment_executable_name ) )
			string_copy( _environment_executable_name, executable_path, FOUNDATION_MAX_PATHLEN );
	}
#if FOUNDATION_PLATFORM_WINDOWS
	last_path = string_length( _environment_executable_name );
	if( ( last_path > 4 ) && ( string_equal( _environment_executable_name + ( last_path - 4 ), ".exe" ) || string_equal( _environment_executable_name + ( last_path - 4 ), ".EXE" ) ) )
		_environment_executable_name[ last_path - 4 ] = 0;
#endif
	string_copy( _environment_executable_path, executable_path, FOUNDATION_MAX_PATHLEN );
}

#endif

int _environment_initialize( const application_t application )
{
#if FOUNDATION_PLATFORM_WINDOWS
	int ia;
	int num_args = 0;
	DWORD ret = 0;
	wchar_t module_filename[FOUNDATION_MAX_PATHLEN];
	LPWSTR* arg_list = CommandLineToArgvW( GetCommandLineW(), &num_args );
	if( !arg_list )
		return -1;

	for( ia = 0; ia < num_args; ++ia )
		array_push( _environment_argv, string_allocate_from_wstring( arg_list[ia], 0 ) );

	LocalFree( arg_list );

	if( GetModuleFileNameW( 0, module_filename, FOUNDATION_MAX_PATHLEN ) )
	{
		char* exe_path = string_allocate_from_wstring( module_filename, 0 );
		char* dir_path = path_make_absolute( exe_path );

		_environment_set_executable_paths( dir_path );

		string_deallocate( dir_path );
		string_deallocate( exe_path );
	}
	else
	{
		log_errorf( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to get module filename" );
		return -1;
	}

#elif FOUNDATION_PLATFORM_MACOSX || FOUNDATION_PLATFORM_IOS

	_environment_ns_command_line( &_environment_argv );

	char* exe_path = path_make_absolute( _environment_argv[0] );
	_environment_set_executable_paths( exe_path );
	string_deallocate( exe_path );

#elif FOUNDATION_PLATFORM_ANDROID

	stream_t* cmdline = fs_open_file( "/proc/self/cmdline", STREAM_IN | STREAM_BINARY );
	if( !cmdline )
	{
		log_errorf( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to read /proc/self/cmdline" );
		return -1;
	}

	while( true )
	{
		char* arg = stream_read_string( cmdline );
		if( !string_length( arg ) )
		{
			string_deallocate( arg );
			break;
		}

		array_push( _environment_argv, arg );
	}

	char* exe_path = path_append( path_directory_name( android_app()->activity->internalDataPath ), "lib" );

	// This will return something like "app_process" since we're just a dynamic
	// library that gets invoked by a launcher process
	char exelink[FOUNDATION_MAX_PATHLEN] = {0};
	if( readlink( "/proc/self/exe", exelink, FOUNDATION_MAX_PATHLEN ) < 0 )
	{
		log_errorf( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to read /proc/self/exe link" );
		return -1;
	}

	char* exe_name = path_file_name( exelink );
	exe_path = path_append( exe_path, exe_name );

	_environment_set_executable_paths( exe_path );

	string_deallocate( exe_path );
	string_deallocate( exe_name );

#elif FOUNDATION_PLATFORM_BSD

	for( int ia = 0; ia < _environment_main_argc; ++ia )
		array_push( _environment_argv, string_clone( _environment_main_argv[ia] ) );

	int callarg[4];
	char buf[1024];
	size_t size = sizeof(buf);
	callarg[0] = CTL_KERN;
	callarg[1] = KERN_PROC;
	callarg[2] = KERN_PROC_PATHNAME;
	callarg[3] = -1;
	sysctl(callarg, 4, buf, &size, 0, 0);

	char* exe_path;
	char* dir_path;

	exe_path = path_clean( string_clone( buf ), path_is_absolute( buf ) );
	dir_path = path_make_absolute( exe_path );

	_environment_set_executable_paths( dir_path );

	string_deallocate( dir_path );
	string_deallocate( exe_path );

#elif FOUNDATION_PLATFORM_POSIX

	stream_t* cmdline = fs_open_file( "/proc/self/cmdline", STREAM_IN | STREAM_BINARY );
	if( !cmdline )
	{
		log_error( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to read /proc/self/cmdline" );
		return -1;
	}

	while( true )
	{
		char* arg = stream_read_string( cmdline );
		if( !string_length( arg ) )
		{
			string_deallocate( arg );
			break;
		}

		array_push( _environment_argv, arg );
	}

	char exelink[FOUNDATION_MAX_PATHLEN] = {0};
	if( readlink( "/proc/self/exe", exelink, FOUNDATION_MAX_PATHLEN ) < 0 )
	{
		log_error( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to read /proc/self/exe link" );
		return -1;
	}

	char* exe_path;
	char* dir_path;

	exe_path = path_clean( string_clone( exelink ), path_is_absolute( exelink ) );
	dir_path = path_make_absolute( exe_path );

	_environment_set_executable_paths( dir_path );

	string_deallocate( dir_path );
	string_deallocate( exe_path );

#elif FOUNDATION_PLATFORM_PNACL

	for( int ia = 0; ia < _environment_main_argc; ++ia )
		array_push( _environment_argv, string_clone( _environment_main_argv[ia] ) );

	string_copy( _environment_executable_dir, "/cache", FOUNDATION_MAX_PATHLEN );
	string_copy( _environment_current_working_dir, "/cache", FOUNDATION_MAX_PATHLEN );
	string_copy( _environment_home_dir, "/persistent", FOUNDATION_MAX_PATHLEN );
	string_copy( _environment_temp_dir, "/tmp", FOUNDATION_MAX_PATHLEN );
	string_copy( _environment_executable_path, application.short_name, FOUNDATION_MAX_PATHLEN );

#else
#  error Not implemented
	/*if( array_size( _environment_argv ) > 0 )
	{
		char* exe_path = path_clean( string_clone( _environment_argv[0] ), path_is_absolute( _environment_argv[0] ) );
		char* dir_path = path_make_absolute( exe_path );

		_environment_set_executable_paths( dir_path );

		string_deallocate( dir_path );
		string_deallocate( exe_path );
	}
	else if( !string_length( _environment_executable_dir ) )
	   	string_copy( _environment_executable_dir, environment_current_working_directory(), FOUNDATION_MAX_PATHLEN ); */
#endif

   	_environment_app = application;

	if( uuid_is_null( _environment_app.instance ) )
		_environment_app.instance = uuid_generate_random();

   	string_copy( _environment_initial_working_dir, environment_current_working_directory(), FOUNDATION_MAX_PATHLEN );

   	_environment_clean_temporary_directory( true );

	return 0;
}


void _environment_shutdown( void )
{
	_environment_clean_temporary_directory( false );

	string_array_deallocate( _environment_argv );

#if FOUNDATION_PLATFORM_WINDOWS
	string_deallocate( _environment_var );
#endif
}


const char* const* environment_command_line( void )
{
	return (const char* const*)_environment_argv;
}


const char* environment_executable_name( void )
{
	return _environment_executable_name;
}


const char* environment_executable_directory( void )
{
	return _environment_executable_dir;
}


const char* environment_executable_path( void )
{
	return _environment_executable_path;
}


const char* environment_initial_working_directory( void )
{
	return _environment_initial_working_dir;
}


const char* environment_current_working_directory( void )
{
	if( _environment_current_working_dir[0] )
		return _environment_current_working_dir;
#if FOUNDATION_PLATFORM_WINDOWS
	{
		char* path;
		wchar_t* wd = memory_allocate( 0, sizeof( wchar_t ) * FOUNDATION_MAX_PATHLEN, 0, MEMORY_TEMPORARY | MEMORY_ZERO_INITIALIZED );
		GetCurrentDirectoryW( FOUNDATION_MAX_PATHLEN-1, wd );
		path = path_clean( string_allocate_from_wstring( wd, 0 ), true );
		string_copy( _environment_current_working_dir, path, FOUNDATION_MAX_PATHLEN );
		string_copy( _environment_current_working_dir, path, FOUNDATION_MAX_PATHLEN );
		string_deallocate( path );
		memory_deallocate( wd );
	}
#elif FOUNDATION_PLATFORM_POSIX
	char* path = memory_allocate( 0, FOUNDATION_MAX_PATHLEN, 0, MEMORY_TEMPORARY | MEMORY_ZERO_INITIALIZED );
	if( !getcwd( path, FOUNDATION_MAX_PATHLEN ) )
	{
		log_errorf( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to get cwd: %s", system_error_message( 0 ) );
		return "";
	}
	path = path_clean( path, true );
	string_copy( _environment_current_working_dir, path, FOUNDATION_MAX_PATHLEN );
	memory_deallocate( path );
#elif FOUNDATION_PLATFORM_PNACL
	return "/persistent";
#else
#  error Not implemented
#endif
	return _environment_current_working_dir;
}


void environment_set_current_working_directory( const char* path )
{
	if( !path )
		return;
	log_debugf( 0, "Setting current working directory to: %s", path );
#if FOUNDATION_PLATFORM_WINDOWS
	{
		wchar_t* wpath = wstring_allocate_from_string( path, 0 );
		if( !SetCurrentDirectoryW( wpath ) )
			log_warnf( 0, WARNING_SUSPICIOUS, "Unable to set working directory: %ls", wpath );
		wstring_deallocate( wpath );
	}
#elif FOUNDATION_PLATFORM_POSIX
	if( chdir( path ) < 0 )
		log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to set working directory: %s", path );
#elif FOUNDATION_PLATFORM_PNACL
	//Allow anything
	char* abspath = path_make_absolute( path );
	string_copy( _environment_current_working_dir, abspath, FOUNDATION_MAX_PATHLEN );
	string_deallocate( abspath );
	return;
#else
#  error Not implemented
#endif
	_environment_current_working_dir[0] = 0;
}


const char* environment_home_directory( void )
{
	if( _environment_home_dir[0] )
		return _environment_home_dir;
#if FOUNDATION_PLATFORM_WINDOWS
	{
		char* path;
		wchar_t* wpath = memory_allocate( 0, sizeof( wchar_t ) * FOUNDATION_MAX_PATHLEN, 0, MEMORY_TEMPORARY | MEMORY_ZERO_INITIALIZED );
		SHGetFolderPathW( 0, CSIDL_LOCAL_APPDATA, 0, 0, wpath );
		path = path_clean( string_allocate_from_wstring( wpath, 0 ), true );
		string_copy( _environment_home_dir, path, FOUNDATION_MAX_PATHLEN );
		string_deallocate( path );
		memory_deallocate( wpath );
	}
#elif FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_BSD || FOUNDATION_PLATFORM_TIZEN
	const char* env_home = environment_variable( "HOME" );
	if( !env_home )
	{
		struct passwd* pw = getpwuid( getuid() );
		env_home = pw->pw_dir;
	}
	if( env_home )
		string_copy( _environment_home_dir, env_home, FOUNDATION_MAX_PATHLEN );
#elif FOUNDATION_PLATFORM_IOS
	_environment_ns_home_directory( _environment_home_dir );
#elif FOUNDATION_PLATFORM_MACOSX
	if( environment_application()->flags & APPLICATION_UTILITY )
	{
		_environment_ns_home_directory( _environment_home_dir );
	}
	else
	{
		char bundle_identifier[FOUNDATION_MAX_PATHLEN+1];
		environment_bundle_identifier( bundle_identifier, FOUNDATION_MAX_PATHLEN );

		char* path = path_append( path_merge( _environment_home_dir, "/Library/Application Support" ), bundle_identifier );
		string_copy( _environment_home_dir, path, FOUNDATION_MAX_PATHLEN );
		string_deallocate( path );
	}
#elif FOUNDATION_PLATFORM_ANDROID
	string_copy( _environment_home_dir, android_app()->activity->internalDataPath, FOUNDATION_MAX_PATHLEN );
#elif FOUNDATION_PLATFORM_PNACL
#else
#  error Not implemented
#endif
	return _environment_home_dir;
}


const char* environment_temporary_directory( void )
{
	if( _environment_temp_dir[0] )
		return _environment_temp_dir;
#if FOUNDATION_PLATFORM_WINDOWS
	{
		char* path;
		wchar_t* wpath = memory_allocate( 0, sizeof( wchar_t ) * FOUNDATION_MAX_PATHLEN, 0, MEMORY_TEMPORARY | MEMORY_ZERO_INITIALIZED );
		GetTempPathW( FOUNDATION_MAX_PATHLEN, wpath );
		path = path_clean( string_allocate_from_wstring( wpath, 0 ), true );
		string_copy( _environment_temp_dir, path, FOUNDATION_MAX_PATHLEN );
		string_deallocate( path );
		memory_deallocate( wpath );
	}
#endif
#if FOUNDATION_PLATFORM_ANDROID
	//Use application internal data path, or if that fails, external data path
	struct android_app* app = android_app();
	const char* test_path[] = { app && app->activity ? app->activity->internalDataPath : 0, app && app->activity ? app->activity->externalDataPath : 0 };
	for( int itest = 0; !_environment_temp_dir[0] && ( itest < 2 ); ++itest )
	{
		if( test_path[itest] && string_length( test_path[itest] ) )
		{
			fs_make_directory( test_path[itest] );

			char* temp_path = path_prepend( string_format( ".tmp-%s", string_from_uuid_static( uuid_generate_random() ) ), test_path[itest] );
			stream_t* temp_stream = fs_open_file( temp_path, STREAM_CREATE | STREAM_OUT | STREAM_BINARY );
			if( temp_stream )
			{
				stream_deallocate( temp_stream );

				string_copy( _environment_temp_dir, test_path[itest], FOUNDATION_MAX_PATHLEN );
				unsigned int len = string_length( _environment_temp_dir );
				if( !len || ( _environment_temp_dir[ len - 1 ] != '/' ) )
				{
					_environment_temp_dir[ len++ ] = '/';
					_environment_temp_dir[ len ] = 0;
				}
				string_copy( _environment_temp_dir + len, ".tmp", FOUNDATION_MAX_PATHLEN - len );

				_environment_temp_dir_local = true;
			}
			else
			{
				_environment_temp_dir[0] = 0;
			}
			string_deallocate( temp_path );
		}
	}
#endif
#if FOUNDATION_PLATFORM_APPLE
	if( !_environment_temp_dir[0] )
	{
		_environment_ns_temporary_directory( _environment_temp_dir );
#if FOUNDATION_PLATFORM_IOS
		_environment_temp_dir_local = true;
#endif
	}
#endif
#if FOUNDATION_PLATFORM_POSIX
	if( !_environment_temp_dir[0] )
	{
		string_copy( _environment_temp_dir, P_tmpdir, FOUNDATION_MAX_PATHLEN );
		unsigned int len = string_length( _environment_temp_dir );
		if( ( len > 1 ) && ( _environment_temp_dir[ len - 1 ] == '/' ) )
			_environment_temp_dir[ len - 1 ] = 0;
	}
#endif
#if !FOUNDATION_PLATFORM_ANDROID && !FOUNDATION_PLATFORM_IOS
	if( _environment_app.config_dir )
	{
		unsigned int curlen = string_length( _environment_temp_dir );
		unsigned int cfglen = string_length( _environment_app.config_dir );
		if( ( curlen + cfglen + 39 ) < FOUNDATION_MAX_PATHLEN )
		{
			if( _environment_temp_dir[curlen-1] != '/' )
				_environment_temp_dir[curlen++] = '/';
			memcpy( _environment_temp_dir + curlen, _environment_app.config_dir, cfglen );
			_environment_temp_dir[curlen + cfglen] = '-';
			memcpy( _environment_temp_dir + curlen + cfglen + 1, string_from_uuid_static( _environment_app.instance ), 37 );
			_environment_temp_dir_local = true;
		}
	}
#endif
	{
		unsigned int curlen = string_length( _environment_temp_dir );
		if( _environment_temp_dir[curlen-1] == '/' )
			_environment_temp_dir[curlen-1] = 0;
	}
	return _environment_temp_dir;
}


static void _environment_clean_temporary_directory( bool recreate )
{
	const char* path = environment_temporary_directory();

	if( _environment_temp_dir_local && fs_is_directory( path ) )
	{
		fs_remove_directory( path );
		if( recreate )
			fs_make_directory( path );
	}
}


const char* environment_variable( const char* var )
{
#if FOUNDATION_PLATFORM_WINDOWS
	unsigned int required;
	wchar_t* key = wstring_allocate_from_string( var, 0 );
	wchar_t val[FOUNDATION_MAX_PATHLEN]; val[0] = 0;
	if( ( required = GetEnvironmentVariableW( key, val, FOUNDATION_MAX_PATHLEN ) ) > FOUNDATION_MAX_PATHLEN )
	{
		wchar_t* val_local = memory_allocate( 0, sizeof( wchar_t ) * ( required + 2 ), 0, MEMORY_TEMPORARY );
		val_local[0] = 0;
		GetEnvironmentVariableW( key, val_local, required + 1 );
		if( _environment_var )
			string_deallocate( _environment_var );
		_environment_var = string_allocate_from_wstring( val_local, 0 );
		memory_deallocate( val_local );
	}
	else
	{
		if( _environment_var )
			string_deallocate( _environment_var );
		_environment_var = string_allocate_from_wstring( val, 0 );
	}
	wstring_deallocate( key );
	return _environment_var;
#elif FOUNDATION_PLATFORM_POSIX
	return getenv( var );
#elif FOUNDATION_PLATFORM_PNACL
	FOUNDATION_UNUSED( var );
	return 0; //No env vars on PNaCl
#else
#  error Not implemented
	return 0;
#endif
}


const application_t* environment_application( void )
{
	return &_environment_app;
}
