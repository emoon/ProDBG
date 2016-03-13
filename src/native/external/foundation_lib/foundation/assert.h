/* assert.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#pragma once

#include <foundation/platform.h>
#include <foundation/types.h>
#include <foundation/crash.h>


FOUNDATION_API assert_handler_fn    assert_handler( void );
FOUNDATION_API void                 assert_set_handler( assert_handler_fn new_handler );
FOUNDATION_API int                  assert_report( uint64_t context, const char* condition, const char* file, int line, const char* msg );
FOUNDATION_API int                  assert_report_formatted( uint64_t context, const char* condition, const char* file, int line, const char* msg, ... );

#if BUILD_ENABLE_ASSERT

#  define FOUNDATION_ASSERT( cond ) do { if( ( !(cond) ) && assert_report( 0ULL, #cond, __FILE__, __LINE__, 0 ) ) crash_debug_break(); } while(0)
#  define FOUNDATION_ASSERT_MSG( cond, msg ) do { if( ( !(cond) ) && assert_report_formatted( 0ULL, #cond, __FILE__, __LINE__, (msg) ) ) crash_debug_break(); } while(0)
#  define FOUNDATION_ASSERT_MSGFORMAT( cond, msg, ... ) do { if( ( !(cond) ) && assert_report_formatted( 0ULL, #cond, __FILE__, __LINE__, (msg), __VA_ARGS__ ) ) crash_debug_break(); } while(0)
#  define FOUNDATION_ASSERT_FAIL( msg ) do { if( assert_report( 0ULL, 0, __FILE__, __LINE__, (msg) ) ) crash_debug_break(); } while(0)
#  define FOUNDATION_ASSERT_FAIL_LOG( context, msg ) do { if( assert_report( context, 0, __FILE__, __LINE__, (msg) ) ) crash_debug_break(); (void)sizeof( context ); } while(0)
#  define FOUNDATION_ASSERT_FAILFORMAT( msg, ... ) do { if( assert_report_formatted( 0ULL, 0, __FILE__, __LINE__, (msg), __VA_ARGS__ ) ) crash_debug_break(); } while(0)
#  define FOUNDATION_ASSERT_FAILFORMAT_LOG( context, msg, ... ) do { if( assert_report_formatted( context, 0, __FILE__, __LINE__, (msg), __VA_ARGS__ ) ) crash_debug_break(); (void)sizeof( context ); } while(0)

#  define FOUNDATION_ASSERT_ALIGNMENT( addr, alignment ) do { FOUNDATION_ASSERT_MSG( ( (uintptr_t)(addr) % (uintptr_t)(alignment) ) == 0, "Mis-aligned memory" ); } while(0)
#  if FOUNDATION_ARCH_ARM || FOUNDATION_ARCH_ARM_64
#  define FOUNDATION_ASSERT_PLATFORM_ALIGNMENT( addr, alignment ) do { FOUNDATION_ASSERT_ALIGNMENT( addr, alignment ); } while(0)
#else
#  define FOUNDATION_ASSERT_PLATFORM_ALIGNMENT( addr, alignment ) do { (void)sizeof(addr); (void)sizeof( alignment ); } while(0)
#endif

#  define FOUNDATION_VALIDATE( cond ) ( ( !(cond) ) ? ( assert_report( 0ULL, #cond, __FILE__, __LINE__, 0 ) ? ( crash_debug_break(), false ) : false ) : true )
#  define FOUNDATION_VALIDATE_MSG( cond, msg ) ( ( !(cond) ) ? ( assert_report( 0ULL, #cond, __FILE__, __LINE__, (msg) ) ? ( crash_debug_break(), false ) : false ) : true )
#  define FOUNDATION_VALIDATE_MSGFORMAT( cond, msg, ... ) ( ( !(cond) ) ? ( assert_report_formatted( 0ULL, #cond, __FILE__, __LINE__, (msg), __VA_ARGS__ ) ? ( crash_debug_break(), false ) : false ) : true )

#else

#  define FOUNDATION_ASSERT( cond ) do { (void)sizeof( cond ); } while(0)
#  define FOUNDATION_ASSERT_MSG( cond, msg ) do { (void)sizeof( cond ); (void)sizeof( msg ); } while(0)
#  define FOUNDATION_ASSERT_MSGFORMAT( cond, msg, ... ) do { (void)sizeof( cond ); (void)sizeof( msg ); } while(0)
#  define FOUNDATION_ASSERT_FAIL( msg ) do { (void)sizeof( msg ); } while(0)
#  define FOUNDATION_ASSERT_FAIL_LOG( context, msg ) do { log_errorf( context, ERROR_ASSERT, "%s", msg ); } while(0)
#  define FOUNDATION_ASSERT_FAILFORMAT( msg, ... ) do { (void)sizeof( msg ); } while(0)
#  define FOUNDATION_ASSERT_FAILFORMAT_LOG( context, msg, ... ) do { log_errorf( context, ERROR_ASSERT, msg, __VA_ARGS__ ); } while(0)
#  define FOUNDATION_ASSERT_ALIGNMENT( addr, alignment ) do { (void)sizeof(addr); (void)sizeof( alignment ); } while(0)
#  define FOUNDATION_ASSERT_PLATFORM_ALIGNMENT( addr, alignment ) do { (void)sizeof(addr); (void)sizeof( alignment ); } while(0)
#  define FOUNDATION_VALIDATE( cond ) (cond)
#  define FOUNDATION_VALIDATE_MSG( cond, msg ) ( ( !(cond) ) ? ( (void)sizeof( msg ), false ) : true )
#  define FOUNDATION_VALIDATE_MSGFORMAT( cond, msg, ... ) ( ( !(cond) ) ? ( (void)sizeof( msg ), false ) : true )

#endif

#if FOUNDATION_COMPILER_CLANG || ( FOUNDATION_COMPILER_GCC && ( ( __GNUC__ > 4 ) || ( ( __GNUC__ == 4 ) && ( __GNUC_MINOR__ >= 6 ) ) ) )
#  define FOUNDATION_STATIC_ASSERT( cond, msg ) _Static_assert( cond, msg )
#elif FOUNDATION_COMPILER_MSVC && ( _MSC_VER > 1600 )
#  define FOUNDATION_STATIC_ASSERT( cond, msg ) static_assert( cond, msg )
#elif defined( __COUNTER__ )
#  define FOUNDATION_STATIC_ASSERT( cond, msg ) /*lint -e{506, 751, 778} */ typedef char FOUNDATION_PREPROCESSOR_JOIN( _static_assert_number_, __COUNTER__ )[(cond)?1:-1]
#else
#  define FOUNDATION_STATIC_ASSERT( cond, msg ) /*lint -e{506, 751, 778} */ typedef char FOUNDATION_PREPROCESSOR_JOIN( _static_assert_at_line_, __LINE__ )[(cond)?1:-1]
#endif

