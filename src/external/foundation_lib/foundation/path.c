/* path.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <string.h>


char* path_clean( char* path, bool absolute )
{
	//Since this function is used a lot we want to perform as much operations
	//in place instead of splicing up into a string array and remerge
	char* replace;
	char* inpath;
	char* next;
	unsigned int inlength, length, remain, protocollen, up, last_up, prev_up, driveofs;

	if( !path )
		return string_allocate( 0 );

	inpath = path;
	inlength = string_length( path );
	protocollen = string_find_string( path, "://", 0 );
	if( ( protocollen != STRING_NPOS ) && ( protocollen > 1 ) )
	{
		absolute = true;
		protocollen += 3; //Also skip the "://" separator
		inlength -= protocollen;
		path += protocollen;
	}
	else
	{
		protocollen = 0;
	}
	length = inlength;
	driveofs = 0;

	replace = path;
	while( ( replace = strchr( replace, '\\' ) ) != 0 )
		*replace++ = '/';

	remain = length;
	replace = path;
	while( ( next = strstr( replace, "/./" ) ) != 0 )
	{
		remain -= (unsigned int)( next - replace ) + 2;
		length -= 2;
		memmove( next + 1, next + 3, remain ); //Include terminating zero to avoid looping when string ends in "/./"
		replace = next;
	}

	remain = length;
	replace = path;
	while( ( next = strstr( replace, "//" ) ) != 0 )
	{
		remain -= (unsigned int)( next - replace ) + 1;
		--length;
		memmove( next + 1, next + 2, remain ); //Include terminating zero to avoid looping when string ends in "//"
		replace = next;
	}

	path[length] = 0;

	if( string_equal( path, "." ) )
	{
		length = 0;
		path[0] = 0;
	}
	else if( length > 1 )
	{
		if( ( path[ length - 2 ] == '/' ) && ( path[ length - 1 ] == '.' ) )
		{
			path[ length - 2 ] = 0;
			length -= 2;
		}
		if( string_equal( path, "." ) )
		{
			length = 0;
			path[0] = 0;
		}
		else if( string_equal( path, "./" ) )
		{
			length = 1;
			path[0] = '/';
			path[1] = 0;
		}
		else if( ( length > 1 ) && ( path[0] == '.' ) && ( path[1] == '/' ) )
		{
			--length;
			memmove( path, path + 1, length );
			path[length] = 0;
		}
	}

	if( absolute )
	{
		if( !length )
		{
			if( !inlength )
			{
				inlength = 2;
				inpath = memory_reallocate( inpath, inlength + protocollen + 1, 0, protocollen + 1 );
				path = inpath + protocollen;
			}
			path[0] = '/';
			path[1] = 0;
			++length;
		}
		else if( ( length >= 2 ) && ( path[1] == ':' ) )
		{
			driveofs = 2;

			//Make sure first character is upper case
			if( ( path[0] >= 'a' ) && ( path[0] <= 'z' ) )
				path[0] = ( path[0] - (char)( (int)'a' - (int)'A' ) );

			if( length == 2 )
			{
				if( inlength <= 2 )
				{
					inpath = memory_reallocate( inpath, inlength + 2 + protocollen + 1, 0, inlength + protocollen + 1 );
					inlength += 2;
					path = inpath + protocollen;
				}
				path[2] = '/';
				++length;
			}
			else if( path[2] != '/' )
			{
				//splice in slash in weird-format paths (C:foo/bar/...)
				if( inlength < ( length + 1 ) )
				{
					//Need more space
					inpath = memory_reallocate( inpath, length + protocollen + 2, 0, inlength + protocollen + 1 );
					inlength = length + 1;
					path = inpath + protocollen;
				}

				memmove( path + 3, path + 2, length + 1 - 2 );
				path[2] = '/';
				++length;
			}
		}
		else if( !protocollen && ( path[0] != '/' ) )
		{
			//make sure capacity is enough to hold additional character
			if( inlength < ( length + 1 ) )
			{
				//Need more space
				inpath = memory_reallocate( inpath, length + protocollen + 2, 0, inlength + protocollen + 1 );
				inlength = length + 1;
				path = inpath + protocollen;
			}

			memmove( path + 1, path, length + 1 );
			path[0] = '/';
			++length;
		}
	}
	else //relative
	{
		if( length && ( path[0] == '/' ) )
		{
			memmove( path, path + 1, length - 1 );
			--length;
		}
	}

	//Deal with .. references
	last_up = driveofs;
	while( ( up = string_find_string( path, "/../", last_up ) ) != STRING_NPOS )
	{
		if( up >= length )
			break;
		if( up == driveofs )
		{
			if( absolute )
			{
				memmove( path + driveofs + 1, path + driveofs + 4, length - ( driveofs + 3 ) );
				length -= 3;
			}
			else
			{
				last_up = driveofs + 3;
			}
			continue;
		}
		prev_up = string_rfind( path, '/', up - 1 );
		if( prev_up == STRING_NPOS )
		{
			if( absolute )
			{
				memmove( path, path + up + 3, length - up - 2 );
				length -= ( up + 3 );
			}
			else
			{
				memmove( path, path + up + 4, length - up - 3 );
				length -= ( up + 4 );
			}
		}
		else if( prev_up >= last_up )
		{
			memmove( path + prev_up, path + up + 3, length - up - 2 );
			length -= ( up - prev_up + 3 );
		}
		else
		{
			last_up = up + 1;
		}
	}

	if( length > 1 )
	{
		if( path[ length - 1 ] == '/' )
		{
			path[ length - 1 ] = 0;
			--length;
		}
	}

	if( protocollen )
	{
		if( path[0] == '/' )
		{
			if( length == 1 )
				length = 0;
			else
			{
				memmove( path, path + 1, length );
				--length;
			}
		}
		length += protocollen;
		path = inpath;
	}

	path[length] = 0;

	return path;
}


char* path_base_file_name( const char* path )
{
	unsigned int start, end;
	if( !path )
		return string_allocate( 0 );
	start = string_find_last_of( path, "/\\", STRING_NPOS );
	end = string_find( path, '.', ( start != STRING_NPOS ) ? start : 0 );
	//For "dot" files, i.e files with names like "/path/to/.file", return the dot name ".file"
	if( !end || ( end == ( start + 1 ) ) )
		end = STRING_NPOS;
	if( start != STRING_NPOS )
		return string_substr( path, start + 1, ( end != STRING_NPOS ) ? ( end - start - 1 ) : STRING_NPOS );
	return string_substr( path, 0, end );
}


char* path_base_file_name_with_path( const char* path )
{
	unsigned int start, end;
	char* base;
	if( !path )
		return string_allocate( 0 );
	start = string_find_last_of( path, "/\\", STRING_NPOS );
	end = string_rfind( path, '.', STRING_NPOS );
	//For "dot" files, i.e files with names like "/path/to/.file", return the dot name ".file"
	if( !end || ( end == ( start + 1 ) ) || ( ( start != STRING_NPOS ) && ( end < start ) ) )
		end = STRING_NPOS;
	base = string_substr( path, 0, ( end != STRING_NPOS ) ? end : STRING_NPOS );
	base = path_clean( base, path_is_absolute( base ) );
	return base;
}


char* path_file_extension( const char* path )
{
	unsigned int start = string_find_last_of( path, "/\\", STRING_NPOS );
	unsigned int end = string_rfind( path, '.', STRING_NPOS );
	if( ( end != STRING_NPOS ) && ( ( start == STRING_NPOS ) || ( end > start ) ) )
		return string_substr( path, end + 1, STRING_NPOS );
	return string_clone( "" );
}


char* path_file_name( const char* path )
{
	unsigned int end = string_find_last_of( path, "/\\", STRING_NPOS );
	if( end == STRING_NPOS )
		return string_clone( path );
	return string_substr( path, end + 1, STRING_NPOS );
}


char* path_directory_name( const char* path )
{
	char* pathname;
	unsigned int pathprotocol;
	unsigned int pathstart = 0;
	unsigned int end = string_find_last_of( path , "/\\", STRING_NPOS );
	if( end == 0 )
		return string_clone( "/" );
	if( end == STRING_NPOS )
		return string_allocate( 0 );
	pathprotocol = string_find_string( path, "://", 0 );
	if( pathprotocol != STRING_NPOS )
		pathstart = pathprotocol +=2; // add two to treat as absolute path
	pathname = string_substr( path, pathstart, end - pathstart );
	pathname = path_clean( pathname, path_is_absolute( pathname ) );
	return pathname;
}


char* path_subdirectory_name( const char* path, const char* root )
{
	char* subpath;
	char* testpath;
	char* testroot;
	char* pathofpath;
	unsigned int pathprotocol, rootprotocol;
	char* cpath = string_clone( path );
	char* croot = string_clone( root );

	cpath = path_clean( cpath, path_is_absolute( cpath ) );
	croot = path_clean( croot, path_is_absolute( croot ) );

	pathofpath = path_directory_name( cpath );

	testpath = pathofpath;
	pathprotocol = string_find_string( testpath, "://", 0 );
	if( pathprotocol != STRING_NPOS )
		testpath += pathprotocol + 2; // add two to treat as absolute path

	testroot = croot;
	rootprotocol = string_find_string( testroot, "://", 0 );
	if( rootprotocol != STRING_NPOS )
		testroot += rootprotocol + 2;

	if( ( rootprotocol != STRING_NPOS ) && ( ( pathprotocol == STRING_NPOS ) || ( pathprotocol != rootprotocol ) || !string_equal_substr( cpath, croot, rootprotocol ) ) )
		subpath = string_allocate( 0 );
	else if( !string_equal_substr( testpath, testroot, string_length( testroot ) ) )
		subpath = string_allocate( 0 );
	else
	{
		char* filename = path_file_name( cpath );

		subpath = string_substr( testpath, string_length( testroot ), STRING_NPOS );
		subpath = path_clean( path_append( subpath, filename ), false );

		string_deallocate( filename );
	}
	string_deallocate( pathofpath );
	string_deallocate( cpath );
	string_deallocate( croot );

	return subpath;
}


char* path_protocol( const char* uri )
{
	unsigned int end = string_find_string( uri, "://", 0 );
	if( end == STRING_NPOS )
		return string_allocate( 0 );
	return string_substr( uri, 0, end );
}


char* path_merge( const char* first, const char* second )
{
	char* merged;
	if( !first )
		merged = string_clone( second );
	else if( !second )
		merged = string_clone( first );
	else
	{
		merged = string_format( "%s/", first );
		merged = string_append( merged, second );
	}
	merged = path_clean( merged, path_is_absolute( first ) );
	return merged;
}


char* path_append( char* base, const char* tail )
{
	if( !base )
		return path_clean( string_clone( tail ), path_is_absolute( tail ) );
	base = string_append( base, "/" );
	base = string_append( base, tail );
	base = path_clean( base, path_is_absolute( base ) );
	return base;
}


char* path_prepend( char* tail, const char* base )
{
	if( !tail )
		return path_clean( string_clone( base ), path_is_absolute( base ) );
	tail = string_prepend( tail, "/" );
	tail = string_prepend( tail, base );
	tail = path_clean( tail, path_is_absolute( tail ) );
	return tail;
}


bool path_is_absolute( const char* path )
{
	if( !path || !strlen( path ) )
		return false;
	if( strstr( path, "://" ) )
		return true;
	if( ( path[0] == '/' ) || ( path[0] == '\\' ) )
		return true;
	if( path[1] == ':' ) //Skip separator test to treat weird formats like "C:path/to/something" as absolute
		return true;
	return false;
}


char* path_make_absolute( const char* path )
{
	unsigned int up, last, length, protocollen;
	char* abspath = string_clone( path );
	if( !path_is_absolute( abspath ) )
	{
		abspath = string_prepend( abspath, "/" );
		abspath = string_prepend( abspath, environment_current_working_directory() );
		abspath = path_clean( abspath, true );
	}
	else
	{
		abspath = path_clean( abspath, true );
	}

	protocollen = string_find_string( abspath, "://", 0 );
	if( protocollen != STRING_NPOS )
		protocollen += 3; //Also skip the "://" separator
	else
		protocollen = 0;

	//Deal with .. references
	while( ( up = string_find_string( abspath, "/../", 0 ) ) != STRING_NPOS )
	{
		char* subpath;
		if( ( protocollen && ( up == ( protocollen - 1 ) ) ) || ( !protocollen && ( up == 0 ) ) )
		{
			//This moves mem so "prot://../path" ends up as "prot://path"
			memmove( abspath + protocollen, abspath + 3 + protocollen, string_length( abspath ) + 1 - ( 3 + protocollen ) );
			continue;
		}
		last = string_rfind( abspath, '/', up - 1 );
		if( last == STRING_NPOS )
		{
			//Must be a path like C:/../something since other absolute paths
			last = up;
		}
		subpath = string_substr( abspath, 0, last );
		subpath = string_append( subpath, abspath + up + 3 ); // +3 will include the / of the later part of the path
		string_deallocate( abspath );
		abspath = subpath;
	}

	length = string_length( abspath );
	if( length >= 3 )
	{
		while( ( length >= 3 ) && ( abspath[length-3] == '/' ) && ( abspath[length-2] == '.' ) && ( abspath[length-1] == L'.' ) )
		{
			//Step up
			if( length == 3 )
			{
				abspath[1] = 0;
				length = 1;
			}
			else
			{
				length = string_rfind( abspath, '/', length - 4 );
				abspath[length] = 0;
			}
		}
	}

	return abspath;
}


char* path_make_temporary( void )
{
	char uintbuffer[18];
	return path_merge( environment_temporary_directory(), string_from_uint_buffer( uintbuffer, random64(), true, 0, '0' ) );
}

