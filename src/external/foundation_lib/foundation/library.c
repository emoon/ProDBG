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
#if FOUNDATION_PLATFORM_WINDOWS
	char* dllname;
	HANDLE dll;
#endif

	//Locate already loaded library
	library = 0;
	namehash = string_hash( name );
	for( i = 0, size = objectmap_size( _library_map ); i < size; ++i )
	{
		library = objectmap_raw_lookup( _library_map, i );
		if( library && ( library->namehash == namehash ) )
		{
			FOUNDATION_ASSERT( string_equal( library->name, name ) );
			atomic_incr32( &library->ref );
			return library->id;
		}
	}

	error_context_push( "loading library", name );

	//Try loading library
#if FOUNDATION_PLATFORM_WINDOWS

	dllname = string_format( "%s.dll", name );
	dll = LoadLibraryA( dllname );
	if( !dll )
	{
#if FOUNDATION_ARCH_X86
		string_deallocate( dllname );
		dllname = string_format( "%s32.dll", name );
		dll = LoadLibraryA( dllname );
#elif FOUNDATION_ARCH_X86_64
		string_deallocate( dllname );
		dllname = string_format( "%s64.dll", name );
		dll = LoadLibraryA( dllname );
#endif
	}
	string_deallocate( dllname );
	if( !dll )
	{
		log_warnf( 0, WARNING_SUSPICIOUS, "Unable to load DLL '%s': %s", name, system_error_message( 0 ) );
		error_context_pop();
		return 0;
	}

#elif FOUNDATION_PLATFORM_POSIX

#  if FOUNDATION_PLATFORM_APPLE
	char* libname = string_format( "lib%s.dylib", name );
#  else
	char* libname = string_format( "lib%s.so", name );
#  endif
	void* lib = dlopen( libname, RTLD_LAZY );
	string_deallocate( libname );
#if FOUNDATION_PLATFORM_ANDROID
	if( !lib )
	{
		libname = string_format( "%s/lib%s.so", environment_executable_directory(), name );
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
	library->namehash = string_hash( name );
	string_copy( library->name, name, 32 );
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
