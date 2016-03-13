/* assert.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <stdio.h>
#include <stdarg.h>

#if FOUNDATION_PLATFORM_WINDOWS
#  define snprintf( p, s, ... ) _snprintf_s( p, s, _TRUNCATE, __VA_ARGS__ )
#endif


#define ASSERT_BUFFER_SIZE             2048U
#define ASSERT_STACKTRACE_MAX_DEPTH    128U
#define ASSERT_STACKTRACE_SKIP_FRAMES  1U

static assert_handler_fn _assert_handler;

static char              _assert_buffer[ASSERT_BUFFER_SIZE];
#if BUILD_ENABLE_ASSERT
static char              _assert_context_buffer[ASSERT_BUFFER_SIZE];
static char              _assert_box_buffer[ASSERT_BUFFER_SIZE];
static char              _assert_stacktrace_buffer[ASSERT_BUFFER_SIZE];
static void*             _assert_stacktrace[ASSERT_STACKTRACE_MAX_DEPTH];
#endif


assert_handler_fn assert_handler( void )
{
	return _assert_handler;
}


void assert_set_handler( assert_handler_fn new_handler )
{
	_assert_handler = new_handler;
}


int assert_report( uint64_t context, const char* condition, const char* file, int line, const char* msg )
{
	static const char nocondition[] = "<Static fail>";
	static const char nofile[] = "<No file>";
	static const char nomsg[] = "<No message>";
	static const char assert_format[] = "****** ASSERT FAILED ******\nCondition: %s\nFile/line: %s : %d\n%s%s\n%s\n";

	if( !condition ) condition = nocondition;
	if( !file      ) file      = nofile;
	if( !msg       ) msg       = nomsg;

	if( _assert_handler && ( _assert_handler != assert_report ) )
		return (*_assert_handler)( context, condition, file, line, msg );

#if BUILD_ENABLE_ASSERT
	_assert_context_buffer[0] = 0;
	error_context_buffer( _assert_context_buffer, ASSERT_BUFFER_SIZE );

	_assert_stacktrace_buffer[0] = 0;
	if( foundation_is_initialized() )
	{
		unsigned int num_frames = stacktrace_capture( _assert_stacktrace, ASSERT_STACKTRACE_MAX_DEPTH, ASSERT_STACKTRACE_SKIP_FRAMES );
		if( num_frames )
		{
			//TODO: Resolve directly into buffer to avoid memory allocations in assert handler
			char* trace = stacktrace_resolve( _assert_stacktrace, num_frames, 0U );
			string_copy( _assert_stacktrace_buffer, trace, ASSERT_BUFFER_SIZE );
			string_deallocate( trace );
		}
	}
	else
	{
		string_copy( _assert_stacktrace_buffer, "<no stacktrace - not initialized>", ASSERT_BUFFER_SIZE );
	}

	snprintf( _assert_box_buffer, (size_t)ASSERT_BUFFER_SIZE, assert_format, condition, file, line, _assert_context_buffer, msg, _assert_stacktrace_buffer );

	log_errorf( context, ERROR_ASSERT, "%s", _assert_box_buffer );

	system_message_box( "Assert Failure", _assert_box_buffer, false );
#else
	log_errorf( context, ERROR_ASSERT, assert_format, condition, file, line, "", msg, "" );
#endif

	return 1;
}


int assert_report_formatted( uint64_t context, const char* condition, const char* file, int line, const char* msg, ... )
{
	if( msg )
	{
		/*lint --e{438} Lint gets confused about assignment to ap */
		va_list ap;
		va_start( ap, msg );
		vsnprintf( _assert_buffer, (size_t)ASSERT_BUFFER_SIZE, msg, ap );
		va_end( ap );
	}
	else
	{
		_assert_buffer[0] = 0;
	}
	return assert_report( context, condition, file, line, _assert_buffer );
}
