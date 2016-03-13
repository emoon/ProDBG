/* build.h  -  Foundation library build setup  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


#ifdef FOUNDATION_PLATFORM_DOXYGEN
#  define BUILD_DEBUG                         1
#  define BUILD_RELEASE                       1
#  define BUILD_PROFILE                       1
#  define BUILD_DEPLOY                        1
#  define BUILD_ENABLE_ASSERT                 1
#  define BUILD_ENABLE_ERROR_CONTEXT          1
#  define BUILD_ENABLE_LOG                    1
#  define BUILD_ENABLE_DEBUG_LOG              1
#  define BUILD_ENABLE_CONFIG_DEBUG           1
#  define BUILD_ENABLE_PROFILE                1
#  define BUILD_ENABLE_MEMORY_CONTEXT         1
#  define BUILD_ENABLE_MEMORY_TRACKER         1
#  define BUILD_ENABLE_MEMORY_GUARD           1
#  define BUILD_ENABLE_STATIC_HASH_DEBUG      1
#endif

#ifndef BUILD_DEBUG
#  define BUILD_DEBUG                         0
#endif

#ifndef BUILD_RELEASE
#  define BUILD_RELEASE                       0
#endif

#ifndef BUILD_PROFILE
#  define BUILD_PROFILE                       0
#endif

#ifndef BUILD_DEPLOY
#  define BUILD_DEPLOY                        0
#endif

//Fallback
#if !BUILD_DEBUG && !BUILD_RELEASE && !BUILD_PROFILE && !BUILD_DEPLOY
#  if defined( NDEBUG )
#    undef  BUILD_RELEASE
#    define BUILD_RELEASE                     1
#  else
#    undef  BUILD_DEBUG
#    define BUILD_DEBUG                       1
#  endif
#endif


// Configurable choises
#ifndef BUILD_ENABLE_ASSERT
#if BUILD_DEBUG || BUILD_RELEASE
#define BUILD_ENABLE_ASSERT                   1
#else
#define BUILD_ENABLE_ASSERT                   0
#endif
#endif

#ifndef BUILD_ENABLE_ERROR_CONTEXT
#if BUILD_DEBUG || BUILD_RELEASE
#define BUILD_ENABLE_ERROR_CONTEXT            1
#else
#define BUILD_ENABLE_ERROR_CONTEXT            0
#endif
#endif

#ifndef BUILD_ENABLE_LOG
#if BUILD_DEBUG || BUILD_RELEASE
#define BUILD_ENABLE_LOG                      1
#else
#define BUILD_ENABLE_LOG                      0
#endif
#endif

#ifndef BUILD_ENABLE_DEBUG_LOG
#if BUILD_DEBUG
#define BUILD_ENABLE_DEBUG_LOG                1
#else
#define BUILD_ENABLE_DEBUG_LOG                0
#endif
#endif

#define BUILD_ENABLE_CONFIG_DEBUG             0

#ifndef BUILD_ENABLE_PROFILE
#if BUILD_DEBUG || BUILD_RELEASE || BUILD_PROFILE
#define BUILD_ENABLE_PROFILE                  1
#else
#define BUILD_ENABLE_PROFILE                  0
#endif
#endif

#ifndef BUILD_ENABLE_MEMORY_CONTEXT
#if BUILD_DEBUG || BUILD_RELEASE
#define BUILD_ENABLE_MEMORY_CONTEXT           1
#else
#define BUILD_ENABLE_MEMORY_CONTEXT           0
#endif
#endif

#ifndef BUILD_ENABLE_MEMORY_TRACKER
#if BUILD_DEBUG || BUILD_RELEASE
#define BUILD_ENABLE_MEMORY_TRACKER           1
#else
#define BUILD_ENABLE_MEMORY_TRACKER           0
#endif
#endif

#ifndef BUILD_ENABLE_MEMORY_GUARD
#if BUILD_DEBUG || BUILD_RELEASE
#define BUILD_ENABLE_MEMORY_GUARD             1
#else
#define BUILD_ENABLE_MEMORY_GUARD             0
#endif
#endif

#ifndef BUILD_ENABLE_STATIC_HASH_DEBUG
#if ( BUILD_DEBUG || BUILD_RELEASE ) && FOUNDATION_PLATFORM_FAMILY_DESKTOP
#define BUILD_ENABLE_STATIC_HASH_DEBUG        1
#else
#define BUILD_ENABLE_STATIC_HASH_DEBUG        0
#endif
#endif

#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID || FOUNDATION_PLATFORM_PNACL || FOUNDATION_PLATFORM_TIZEN
#  undef  BUILD_MONOLITHIC
#  define BUILD_MONOLITHIC 1
#elif !defined( BUILD_MONOLITHIC )
#  define BUILD_MONOLITHIC 0
#endif


#define BUILD_DEFAULT_STREAM_BYTEORDER        BYTEORDER_LITTLEENDIAN


#define BUILD_SIZE_THREAD_MAP                      512
#define BUILD_SIZE_LIBRARY_MAP                     64

#define BUILD_SIZE_TEMPORARY_MEMORY                ( 2 * 1024 * 1024 )

#define BUILD_SIZE_EVENT_BLOCK_LIMIT               ( 1 * 1024 * 1024 )

#define BUILD_SIZE_DEFAULT_THREAD_STACK            0x8000

#define BUILD_SIZE_ERROR_CONTEXT_DEPTH             32

#define BUILD_SIZE_MEMORY_CONTEXT_DEPTH            32

#define BUILD_SIZE_MEMORY_TRACKER_MAX_ALLOCATIONS  ( 32 * 1024 )

#define BUILD_SIZE_STACKTRACE_DEPTH                32

#define BUILD_SIZE_FS_MONITORS                     32

#define BUILD_SIZE_STATIC_HASH_STORE               4192

#define BUILD_SIZE_EVENT_BLOCK_CHUNK               ( 32 * 1024 )


//Define appropriate "standard" macros
#if BUILD_DEBUG
#  undef DEBUG
#  undef _DEBUG
#  undef NDEBUG
#  define _DEBUG 1
#else
#  undef DEBUG
#  undef _DEBUG
#  undef NDEBUG
#  define NDEBUG 1
#endif

