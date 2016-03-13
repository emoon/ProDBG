/* fs.m  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#if FOUNDATION_PLATFORM_MACOSX

#include <foundation/apple.h>

#include <dispatch/queue.h>
#include <sys/stat.h>

void* _fs_event_stream_create( const char* path );
void  _fs_event_stream_destroy( void* stream );
void  _fs_event_stream_flush( void* stream );


//This implementation is not optimal in any way, but will do for now
//Memory allocation mania should really be cleaned up


static void _fs_node_make_path( char* target, const char* first, unsigned int firstlen, const char* second, unsigned int secondlen )
{
	memcpy( target, first, firstlen );
	if( first[firstlen-1] != '/' )
		target[firstlen++] = '/';
	memcpy( target + firstlen, second, secondlen );
	target[firstlen + secondlen] = 0;
}


typedef __attribute__((__aligned__(8))) struct file_node_t file_node_t;

struct file_node_t
{
	char*          name;
	file_node_t**  subdirs;
	char**         files;
	uint64_t*      last_modified;
};


static void _fs_node_deallocate( file_node_t* node )
{
	string_deallocate( node->name );
	string_array_deallocate( node->files );
	for( int isub = 0, subsize = array_size( node->subdirs ); isub < subsize; ++isub )
		_fs_node_deallocate( node->subdirs[isub] );
	array_deallocate( node->subdirs );
	array_deallocate( node->last_modified );
	memory_deallocate( node );
}


static void _fs_node_populate( file_node_t* node, const char* fullpath )
{
	char** subdirs = fs_subdirs( fullpath );
	for( int isub = 0, subsize = array_size( subdirs ); isub < subsize; ++isub )
	{
		file_node_t* child = memory_allocate( 0, sizeof( file_node_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
		child->name = subdirs[isub];
		array_push( node->subdirs, child );
	}
	array_deallocate( subdirs ); //Only array, strings are kept in nodes

	node->files = fs_files( fullpath );
	for( int isub = 0, subsize = array_size( node->files ); isub < subsize; ++isub )
	{
		char* filepath = path_merge( fullpath, node->files[isub] );
		uint64_t last_modified = fs_last_modified( filepath );
		array_push( node->last_modified, last_modified );
		//log_debugf( HASH_FOUNDATION, "  populate found file: %s (%llx)", filepath, last_modified );
		string_deallocate( filepath );
	}

	for( int isub = 0, subsize = array_size( node->subdirs ); isub < subsize; ++isub )
	{
		char* subpath = path_merge( fullpath, node->subdirs[isub]->name );
		_fs_node_populate( node->subdirs[isub], subpath );
		string_deallocate( subpath );
	}
}


static file_node_t* _fs_node_find( file_node_t* root, const char* path )
{
	unsigned int pathlen = string_length( path );
	if( !pathlen || string_equal( path, "/" ) )
		return root;

	file_node_t* node = root;
	do
	{
		file_node_t* next = 0;
		unsigned int separator = string_find( path, '/', 0 );
		for( int isub = 0, subsize = array_size( node->subdirs ); isub < subsize; ++isub )
		{
			if( string_equal_substr( node->subdirs[isub]->name, path, separator != STRING_NPOS ? separator : string_length( path ) ) )
			{
				next = node->subdirs[isub];
				path = path + separator + 1;
				break;
			}
		}
		node = next;
		if( !*path )
			return node;
	} while ( node );

	return 0;
}


static void _fs_node_send_deletions( file_node_t* node, const char* path, unsigned int pathlen )
{
	char pathbuf[FOUNDATION_MAX_PATHLEN+1];

	for( int ifile = 0, fsize = array_size( node->files ); ifile < fsize; ++ifile )
	{
		_fs_node_make_path( pathbuf, path, pathlen, node->files[ifile], string_length( node->files[ifile] ) );
		//log_infof( HASH_FOUNDATION, "    subdeleted %s", filepath );
		fs_post_event( FOUNDATIONEVENT_FILE_DELETED, pathbuf, 0 );
	}

	for( int isub = 0, subsize = array_size( node->subdirs ); isub < subsize; ++isub )
	{
		_fs_node_make_path( pathbuf, path, pathlen, node->subdirs[isub]->name, string_length( node->subdirs[isub]->name ) );
		_fs_node_send_deletions( node->subdirs[isub], pathbuf, string_length( pathbuf ) );
	}
}


static void _fs_node_send_creations( file_node_t* node, const char* path, unsigned int pathlen )
{
	char pathbuf[FOUNDATION_MAX_PATHLEN+1];

	for( int ifile = 0, fsize = array_size( node->files ); ifile < fsize; ++ifile )
	{
		_fs_node_make_path( pathbuf, path, pathlen, node->files[ifile], string_length( node->files[ifile] ) );
		//log_infof( HASH_FOUNDATION, "    subcreated %s", filepath );
		fs_post_event( FOUNDATIONEVENT_FILE_CREATED, pathbuf, 0 );
	}

	for( int isub = 0, subsize = array_size( node->subdirs ); isub < subsize; ++isub )
	{
		_fs_node_make_path( pathbuf, path, pathlen, node->subdirs[isub]->name, string_length( node->subdirs[isub]->name ) );
		_fs_node_send_creations( node->subdirs[isub], pathbuf, string_length( pathbuf ) );
	}
}


static void _fs_event_stream_callback( ConstFSEventStreamRef stream_ref, void* user_data, size_t num_events, const char *const event_paths[], const FSEventStreamEventFlags event_flags[], const FSEventStreamEventId event_ids[] )
{
	file_node_t* root_node = user_data;
	unsigned int root_path_len = string_length( root_node->name );
	char pathbuf[FOUNDATION_MAX_PATHLEN+1];
	FOUNDATION_UNUSED( stream_ref );

	@autoreleasepool
	{
		for( size_t i = 0; i < num_events; ++i )
		{
			const char* rawpath = event_paths[i];

			unsigned int rawpath_len = string_length( rawpath );

			FSEventStreamEventFlags flags = event_flags[i];
			FSEventStreamEventId identifier = event_ids[i];

			/* Store path and recurse flag in paths-to-process,
			   then keep state and rescan for changes in fs monitor thread*/
			if( ( flags & kFSEventStreamEventFlagMustScanSubDirs ) != 0 )
			{
				//TODO: Implement
				log_warnf( 0, WARNING_UNSUPPORTED, "Got kFSEventStreamEventFlagMustScanSubDirs: %s (0x%x 0x%x)", rawpath, (unsigned int)flags, (unsigned int)identifier );
			}
			else
			{
				FOUNDATION_UNUSED( identifier );
				//log_debugf( HASH_FOUNDATION, "Got event for: %s (0x%x 0x%x)", rawpath, (unsigned int)flags, (unsigned int)identifier );

				unsigned int root_ofs = string_find_string( rawpath, root_node->name, 0 );
				if( root_ofs == STRING_NPOS )
					continue;

				const char* path = rawpath + root_ofs;
				unsigned int path_len = rawpath_len - root_ofs;

				const char* subpath = path + root_path_len + 1;

				file_node_t* node = _fs_node_find( root_node, subpath );
				if( !node )
					continue;

				char** files = fs_files( rawpath );

				//Check if file have been added, removed or modified
				for( int isub = 0, subsize = array_size( node->files ); isub < subsize; )
				{
					int ifile;

					_fs_node_make_path( pathbuf, path, path_len, node->files[isub], string_length( node->files[isub] ) );

					if( ( ifile = string_array_find( (const char* const*)files, node->files[isub], array_size( files ) ) ) == -1 )
					{
						//log_debugf( HASH_FOUNDATION, "  deleted: %s", pathbuf );
						string_deallocate( node->files[isub] );
						array_erase_memcpy( node->files, isub );
						array_erase_memcpy( node->last_modified, isub );
						--subsize;
						fs_post_event( FOUNDATIONEVENT_FILE_DELETED, pathbuf, 0 );
					}
					else
					{
						uint64_t last_modified = fs_last_modified( pathbuf );
						if( last_modified > node->last_modified[isub] )
						{
							//log_debugf( HASH_FOUNDATION, "  modified: %s (%llx > %llx)", pathbuf, ifile, last_modified, node->last_modified[isub] );
							node->last_modified[isub] = last_modified;
							fs_post_event( FOUNDATIONEVENT_FILE_MODIFIED, pathbuf, 0 );
						}
						++isub;
					}
				}
				for( int isub = 0, subsize = array_size( files ); isub < subsize; ++isub )
				{
					if( string_array_find( (const char* const*)node->files, files[isub], array_size( node->files ) ) == -1 )
					{
						_fs_node_make_path( pathbuf, path, path_len, files[isub], string_length( files[isub] ) );

						uint64_t last_mod = fs_last_modified( pathbuf );

						array_push( node->last_modified, last_mod );
						array_push( node->files, files[isub] );
						files[isub] = 0;
						//log_debugf( HASH_FOUNDATION, "  created: %s (%llx)", pathbuf, last_mod );
						fs_post_event( FOUNDATIONEVENT_FILE_CREATED, pathbuf, 0 );
					}
				}

				string_array_deallocate( files );

				//Check for subdir additions/removals
				char** subdirs = fs_subdirs( rawpath );
				for( int iexist = 0, existsize = array_size( node->subdirs ); iexist < existsize; )
				{
					bool found = false;
					for( int isub = 0, subsize = array_size( subdirs ); isub < subsize; ++isub )
					{
						if( string_equal( node->subdirs[iexist]->name, subdirs[isub] ) )
						{
							found = true;
							break;
						}
					}

					if( !found )
					{
						//log_debugf( HASH_FOUNDATION, "  del subdir: %s %s", node->name, node->subdirs[iexist]->name );

						//Recurse and send out file deletion events
						_fs_node_make_path( pathbuf, rawpath, rawpath_len, node->subdirs[iexist]->name, string_length( node->subdirs[iexist]->name ) );
						_fs_node_send_deletions( node->subdirs[iexist], pathbuf, string_length( pathbuf ) );
						_fs_node_deallocate( node->subdirs[iexist] );
						array_erase_memcpy( node->subdirs, iexist );
						--existsize;
					}
					else
					{
						++iexist;
					}
				}

				for( int isub = 0, subsize = array_size( subdirs ); isub < subsize; ++isub )
				{
					bool found = false;
					for( int iexist = 0, existsize = array_size( node->subdirs ); iexist < existsize; ++iexist )
					{
						if( string_equal( node->subdirs[iexist]->name, subdirs[isub] ) )
						{
							found = true;
							break;
						}
					}

					if( !found )
					{
						file_node_t* child = memory_allocate( 0, sizeof( file_node_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );

						//log_debugf( HASH_FOUNDATION, "  add subdir: %s %s", node->name, subdirs[isub] );
						child->name = subdirs[isub];
						subdirs[isub] = 0;

						array_push( node->subdirs, child );

						_fs_node_make_path( pathbuf, rawpath, rawpath_len, child->name, string_length( child->name ) );
						_fs_node_populate( child, pathbuf );
						_fs_node_send_creations( child, pathbuf + root_ofs, string_length( pathbuf + root_ofs ) );
					}
				}

				string_array_deallocate( subdirs );
			}
		}
	}

	//This is run in a dispatch thread by the OS, need to clean up
	memory_context_thread_deallocate();
}


