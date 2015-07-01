/* stream.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <stdarg.h>

#if FOUNDATION_PLATFORM_WINDOWS
#  include <foundation/windows.h>
#endif

#if FOUNDATION_PLATFORM_POSIX
#  include <foundation/posix.h>
#  include <sys/select.h>
#  include <sys/stat.h>
#endif


static hashtable64_t* _stream_protocol_table;


static stream_t* _stream_open_stdout( const char* path, unsigned int mode )
{
	FOUNDATION_UNUSED( path );
	stream_t* stream = stream_open_stdout();
	stream->mode = ( mode & STREAM_BINARY ) | STREAM_OUT;
	return stream;
}


static stream_t* _stream_open_stderr( const char* path, unsigned int mode )
{
	FOUNDATION_UNUSED( path );
	stream_t* stream = stream_open_stderr();
	stream->mode = ( mode & STREAM_BINARY ) | STREAM_OUT;
	return stream;
}


static stream_t* _stream_open_stdin( const char* path, unsigned int mode )
{
	FOUNDATION_UNUSED( path );
	stream_t* stream = stream_open_stdin();
	stream->mode = ( mode & STREAM_BINARY ) | STREAM_IN;
	return stream;
}


int _stream_initialize( void )
{
	_stream_protocol_table = hashtable64_allocate( 32 );

	stream_set_protocol_handler( "", fs_open_file );
#if FOUNDATION_PLATFORM_ANDROID
	stream_set_protocol_handler( "asset", asset_stream_open );
#endif
	stream_set_protocol_handler( "file", fs_open_file );
	stream_set_protocol_handler( "stdout", _stream_open_stdout );
	stream_set_protocol_handler( "stderr", _stream_open_stderr );
	stream_set_protocol_handler( "stdin", _stream_open_stdin );
	return 0;
}


void _stream_shutdown( void )
{
	hashtable64_deallocate( _stream_protocol_table );
	_stream_protocol_table = 0;
}


void stream_set_protocol_handler( const char* protocol, stream_open_fn fn )
{
	hashtable64_set( _stream_protocol_table, hash( protocol, string_length( protocol ) ), (uint64_t)(uintptr_t)fn );
}


stream_open_fn stream_protocol_handler( const char* protocol, unsigned int length )
{
	return (stream_open_fn)(uintptr_t)hashtable64_get( _stream_protocol_table, hash( protocol, length ? length : string_length( protocol ) ) );
}


void stream_initialize( stream_t* stream, byteorder_t order )
{
	stream->byteorder = order;
	stream->sequential = 0;
	stream->reliable = 1;
	stream->inorder = 1;
	stream->swap = ( stream->byteorder != system_byteorder() ) ? 1 : 0;
	stream->mode = STREAM_BINARY;
	stream->path = 0;
}


stream_t* stream_open( const char* path, unsigned int mode )
{
	unsigned int protocol_end;
	stream_open_fn open_fn = 0;

	//Check if protocol was given
	protocol_end = string_find_string( path, "://", 0 );
	open_fn = stream_protocol_handler( ( protocol_end != STRING_NPOS ) ? path : "", ( protocol_end != STRING_NPOS ) ? protocol_end : 0 );

	return open_fn ? open_fn( path, mode ) : 0;
}


void stream_deallocate( stream_t* stream )
{
	if( !stream )
		return;
	stream_finalize( stream );
	memory_deallocate( stream );
}


void stream_finalize( stream_t* stream )
{
	if( stream->vtable && stream->vtable->finalize )
		stream->vtable->finalize( stream );

	string_deallocate( stream->path );

	stream->path = 0;
	stream->type = STREAMTYPE_INVALID;
}


stream_t* stream_clone( stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	if( stream->vtable->clone )
		return stream->vtable->clone( stream );
	return 0;
}


void stream_set_byteorder( stream_t* stream, byteorder_t byteorder )
{
	FOUNDATION_ASSERT( stream );
	stream->byteorder = byteorder;
	stream->swap = ( byteorder != system_byteorder() ) ? 1 : 0;
}


void stream_set_binary( stream_t* stream, bool binary )
{
	FOUNDATION_ASSERT( stream );
	if( binary )
		stream->mode |= STREAM_BINARY;
	else
		stream->mode &= ~STREAM_BINARY;
}


bool stream_is_binary( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return ( ( stream->mode & STREAM_BINARY ) == 0 ) ? false : true;
}


bool stream_is_sequential( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return stream->sequential;
}


bool stream_is_swapped( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return stream->swap;
}


bool stream_is_reliable( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return stream->reliable;
}


bool stream_is_inorder( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return stream->inorder;
}


bool stream_eos( stream_t* stream )
{
	return ( stream && stream->vtable->eos ? stream->vtable->eos( stream ) : false );
}


byteorder_t stream_byteorder( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return stream->byteorder;
}


const char* stream_path( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return stream->path;
}


uint64_t stream_last_modified( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return ( stream->vtable->lastmod ? stream->vtable->lastmod( stream ) : 0 );
}


void stream_seek( stream_t* stream, int64_t offset, stream_seek_mode_t direction )
{
	FOUNDATION_ASSERT( stream );
	if( stream->sequential )
		return;

	FOUNDATION_ASSERT( stream->vtable->seek );
	stream->vtable->seek( stream, offset, direction );
}


int64_t stream_tell( stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return stream->vtable->tell ? stream->vtable->tell( stream ) : 0;
}


uint64_t stream_read( stream_t* stream, void* buffer, uint64_t num_bytes )
{
	FOUNDATION_ASSERT( stream );
	if( !( stream->mode & STREAM_IN ) )
		return 0;

	FOUNDATION_ASSERT( stream->vtable->read );
	return stream->vtable->read( stream, buffer, num_bytes );
}


uint64_t stream_read_line_buffer( stream_t* stream, char* dest, unsigned int count, char delimiter )
{
	int i, read, total, limit;

	FOUNDATION_ASSERT( stream );
	FOUNDATION_ASSERT( dest );
	if( !( stream->mode & STREAM_IN ) || ( count < 2 ) )
		return 0;

	FOUNDATION_ASSERT( stream->vtable->read );

	total = 0;

	--count;
	while( !stream_eos( stream ) )
	{
		limit = count - total;
		if( limit > 128 )
			limit = 128;
		if( !limit )
			break;

		if( stream_is_sequential( stream ) ) //Need to read one byte at a time since we can't scan back if overreading
			limit = 1;

		read = (int)stream->vtable->read( stream, dest + total, limit );
		if( !read )
			break;
		for( i = 0; i < read; ++i )
		{
			if( dest[total+i] == delimiter )
				break;
		}
		total += i;
		if( i < read )
		{
			if( ( i + 1 ) < read )
			{
				FOUNDATION_ASSERT( !stream_is_sequential( stream ) ); //Sequential should never end up here reading one byte at a time
				stream_seek( stream, 1 + i - read, FL_STREAM_SEEK_CURRENT );
			}
			break;
		}
	}

	dest[total] = 0;

	return total;
}


char* stream_read_line( stream_t* stream, char delimiter )
{
	char buffer[128];
	char* outbuffer = 0;
	int outsize = 0;
	int cursize = 0;
	int read, i;
	int want_read = 128;

	FOUNDATION_ASSERT( stream );
	if( !( stream->mode & STREAM_IN ) )
		return 0;

	FOUNDATION_ASSERT( stream->vtable->read );

	if( stream_is_sequential( stream ) ) //Need to read one byte at a time since we can't scan back if overreading
		want_read = 1;

	while( !stream_eos( stream ) )
	{
		read = (int)stream->vtable->read( stream, buffer, want_read );
		if( !read )
			break;
		for( i = 0; i < read; ++i )
		{
			if( buffer[i] == delimiter )
				break;
		}
		if( cursize + i > outsize )
		{
			unsigned int nextsize;
			if( !outbuffer )
			{
				nextsize = ( i >= 32 ? i + 1 : ( i > 1 ? i + 1 : 32 ) );
				outbuffer = memory_allocate( 0, nextsize, 0, MEMORY_PERSISTENT );
			}
			else
			{
				nextsize = ( outsize < 511 ? 512 : outsize + 513 ); //Always aligns to 512 multiples
				FOUNDATION_ASSERT( !( nextsize % 512 ) );
				outbuffer = memory_reallocate( outbuffer, nextsize, 0, outsize + 1 );
			}
			outsize = nextsize - 1;
		}
		if( i )
		{
			memcpy( outbuffer + cursize, buffer, i );
			cursize += i;
		}
		if( i < read )
		{
			if( ( i + 1 ) < read )
			{
				FOUNDATION_ASSERT( !stream_is_sequential( stream ) ); //Sequential should never end up here reading one byte at a time
				stream_seek( stream, 1 + i - read, FL_STREAM_SEEK_CURRENT );
			}
			break;
		}
	}

	if( outbuffer )
		outbuffer[cursize] = 0;

	return outbuffer;
}


uint64_t stream_size( stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return ( stream->vtable->size ? stream->vtable->size( stream ) : 0 );
}


void stream_determine_binary_mode( stream_t* stream, unsigned int num )
{
	char* buf;
	int64_t cur;
	uint64_t actual_read, i;

	FOUNDATION_ASSERT( stream );
	if( !( stream->mode & STREAM_IN ) || stream_is_sequential( stream ) )
		return;

	if( !num )
		num = 8;

	buf = memory_allocate( 0, num, 0, MEMORY_TEMPORARY );
	memset( buf, 32, num );

	cur = stream_tell( stream );
	actual_read = stream_read( stream, buf, num );
	stream_seek( stream, cur, FL_STREAM_SEEK_BEGIN );

	stream->mode &= ~STREAM_BINARY;

	for( i = 0; i < actual_read; ++i )
	{
		//TODO: What about UTF-8?
		if( ( ( buf[i] < 0x20 ) && ( buf[i] != 0x09 ) && ( buf[i] != 0x0a ) && ( buf[i] != 0x0d ) ) || ( buf[i] > 0x7e ) )
		{
			stream->mode |= STREAM_BINARY;
			break;
		}
	}

	memory_deallocate( buf );
}


bool stream_read_bool( stream_t* stream )
{
	bool value = false;

	FOUNDATION_ASSERT( stream );
	if( stream_is_binary( stream ) )
	{
		char c = 0;
		stream_read( stream, &c, 1 );
		value = ( c ? true : false );
	}
	else
	{
		char buffer[6] = {0};
		stream_read_string_buffer( stream, buffer, 6 );
		value = !( !string_length( buffer ) || string_equal( buffer, "false" ) || ( string_equal( buffer, "0" ) ) );
	}

	return value;
}


int8_t stream_read_int8( stream_t* stream )
{
	int8_t value = 0;
	if( stream_is_binary( stream ) )
		stream_read( stream, &value, 1 );
	else
	{
		char buffer[6] = {0};
		stream_read_string_buffer( stream, buffer, 6 );
		value = (int8_t)string_to_int( buffer );
	}
	return value;
}


uint8_t stream_read_uint8( stream_t* stream )
{
	uint8_t value = 0;
	if( stream_is_binary( stream ) )
		stream_read( stream, &value, 1 );
	else
	{
		char buffer[6] = {0};
		stream_read_string_buffer( stream, buffer, 6 );
		value = (uint8_t)string_to_uint( buffer, false );
	}
	return value;
}


int16_t stream_read_int16( stream_t* stream )
{
	int16_t value = 0;
	if( stream_is_binary( stream ) )
	{
		stream_read( stream, &value, 2 );
		if( stream && stream->swap )
			value = byteorder_swap16( value );
	}
	else
	{
		char buffer[8] = {0};
		stream_read_string_buffer( stream, buffer, 8 );
		value = (int16_t)string_to_int( buffer );
	}
	return value;
}


uint16_t stream_read_uint16( stream_t* stream )
{
	uint16_t value = 0;
	if( stream_is_binary( stream ) )
	{
		stream_read( stream, &value, 2 );
		if( stream && stream->swap )
			value = byteorder_swap16( value );
	}
	else
	{
		char buffer[8] = {0};
		stream_read_string_buffer( stream, buffer, 8 );
		value = (uint16_t)string_to_uint( buffer, false );
	}
	return value;
}


int32_t stream_read_int32( stream_t* stream )
{
	int32_t value = 0;
	if( stream_is_binary( stream ) )
	{
		stream_read( stream, &value, 4 );
		if( stream && stream->swap )
			value = byteorder_swap32( value );
	}
	else
	{
		char buffer[12] = {0};
		stream_read_string_buffer( stream, buffer, 12 );
		value = (int32_t)string_to_int( buffer );
	}
	return value;
}


uint32_t stream_read_uint32( stream_t* stream )
{
	uint32_t value = 0;
	if( stream_is_binary( stream ) )
	{
		stream_read( stream, &value, 4 );
		if( stream && stream->swap )
			value = byteorder_swap32( value );
	}
	else
	{
		char buffer[12] = {0};
		stream_read_string_buffer( stream, buffer, 12 );
		value = (uint32_t)string_to_uint( buffer, false );
	}
	return value;
}


int64_t stream_read_int64( stream_t* stream )
{
	int64_t value = 0;
	if( stream_is_binary( stream ) )
	{
		stream_read( stream, &value, 8 );
		if( stream && stream->swap )
			value = byteorder_swap64( value );
	}
	else
	{
		char buffer[22] = {0};
		stream_read_string_buffer( stream, buffer, 22 );
		value = string_to_int64( buffer );
	}
	return value;
}


uint64_t stream_read_uint64( stream_t* stream )
{
	uint64_t value = 0;
	if( stream_is_binary( stream ) )
	{
		stream_read( stream, &value, 8 );
		if( stream && stream->swap )
			value = byteorder_swap64( value );
	}
	else
	{
		char buffer[22] = {0};
		stream_read_string_buffer( stream, buffer, 22 );
		value = string_to_uint64( buffer, false );
	}
	return value;
}


float32_t stream_read_float32( stream_t* stream )
{
	float32_t value = 0;
	if( stream_is_binary( stream ) )
	{
		stream_read( stream, &value, 4 );
		if( stream && stream->swap )
		{
			float32_cast_t cast;
			cast.fval = value;
			cast.ival = byteorder_swap32( cast.ival );
			value = cast.fval;
		}
	}
	else
	{
		char buffer[32] = {0};
		stream_read_string_buffer( stream, buffer, 32 );
		value = string_to_float32( buffer );
	}
	return value;
}


float64_t stream_read_float64( stream_t* stream )
{
	float64_t value = 0;
	if( stream_is_binary( stream ) )
	{
		stream_read( stream, &value, 8 );
		if( stream && stream->swap )
		{
			float64_cast_t cast;
			cast.fval = value;
			cast.ival = byteorder_swap64( cast.ival );
			value = cast.fval;
		}
	}
	else
	{
		char buffer[64] = {0};
		stream_read_string_buffer( stream, buffer, 64 );
		value = string_to_float64( buffer );
	}
	return value;
}


char* stream_read_string( stream_t* stream )
{
	char buffer[128];
	char* outbuffer = buffer;
	int outsize = sizeof( buffer );
	int cursize = 0;
	int read, i;
	bool binary = stream_is_binary( stream );

	FOUNDATION_ASSERT( stream );
	if( !( stream->mode & STREAM_IN ) )
		return 0;

	FOUNDATION_ASSERT( stream->vtable->read );

	if( stream_is_sequential( stream ) )
	{
		//Single byte reading since we can't seek backwards (and don't want to block on network sockets)
		char c;
		if( !binary )
		{
			//Consume whitespace
			while( !stream_eos( stream ) )
			{
				read = (int)stream->vtable->read( stream, &c, 1 );
				if( !read )
					break;
				if( ( c != ' ' ) && ( c != '\n' ) && ( c != '\r' ) && ( c != '\t' ) )
				{
					buffer[cursize++] = c;
					break;
				}
			}
		}

		if( cursize > 0 )
		{
			while( !stream_eos( stream ) )
			{
				read = (int)stream->vtable->read( stream, &c, 1 );
				if( !read )
					break;
				if( !c )
					break;
				if( !binary && ( ( c == ' ' ) || ( c == '\n' ) || ( c == '\r' ) || ( c == '\t' ) ) )
					break;
				if( cursize + 1 >= outsize )
				{
					if( !outbuffer )
					{
						outsize += 512;
						if( outbuffer != buffer )
						{
							outbuffer = memory_reallocate( outbuffer, outsize, 0, cursize );
						}
						else
						{
							outbuffer = memory_allocate( 0, outsize, 0, MEMORY_PERSISTENT );
							memcpy( outbuffer, buffer, sizeof( buffer ) );
						}
					}
				}
				outbuffer[cursize++] = c;
			}

			outbuffer[cursize] = 0;
		}
	}
	else
	{
		if( !binary )
		{
			//Consume whitespace
			while( !stream_eos( stream ) )
			{
				read = (int)stream->vtable->read( stream, buffer, 16 );
				if( !read )
					break;
				for( i = 0; i < read; ++i )
				{
					char c = buffer[i];
					if( ( c != ' ' ) && ( c != '\n' ) && ( c != '\r' ) && ( c != '\t' ) )
						break;
				}
				if( i < read )
				{
					stream_seek( stream, i - read, FL_STREAM_SEEK_CURRENT );
					break;
				}
			}
		}

		while( !stream_eos( stream ) )
		{
			read = (int)stream->vtable->read( stream, buffer, 128 );
			if( !read )
				break;
			for( i = 0; i < read; ++i )
			{
				char c = buffer[i];
				if( !c )
					break;
				if( !binary && ( ( c == ' ' ) || ( c == '\n' ) || ( c == '\r' ) || ( c == '\t' ) ) )
					break;
			}
			if( i )
			{
				if( cursize + i >= outsize )
				{
					outsize += 512;
					if( outbuffer != buffer )
					{
						outbuffer = memory_reallocate( outbuffer, outsize, 0, cursize );
					}
					else
					{
						FOUNDATION_ASSERT( cursize == 0 ); //Or internal assumptions about code flow is incorrect
						outbuffer = memory_allocate( 0, outsize, 0, MEMORY_PERSISTENT );
					}
				}
				memcpy( outbuffer + cursize, buffer, i );
				cursize += i;
			}
			if( i < 128 )
			{
				if( ( i + 1 ) < read )
					stream_seek( stream, 1 + i - read, FL_STREAM_SEEK_CURRENT );
				break;
			}
		}

		outbuffer[cursize] = 0;
	}

	if( outbuffer == buffer )
	{
		if( cursize == 0 )
			return 0;
		outbuffer = memory_allocate( 0, cursize + 1, 0, MEMORY_PERSISTENT );
		memcpy( outbuffer, buffer, cursize );
		outbuffer[cursize] = 0;
	}

	return outbuffer;
}


uint64_t stream_read_string_buffer( stream_t* stream, char* outbuffer, uint64_t size )
{
	char buffer[128];
	int cursize = 0;
	int read, i;
	bool binary = stream_is_binary( stream );

	FOUNDATION_ASSERT( stream );
	if( !( stream->mode & STREAM_IN ) || !size )
		return 0;

	FOUNDATION_ASSERT( stream->vtable->read );

	--size;

	if( stream_is_sequential( stream ) )
	{
		//Single byte reading since we can't seek backwards (and don't want to block on network sockets)
		char c;
		if( !binary )
		{
			//Consume whitespace
			while( !stream_eos( stream ) )
			{
				read = (int)stream->vtable->read( stream, &c, 1 );
				if( !read )
					break;
				if( ( c != ' ' ) && ( c != '\n' ) && ( c != '\r' ) && ( c != '\t' ) )
				{
					outbuffer[cursize++] = c;
					break;
				}
			}
		}

		if( cursize > 0 )
		{
			while( !stream_eos( stream ) && ( cursize < (int)size ) )
			{
				read = (int)stream->vtable->read( stream, &c, 1 );
				if( !read )
					break;
				if( !c )
					break;
				if( !binary && ( ( c == ' ' ) || ( c == '\n' ) || ( c == '\r' ) || ( c == '\t' ) ) )
					break;
				outbuffer[cursize++] = c;
			}
		}
	}
	else
	{
		if( !binary )
		{
			//Consume whitespace
			while( !stream_eos( stream ) )
			{
				read = (int)stream->vtable->read( stream, buffer, 16 );
				if( !read )
					break;
				for( i = 0; i < read; ++i )
				{
					char c = buffer[i];
					if( ( c != ' ' ) && ( c != '\n' ) && ( c != '\r' ) && ( c != '\t' ) )
						break;
				}
				if( i < read )
				{
					stream_seek( stream, i - read, FL_STREAM_SEEK_CURRENT );
					break;
				}
			}
		}

		while( !stream_eos( stream ) && ( cursize < (int)size ) )
		{
			read = (int)stream->vtable->read( stream, buffer, 128 );
			if( !read )
				break;
			for( i = 0; i < read; ++i )
			{
				char c = buffer[i];
				if( !c )
					break;
				if( !binary && ( ( c == ' ' ) || ( c == '\n' ) || ( c == '\r' ) || ( c == '\t' ) ) )
					break;
			}
			if( !i )
				break;
			if( cursize + i > (int)size )
				i = (int)size - cursize;
			memcpy( outbuffer + cursize, buffer, i );
			cursize += i;
			if( i < 128 )
			{
				if( ( i + 1 ) < read )
					stream_seek( stream, 1 + i - read, FL_STREAM_SEEK_CURRENT );
				break;
			}
		}
	}

	if( cursize < (int)size )
		outbuffer[cursize] = 0;

	return cursize;
}


void stream_buffer_read( stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	if( stream->vtable->buffer_read )
		stream->vtable->buffer_read( stream );
}


unsigned int stream_available_read( stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	if( stream->vtable->available_read )
		return (unsigned int)stream->vtable->available_read( stream );
	return (unsigned int)( stream_size( stream ) - stream_tell( stream ) );
}


uint128_t stream_md5( stream_t* stream )
{
	int64_t cur, ic, lastc, num;
	md5_t md5;
	uint128_t ret = uint128_null();
	unsigned char buf[1025];
	bool ignore_lf = false;

	FOUNDATION_ASSERT( stream );
	if( stream->vtable->md5 )
		return stream->vtable->md5( stream );

	if( !stream || stream_is_sequential( stream ) || !( stream->mode & STREAM_IN ) )
		return ret;

	FOUNDATION_ASSERT( stream->vtable->read );

	cur = stream_tell( stream );
	stream_seek( stream, 0, FL_STREAM_SEEK_BEGIN );

	md5_initialize( &md5 );
	buf[1024] = 0;

	while( !stream_eos( stream ) )
	{
		num = (int64_t)stream->vtable->read( stream, buf, 1024 );
		if( !num )
			continue;
		if( stream->mode & STREAM_BINARY )
			md5_digest_raw( &md5, buf, (size_t)num );
		else
		{
			//If last buffer ended with CR, ignore a leading LF
			lastc = 0;
			if( ignore_lf && ( buf[0] == '\n' ) )
				lastc = 1;
			ignore_lf = false;

			//Digest one line at a time
			//Treat all line endings (LF, CR, CR+LF) as Unix style LF. If file has mixed line endings
			//(for example, one line ending in a single CR and next is empty and ending in a single LF),
			//it will not work!
			for( ic = lastc; ic < num; ++ic )
			{
				bool was_cr = ( buf[ic] == '\r' );
				bool was_lf = ( buf[ic] == '\n' );
				if( was_cr || was_lf )
				{
					if( was_cr && ( ic >= 1023 ) )
						ignore_lf = true; //Make next buffer ignore leading LF as it is part of CR+LF
					buf[ic] = '\n';
					md5_digest_raw( &md5, buf + lastc, (size_t)( ic - lastc + 1 ) ); //Include the LF
					if( was_cr && ( buf[ic+1] == '\n' ) ) //Check for CR+LF
						++ic;
					lastc = ic + 1;
				}
			}
			if( lastc < num )
				md5_digest_raw( &md5, buf + lastc, (size_t)( num - lastc ) );
		}
	}

	stream_seek( stream, cur, FL_STREAM_SEEK_BEGIN );

	md5_digest_finalize( &md5 );
	ret = md5_get_digest_raw( &md5 );

	md5_finalize( &md5 );

	return ret;
}


uint64_t stream_write( stream_t* stream, const void* buffer, uint64_t num_bytes )
{
	FOUNDATION_ASSERT( stream );
	if( !( stream->mode & STREAM_OUT ) )
		return 0;

	FOUNDATION_ASSERT( stream->vtable->write );

	return stream->vtable->write( stream, buffer, num_bytes );
}


void stream_write_bool( stream_t* stream, bool data )
{
	if( stream_is_binary( stream ) )
	{
		char c = ( data ? 1 : 0 );
		stream_write( stream, &c, 1 );
	}
	else
	{
		if( data )
			stream_write( stream, "true", 4 );
		else
			stream_write( stream, "false", 5 );
	}
}


void stream_write_int8( stream_t* stream, int8_t data )
{
	if( stream_is_binary( stream ) )
		stream_write( stream, &data, 1 );
	else
		stream_write_string( stream, string_from_int_static( (int32_t)data, 0, 0 ) );
}


void stream_write_uint8( stream_t* stream, uint8_t data )
{
	if( stream_is_binary( stream ) )
		stream_write( stream, &data, 1 );
	else
		stream_write_string( stream, string_from_uint_static( (uint32_t)data, false, 0, 0 ) );
}


void stream_write_int16( stream_t* stream, int16_t data )
{
	if( stream_is_binary( stream ) )
	{
		if( stream && stream->swap )
			data = byteorder_swap16( data );
		stream_write( stream, &data, 2 );
	}
	else
		stream_write_string( stream, string_from_int_static( data, 0, 0 ) );
}


void stream_write_uint16( stream_t* stream, uint16_t data )
{
	if( stream_is_binary( stream ) )
	{
		if( stream && stream->swap )
			data = byteorder_swap16( data );
		stream_write( stream, &data, 2 );
	}
	else
		stream_write_string( stream, string_from_uint_static( data, false, 0, 0 ) );
}


void stream_write_int32( stream_t* stream, int32_t data )
{
	if( stream_is_binary( stream ) )
	{
		if( stream && stream->swap )
			data = byteorder_swap32( data );
		stream_write( stream, &data, 4 );
	}
	else
		stream_write_string( stream, string_from_int_static( data, 0, 0 ) );
}


void stream_write_uint32( stream_t* stream, uint32_t data )
{
	if( stream_is_binary( stream ) )
	{
		if( stream && stream->swap )
			data = byteorder_swap32( data );
		stream_write( stream, &data, 4 );
	}
	else
		stream_write_string( stream, string_from_uint_static( data, false, 0, 0 ) );
}


void stream_write_int64( stream_t* stream, int64_t data )
{
	if( stream_is_binary( stream ) )
	{
		if( stream && stream->swap )
			data = byteorder_swap64( data );
		stream_write( stream, &data, 8 );
	}
	else
		stream_write_string( stream, string_from_int_static( data, 0, 0 ) );
}


void stream_write_uint64( stream_t* stream, uint64_t data )
{
	if( stream_is_binary( stream ) )
	{
		if( stream && stream->swap )
			data = byteorder_swap64( data );
		stream_write( stream, &data, 8 );
	}
	else
		stream_write_string( stream, string_from_uint_static( data, false, 0, 0 ) );
}


void stream_write_float32( stream_t* stream, float32_t data )
{
	if( stream_is_binary( stream ) )
	{
		if( stream && stream->swap )
		{
			float32_cast_t cast;
			cast.fval = data;
			cast.ival = byteorder_swap32( cast.ival );
			stream_write( stream, &cast.ival, 4 );
		}
		else
		{
			stream_write( stream, &data, 4 );
		}
	}
	else
		stream_write_string( stream, string_from_real_static( data, 0, 0, 0 ) );
}


void stream_write_float64( stream_t* stream, float64_t data )
{
	if( stream_is_binary( stream ) )
	{
		if( stream && stream->swap )
		{
			float64_cast_t cast;
			cast.fval = data;
			cast.ival = byteorder_swap64( cast.ival );
			stream_write( stream, &cast.ival, 8 );
		}
		else
		{
			stream_write( stream, &data, 8 );
		}
	}
	else
		stream_write_string( stream, string_from_real_static( (real)data, 0, 0, 0 ) );
}


void stream_write_string( stream_t* stream, const char* str )
{
	if( str )
		stream_write( stream, str, string_length( str ) + ( stream_is_binary( stream ) ? 1 : 0 ) );
	else if( stream_is_binary( stream ) )
		stream_write( stream, &str, 1 ); //Points to null, so safe to use as ref value for null terminator char
}


void stream_write_endl( stream_t* stream )
{
	if( !stream_is_binary( stream ) )
		stream_write( stream, "\n", 1 );
	stream_flush( stream );
}


void stream_write_format( stream_t* stream, const char* format, ... )
{
	va_list list;
	char* buffer;

	FOUNDATION_ASSERT( format );

	va_start( list, format );
	buffer = string_vformat( format, list );
	va_end( list );

	stream_write_string( stream, buffer );
	string_deallocate( buffer );
}


void stream_truncate( stream_t* stream, uint64_t size )
{
	FOUNDATION_ASSERT( stream );
	if( stream && stream->vtable->truncate )
		stream->vtable->truncate( stream, size );
}


void stream_flush( stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	if( stream && stream->vtable->flush )
		stream->vtable->flush( stream );
}


#include <stdio.h>

struct stream_std_t
{
	FOUNDATION_DECLARE_STREAM;
	void*              std;
	bool               eos;
};
typedef FOUNDATION_ALIGN(8) struct stream_std_t stream_std_t;


static uint64_t  _stream_stdin_read( stream_t*, void*, uint64_t );
static uint64_t  _stream_stdout_write( stream_t*, const void*, uint64_t );
static void      _stream_stdout_flush( stream_t* );
static stream_t* _stream_std_clone( stream_t* );
static bool      _stream_stdin_eos( stream_t* );
static uint64_t  _stream_stdin_available_read( stream_t* stream );
static uint64_t  _stream_std_last_modified( const stream_t* stream );

static stream_vtable_t _stream_stdout_vtable = {
	0,
	_stream_stdout_write,
	0,
	_stream_stdout_flush,
	0,
	0,
	0,
	0,
	_stream_std_last_modified,
	0,
	0,
	0,
	0,
	_stream_std_clone
};


static stream_vtable_t _stream_stdin_vtable = {
	_stream_stdin_read,
	0,
	_stream_stdin_eos,
	0,
	0,
	0,
	0,
	0,
	_stream_std_last_modified,
	0,
	0,
	_stream_stdin_available_read,
	0,
	_stream_std_clone
};


stream_t* stream_open_stdout( void )
{
	stream_std_t* stream = memory_allocate( HASH_STREAM, sizeof( stream_std_t ), 8, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	stream_initialize( (stream_t*)stream, system_byteorder() );
	stream->sequential = 1;
	stream->mode = STREAM_OUT;
	stream->type = STREAMTYPE_STDSTREAM;
	stream->vtable = &_stream_stdout_vtable;
	stream->path = string_clone( "stdout://" );
	stream->std = stdout;
	return (stream_t*)stream;
}


stream_t* stream_open_stderr( void )
{
	stream_std_t* stream = memory_allocate( HASH_STREAM, sizeof( stream_std_t ), 8, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	stream_initialize( (stream_t*)stream, system_byteorder() );
	stream->sequential = 1;
	stream->mode = STREAM_OUT;
	stream->type = STREAMTYPE_STDSTREAM;
	stream->vtable = &_stream_stdout_vtable;
	stream->path = string_clone( "stderr://" );
	stream->std = stderr;
	return (stream_t*)stream;
}


stream_t* stream_open_stdin( void )
{
	stream_std_t* stream = memory_allocate( HASH_STREAM, sizeof( stream_std_t ), 8, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	stream_initialize( (stream_t*)stream, system_byteorder() );
	stream->sequential = 1;
	stream->mode = STREAM_IN;
	stream->type = STREAMTYPE_STDSTREAM;
	stream->vtable = &_stream_stdin_vtable;
	stream->path = string_clone( "stdin://" );
	stream->std = stdin;
	return (stream_t*)stream;
}


static uint64_t _stream_stdin_read( stream_t* stream, void* buffer, uint64_t size )
{
	stream_std_t* stdstream = (stream_std_t*)stream;
	FILE* stdfile = (FILE*)stdstream->std;
	char* bytebuffer = (char*)buffer;
	uint64_t read = 0;

	stdstream->eos = false;

	while( read < size )
	{
		int c = getc( stdfile );
		if( c == EOF )
		{
			stdstream->eos = true;
			break;
		}
		bytebuffer[read++] = (char)c;
	}

	return read;
}


static uint64_t _stream_stdout_write( stream_t* stream, const void* buffer, uint64_t size )
{
	stream_std_t* stdstream = (stream_std_t*)stream;
	uint64_t was_written = fwrite( buffer, 1, (size_t)size, stdstream->std );
	return was_written;
}


static void _stream_stdout_flush( stream_t* stream )
{
	fflush( ((stream_std_t*)stream)->std );
}


static stream_t* _stream_std_clone( stream_t* stream )
{
	stream_std_t* clone = memory_allocate( HASH_STREAM, sizeof( stream_std_t ), 8, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	memcpy( clone, stream, sizeof( stream_std_t ) );
	clone->path = string_clone( stream->path );
	return (stream_t*)clone;
}


static bool _stream_stdin_eos( stream_t* stream )
{
	return ((stream_std_t*)stream)->eos;
}


static uint64_t _stream_stdin_available_read( stream_t* stream )
{
#if FOUNDATION_PLATFORM_WINDOWS

	HANDLE in_handle;
	DWORD size;

	in_handle = GetStdHandle( STD_INPUT_HANDLE );
	size = GetFileSize( in_handle, 0 );
	if( size > 0 )
		return size;

#elif FOUNDATION_PLATFORM_POSIX

	fd_set fds;
	struct timeval timeout;
	int res;

	memset( &timeout, 0, sizeof( timeout ) );
	FD_ZERO( &fds );
	FD_SET( STDIN_FILENO, &fds );

	res = select( STDIN_FILENO + 1, &fds, 0, 0, &timeout );
	if( ( res > 0 ) && FD_ISSET( STDIN_FILENO, &fds ) )
	{
		struct stat buf;
		if( fstat( STDIN_FILENO, &buf ) == 0 )
			return (uint64_t)buf.st_size;
		return 1;
	}

#endif

	FOUNDATION_UNUSED( stream );
	return 0;
}


static uint64_t _stream_std_last_modified( const stream_t* stream )
{
	FOUNDATION_ASSERT( stream );
	return time_system();
}
