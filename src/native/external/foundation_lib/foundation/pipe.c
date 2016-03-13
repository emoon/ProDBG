/* pipe.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#include <foundation/windows.h>
#elif FOUNDATION_PLATFORM_POSIX
#include <foundation/posix.h>
#elif FOUNDATION_PLATFORM_PNACL
#include <foundation/pnacl.h>
#endif


static stream_vtable_t _pipe_stream_vtable;


stream_t* pipe_allocate( void )
{
	stream_pipe_t* pipestream = memory_allocate( HASH_STREAM, sizeof( stream_pipe_t ), 8, MEMORY_PERSISTENT );

	pipe_initialize( pipestream );

	return (stream_t*)pipestream;
}


void pipe_initialize( stream_pipe_t* pipestream )
{
	stream_t* stream = (stream_t*)pipestream;

	memset( stream, 0, sizeof( stream_pipe_t ) );

	stream_initialize( stream, system_byteorder() );

	pipestream->type = STREAMTYPE_PIPE;
	pipestream->path = string_format( "pipe://0x" PRIfixPTR, pipestream );
	pipestream->mode = STREAM_OUT | STREAM_IN | STREAM_BINARY;
	pipestream->sequential = true;

#if FOUNDATION_PLATFORM_WINDOWS
	{
		//Inheritable by default so process can use for stdstreams
		SECURITY_ATTRIBUTES security_attribs = {0};
		security_attribs.nLength = sizeof( SECURITY_ATTRIBUTES );
		security_attribs.bInheritHandle = TRUE;
		security_attribs.lpSecurityDescriptor = 0;

		if( !CreatePipe( &pipestream->handle_read, &pipestream->handle_write, &security_attribs, 0 ) )
			log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to create unnamed pipe: %s", system_error_message( GetLastError() ) );
	}
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	int fds[2] = { 0, 0 };
	if( pipe( fds ) < 0 )
		log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to create unnamed pipe: %s", system_error_message( 0 ) );
	pipestream->fd_read = fds[0];
	pipestream->fd_write = fds[1];
#endif

	pipestream->vtable = &_pipe_stream_vtable;
}


static void _pipe_finalize( stream_t* stream )
{
	stream_pipe_t* pipe = (stream_pipe_t*)stream;

	if( !pipe || ( stream->type != STREAMTYPE_PIPE ) )
		return;

#if FOUNDATION_PLATFORM_WINDOWS
	if( pipe->handle_read )
		CloseHandle( pipe->handle_read );
	pipe->handle_read = 0;

	if( pipe->handle_write )
		CloseHandle( pipe->handle_write );
	pipe->handle_write = 0;
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	if( pipe->fd_read )
		close( pipe->fd_read );
	pipe->fd_read = 0;

	if( pipe->fd_write )
		close( pipe->fd_write );
	pipe->fd_write = 0;
#endif
}


void pipe_close_read( stream_t* stream )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
	FOUNDATION_ASSERT( stream->type == STREAMTYPE_PIPE );

#if FOUNDATION_PLATFORM_WINDOWS
	if( pipestream->handle_read )
	{
		CloseHandle( pipestream->handle_read );
		pipestream->handle_read = 0;
	}
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	if( pipestream->fd_read )
	{
		close( pipestream->fd_read );
		pipestream->fd_read = 0;
	}
#endif

	pipestream->mode &= ~STREAM_IN;
}


void pipe_close_write( stream_t* stream )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
	FOUNDATION_ASSERT( stream->type == STREAMTYPE_PIPE );

#if FOUNDATION_PLATFORM_WINDOWS
	if( pipestream->handle_write )
	{
		CloseHandle( pipestream->handle_write );
		pipestream->handle_write = 0;
	}
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	if( pipestream->fd_write )
	{
		close( pipestream->fd_write );
		pipestream->fd_write = 0;
	}
#endif

	pipestream->mode &= ~STREAM_OUT;
}


#if FOUNDATION_PLATFORM_WINDOWS


void* pipe_read_handle( stream_t* stream )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
	return stream && ( stream->type == STREAMTYPE_PIPE ) ? pipestream->handle_read : 0;
}


void* pipe_write_handle( stream_t* stream )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
	return stream && ( stream->type == STREAMTYPE_PIPE ) ? pipestream->handle_write : 0;
}


#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL


int pipe_read_fd( stream_t* stream )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
	return stream && ( stream->type == STREAMTYPE_PIPE ) ? pipestream->fd_read : 0;
}


int pipe_write_fd( stream_t* stream )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
	return stream && ( stream->type == STREAMTYPE_PIPE ) ? pipestream->fd_write : 0;
}


#endif


static uint64_t _pipe_stream_read( stream_t* stream, void* dest, uint64_t num )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
	FOUNDATION_ASSERT( stream->type == STREAMTYPE_PIPE );
#if FOUNDATION_PLATFORM_WINDOWS
	if( pipestream->handle_read && ( ( pipestream->mode & STREAM_IN ) != 0 ) )
	{
		uint64_t total_read = 0;
		do
		{
			unsigned long num_read = 0;
			if( !ReadFile( pipestream->handle_read, pointer_offset( dest, total_read ), (unsigned int)( num - total_read ), &num_read, 0 ) )
			{
				int err = GetLastError();
				if( err == ERROR_BROKEN_PIPE )
				{
					pipestream->eos = true;
					break;
				}
				log_warnf( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to read from pipe: %s (%d)", system_error_message( err ), err );
			}
			else
			{
				total_read += num_read;
			}
		} while( total_read < num );
		return total_read;
	}
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	if( pipestream->fd_read && ( ( pipestream->mode & STREAM_IN ) != 0 ) )
	{
		uint64_t total_read = 0;
		do
		{
			ssize_t num_read = read( pipestream->fd_read, pointer_offset( dest, total_read ), (size_t)( num - total_read ) );
			if( num_read <= 0 )
			{
				pipestream->eos = true;
				break;
			}
			total_read += num_read;
		} while( total_read < num );
		return total_read;
	}
#endif

	return 0;
}


static uint64_t _pipe_stream_write( stream_t* stream, const void* source, uint64_t num )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
	FOUNDATION_ASSERT( stream->type == STREAMTYPE_PIPE );
#if FOUNDATION_PLATFORM_WINDOWS
	if( pipestream->handle_write && ( ( pipestream->mode & STREAM_OUT ) != 0 ) )
	{
		uint64_t total_written = 0;
		do
		{
			unsigned long num_written = 0;
			if( !WriteFile( pipestream->handle_write, pointer_offset_const( source, total_written ), (unsigned int)( num - total_written ), &num_written, 0 ) )
			{
				pipestream->eos = true;
				break;
			}
			total_written += num_written;
		} while( total_written < num );
		return total_written;
	}
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	if( pipestream->fd_write && ( ( pipestream->mode & STREAM_OUT ) != 0 ) )
	{
		uint64_t total_written = 0;
		do
		{
			ssize_t num_written = write( pipestream->fd_write, pointer_offset_const( source, total_written ), (size_t)( num - total_written ) );
			if( num_written <= 0 )
			{
				pipestream->eos = true;
				break;
			}
			total_written += num_written;
		} while( total_written < num );
		return total_written;
	}
#endif

	return 0;
}


static bool _pipe_stream_eos( stream_t* stream )
{
	stream_pipe_t* pipestream = (stream_pipe_t*)stream;
#if FOUNDATION_PLATFORM_WINDOWS
	return !stream || ( !pipestream->handle_read && !pipestream->handle_write ) || pipestream->eos;
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	return !stream || ( !pipestream->fd_read && !pipestream->fd_write ) || pipestream->eos;
#else
	return !stream || pipestream->eos;
#endif
}


static void _pipe_stream_flush( stream_t* stream )
{
	FOUNDATION_UNUSED( stream );
}


static void _pipe_stream_truncate( stream_t* stream, uint64_t size )
{
	FOUNDATION_UNUSED( stream );
	FOUNDATION_UNUSED( size );
}


static uint64_t _pipe_stream_size( stream_t* stream )
{
	FOUNDATION_UNUSED( stream );
	return 0;
}


static void _pipe_stream_seek( stream_t* stream, int64_t offset, stream_seek_mode_t direction )
{
	FOUNDATION_UNUSED( stream );
	FOUNDATION_UNUSED( offset );
	FOUNDATION_UNUSED( direction );
}


static int64_t _pipe_stream_tell( stream_t* stream )
{
	FOUNDATION_UNUSED( stream );
	return 0;
}


static uint64_t _pipe_stream_lastmod( const stream_t* stream )
{
	FOUNDATION_UNUSED( stream );
	return time_current();
}


static uint64_t _pipe_stream_available_read( stream_t* stream )
{
	FOUNDATION_UNUSED( stream );
	return 0;
}


void _pipe_stream_initialize( void )
{
	_pipe_stream_vtable.read = _pipe_stream_read;
	_pipe_stream_vtable.write = _pipe_stream_write;
	_pipe_stream_vtable.eos = _pipe_stream_eos;
	_pipe_stream_vtable.flush = _pipe_stream_flush;
	_pipe_stream_vtable.truncate = _pipe_stream_truncate;
	_pipe_stream_vtable.size = _pipe_stream_size;
	_pipe_stream_vtable.seek = _pipe_stream_seek;
	_pipe_stream_vtable.tell = _pipe_stream_tell;
	_pipe_stream_vtable.lastmod = _pipe_stream_lastmod;
	_pipe_stream_vtable.available_read = _pipe_stream_available_read;
	_pipe_stream_vtable.finalize = _pipe_finalize;
}