void _fs_event_stream_flush( void* stream )
{
	FSEventStreamFlushAsync( stream );
}


static const void* _fs_event_stream_retain( const void* info )
{
	return info;
}


static void _fs_event_stream_release( const void* info )
{
	if( info )
		_fs_node_deallocate( (file_node_t*)info );
}


void* _fs_event_stream_create( const char* path )
{
	@autoreleasepool
	{
		file_node_t* node = memory_allocate( 0, sizeof( file_node_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
		node->name = string_clone( path );

		_fs_node_populate( node, path );

		NSString* nspath = [[NSString alloc] initWithUTF8String:path];
		NSArray* patharr = [NSArray arrayWithObject:nspath];
		FSEventStreamContext context = { 0, node, _fs_event_stream_retain, _fs_event_stream_release, 0 };
		NSTimeInterval latency = 1.0;

		//TODO: Implement allocator based on foundation memory allocation subsystem
		void* stream = FSEventStreamCreate( 0, (FSEventStreamCallback)&_fs_event_stream_callback, &context, (__bridge CFArrayRef)patharr, kFSEventStreamEventIdSinceNow, (CFAbsoluteTime)latency, kFSEventStreamCreateFlagNone );
		if( stream )
		{
			FSEventStreamSetDispatchQueue( stream, dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 ) );
			if( NO == FSEventStreamStart( stream ) )
			{
				log_error( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to start FS event stream" );
			}
		}
		else
		{
			log_error( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to create FS event stream" );
		}

		log_debugf( 0, "Started FS event stream for: %s", path );

		return stream;
	}
}


void _fs_event_stream_destroy( void* stream )
{
	if( !stream )
		return;

	@autoreleasepool
	{
		FSEventStreamStop( stream );
		FSEventStreamInvalidate( stream );
		FSEventStreamRelease( stream );
	}
}

#endif

