/* log.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#include <foundation/error.h>


FOUNDATION_API void              log_debugf( uint64_t context, const char* format, ... );
FOUNDATION_API void              log_infof( uint64_t context, const char* format, ... );
FOUNDATION_API void              log_warnf( uint64_t context, warning_t warn, const char* format, ... );
FOUNDATION_API void              log_errorf( uint64_t context, error_t err, const char* format, ... );
FOUNDATION_API void              log_panicf( uint64_t context, error_t err, const char* format, ... );

FOUNDATION_API void              log_error_context( uint64_t context, error_level_t error_level );
FOUNDATION_API log_callback_fn   log_callback( void );
FOUNDATION_API void              log_set_callback( log_callback_fn callback );
FOUNDATION_API void              log_enable_stdout( bool enable );
FOUNDATION_API void              log_enable_prefix( bool enable );
FOUNDATION_API void              log_set_suppress( uint64_t context, error_level_t level );
FOUNDATION_API error_level_t     log_suppress( uint64_t context );
FOUNDATION_API void              log_suppress_clear( void );


#if BUILD_ENABLE_LOG && BUILD_ENABLE_DEBUG_LOG

#  define log_debug( context, msg )             log_debugf( context, "%s", msg )

#else

#  define log_debug( context, msg ) /*lint -save -e717 */ do { (void)sizeof( context ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_debugf( context, msg, ... ) /*lint -save -e717 */ do { (void)sizeof( context ); (void)sizeof( msg ); } while(0) /*lint -restore */

#endif


#if BUILD_ENABLE_LOG

#  define log_info( context, msg )              log_infof( context, "%s", msg )
#  define log_warn( context, warn, msg )        log_warnf( context, warn, "%s", msg )
#  define log_error( context, err, msg )        log_errorf( context, err, "%s", msg )
#  define log_panic( context, err, msg )        log_panicf( context, err, "%s", msg )

#else

#  define log_info( context, msg ) /*lint -save -e717 */ do { (void)sizeof( context ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_infof( context, msg, ... ) /*lint -save -e717 */ do { (void)sizeof( context ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_warn( context, warn, msg ) /*lint -save -e717 */ do { (void)sizeof( context ); (void)sizeof( warn ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_warnf( context, warn, msg, ... ) /*lint -save -e717 */ do { (void)sizeof( context ); (void)sizeof( warn ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_error( context, err, msg ) /*lint -save -e717 */ do { error_report( ERRORLEVEL_ERROR, err ); (void)sizeof( context ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_errorf( context, err, msg, ... ) /*lint -save -e717 */ do { error_report( ERRORLEVEL_ERROR, err ); (void)sizeof( context ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_panic( context, err, msg ) /*lint -save -e717 */ do { error_report( ERRORLEVEL_PANIC, err ); (void)sizeof( context ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_panicf( context, err, msg, ... ) /*lint -save -e717 */ do { error_report( ERRORLEVEL_PANIC, err ); (void)sizeof( context ); (void)sizeof( msg ); } while(0) /*lint -restore */
#  define log_error_context( context, error_level ) /*lint -save -e717 */ do { (void)sizeof( context ); (void)sizeof( error_level ); } while(0) /*lint -restore */
#  define log_set_callback( callback ) /*lint -save -e717 */ do { (void)sizeof( (void*)callback ); } while(0) /*lint -restore */
#  define log_enable_stdout( enable ) /*lint -save -e717 */ do { (void)sizeof( enable ); } while(0) /*lint -restore */
#  define log_enable_prefix( enable ) /*lint -save -e717 */ do { (void)sizeof( enable ); } while(0) /*lint -restore */
#  define log_set_suppress( context, level ) /*lint -save -e717 */ do { (void)sizeof( context ); (void)sizeof( level ); } while(0) /*lint -restore */
#  define log_suppress( context ) ERRORLEVEL_NONE
#  define log_suppress_clear() do {} while(0)

#endif
