/* library.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#elif FOUNDATION_PLATFORM_POSIX
#  include <dlfcn.h>
#endif

struct library_t
{
	FOUNDATION_DECLARE_OBJECT;

	hash_t           namehash;
	char             name[32];

#if FOUNDATION_PLATFORM_WINDOWS
	HANDLE           dll;
#elif FOUNDATION_PLATFORM_POSIX
	void*            lib;
#endif
};
typedef FOUNDATION_ALIGN(8) struct library_t library_t;


static objectmap_t* _library_map;


int _library_initialize( void )
{
	_library_map = objectmap_allocate( BUILD_SIZE_LIBRARY_MAP );
	if( !_library_map )
		return -1;
	return 0;
}


void _library_shutdown( void )
{
	objectmap_deallocate( _library_map );
	_library_map = 0;
}


static void _library_destroy( library_t* library )
{
	if( !library )
		return;

	objectmap_free( _library_map, library->id );

#if FOUNDATION_PLATFORM_WINDOWS
	FreeLibrary( library->dll );
#elif FOUNDATION_PLATFORM_POSIX
	dlclose( library->lib );
#endif

	memory_deallocate( library );
}


object_t library_load( const char* name )
{
	library_t* library;
	hash_t namehash;
	unsigned int i, size;
	uint64_t id;
	const char* basename;
	unsigned int last_slash;
#if FOUNDATION_PLATFORM_WINDOWS
	char* dllname;
	HANDLE dll;
#endif

#if FOUNDATION_PLATFORM_APPLE
#  define FOUNDATION_LIB_PRE "lib"
#  define FOUNDATION_LIB_EXT ".dylib"
#elif FOUNDATION_PLATFORM_WINDOWS
#  define FOUNDATION_LIB_EXT ".dll"
#else
#  define FOUNDATION_LIB_PRE "lib"
#  define FOUNDATION_LIB_EXT ".so"
#endif

	basename = name;
	last_slash = string_rfind( name, '/', STRING_NPOS );
#if FOUNDATION_PLATFORM_WINDOWS
	if( last_slash == STRING_NPOS )
		last_slash = string_rfind( name, '\\', STRING_NPOS );
#endif
	if( last_slash != STRING_NPOS )
		basename = name + last_slash + 1;

	//Locate already loaded library
	library = 0;
	namehash = string_hash( basename );
	for( i = 0, size = objectmap_size( _library_map ); i < size; ++i )
	{
		library = objectmap_raw_lookup( _library_map, i );
		if( library && ( library->namehash == namehash ) )
		{
			FOUNDATION_ASSERT( string_equal( library->name, basename ) );
			atomic_incr32( &library->ref );
			return library->id;
		}
	}

	error_context_push( "loading library", name );

	//Try loading library
#if FOUNDATION_PLATFORM_WINDOWS

	dll = LoadLibraryA( name );
	if( !dll )
	{
		unsigned int last_dot = string_rfind( name, '/', STRING_NPOS );
		if( ( last_dot == STRING_NPOS ) || ( last_dot < last_slash ) )
		{
			dllname = string_format( "%s" FOUNDATION_LIB_EXT, name );
			dll = LoadLibraryA( dllname );
			string_deallocate( dllname );
		}
	}
	if( !dll )
	{
		log_warnf( 0, WARNING_SUSPICIOUS, "Unable to load DLL '%s': %s", name, system_error_message( 0 ) );
		error_context_pop();
		return 0;
	}

#elif FOUNDATION_PLATFORM_POSIX
	char* libname;
	void* lib = dlopen( name, RTLD_LAZY );
	if( !lib && !string_ends_with( name, FOUNDATION_LIB_EXT ) )
	{
		if( last_slash == STRING_NPOS )
		{
			libname = string_format( FOUNDATION_LIB_PRE "%s" FOUNDATION_LIB_EXT, name );
		}
		else
		{
			char* path = path_directory_name( name );
			char* file = path_file_name( name );
			char* decorated_name = string_format( FOUNDATION_LIB_PRE "%s" FOUNDATION_LIB_EXT, file );
			libname = path_merge( path, decorated_name );
			string_deallocate( decorated_name );
			string_deallocate( path );
			string_deallocate( file );
		}
		lib = dlopen( libname, RTLD_LAZY );
		string_deallocate( libname );
	}
#if FOUNDATION_PLATFORM_ANDROID
	if( !lib && ( last_slash == STRING_NPOS ) )
	{
		if( !string_ends_with( name, FOUNDATION_LIB_EXT ) )
			libname = string_format( "%s/" FOUNDATION_LIB_PRE "%s" FOUNDATION_LIB_EXT, environment_executable_directory(), name );
		else
			libname = string_format( "%s/" FOUNDATION_LIB_PRE "%s", environment_executable_directory(), name );
		lib = dlopen( libname, RTLD_LAZY );
		string_deallocate( libname );
	}
#endif
	if( !lib )
	{
		log_warnf( 0, WARNING_SUSPICIOUS, "Unable to load dynamic library '%s': %s", name, dlerror() );
		error_context_pop();
		return 0;
	}

#else

	log_errorf( 0, ERROR_NOT_IMPLEMENTED, "Dynamic library loading not implemented for this platform: %s", name );
	error_context_pop();
	return 0;

#endif

	id = objectmap_reserve( _library_map );
	if( !id )
	{
#if FOUNDATION_PLATFORM_WINDOWS
		FreeLibrary( dll );
#elif FOUNDATION_PLATFORM_POSIX
		dlclose( lib );
#endif
		log_errorf( 0, ERROR_OUT_OF_MEMORY, "Unable to allocate new library '%s', map full", name );
		error_context_pop();
		return 0;
	}
	library = memory_allocate( 0, sizeof( library_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	_object_initialize( (object_base_t*)library, id );
	library->namehash = namehash;
	string_copy( library->name, basename, 32 );
#if FOUNDATION_PLATFORM_WINDOWS
	library->dll = dll;
#elif FOUNDATION_PLATFORM_POSIX
	library->lib = lib;
#endif
	objectmap_set( _library_map, id, library );

	error_context_pop();

	return library->id;
}


object_t library_ref( object_t id )
{
	return _object_ref( objectmap_lookup( _library_map, id ) );
}


void library_unload( object_t id )
{
	void* library = objectmap_lookup( _library_map, id );
	if( !_object_unref( library ) )
		_library_destroy( library );
}


void* library_symbol( object_t id, const char* name )
{
	library_t* library = objectmap_lookup( _library_map, id );
	if( library )
	{
#if FOUNDATION_PLATFORM_WINDOWS
		return GetProcAddress( library->dll, name );
#elif FOUNDATION_PLATFORM_POSIX
		return dlsym( library->lib, name );
#else
		FOUNDATION_UNUSED( name );
		log_errorf( 0, ERROR_NOT_IMPLEMENTED, "Dynamic library symbol lookup implemented for this platform: %s not found", name );
#endif
	}
	return 0;
}


const char* library_name( object_t id )
{
	library_t* library = objectmap_lookup( _library_map, id );
	if( library )
		return library->name;
	return "";
}


bool library_valid( object_t id )
{
	return objectmap_lookup( _library_map, id ) != 0;
}
