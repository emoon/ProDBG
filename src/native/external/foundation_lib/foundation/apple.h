/* apple.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

//NOTE - The base of all header problems with XCode is that includes like
//       #include <Foundation/Foundation.h>
//       in system headers will actually map to our foundation/foundation.h since the preprocessor
//       seems to be case insensitive. Solution is to use this header which wraps the Cocoa includes
//       #include <foundation/foundation.h>
//       #include <foundation/apple.h>

#include <foundation/types.h>
#include <foundation/uuid.h>
#include <foundation/radixsort.h>
#include <foundation/semaphore.h>


#if FOUNDATION_PLATFORM_APPLE

#define __error_t_defined 1

#define semaphore_t __system_semaphore_t
#define radixsort __stdlib_radixsort
#define _UUID_T
#define uuid_generate_random __system_uuid_generate_random
#define uuid_generate_time __system_uuid_generate_time
#define uuid_is_null __system_uuid_is_null
#define semaphore_wait __system_semaphore_wait
#define semaphore_destroy __system_semaphore_destroy
#define thread_create __system_thread_create
#define thread_terminate __system_thread_terminate
#define task_t __system_task_t

#include <mach/mach_types.h>
#include <mach/mach_interface.h>

#undef semaphore_wait
#undef semaphore_destroy
#undef thread_create
#undef thread_terminate

#if FOUNDATION_COMPILER_CLANG
#pragma clang diagnostic push 
#pragma clang diagnostic ignored "-Wpedantic"         // error: #include_next is a language extension [-Werror,-Wpedantic]
#endif

#ifdef __OBJC__
#  import <CoreFoundation/CoreFoundation.h>
#  include_next <Foundation/Foundation.h>
#  if FOUNDATION_PLATFORM_MACOSX
#    import <AppKit/AppKit.h>
#  elif FOUNDATION_PLATFORM_IOS
#    import <UIKit/UIKit.h>
#    import <QuartzCore/QuartzCore.h>
#  endif
#else
#  include <CoreFoundation/CoreFoundation.h>
#  if FOUNDATION_PLATFORM_MACOSX
#    include <Carbon/Carbon.h>
#    include <ApplicationServices/ApplicationServices.h>
#  endif
#endif

#if FOUNDATION_COMPILER_CLANG
#pragma clang diagnostic pop 
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#undef radixsort
#undef semaphore_t
#undef _UUID_T
#undef uuid_generate_random
#undef uuid_generate_time
#undef uuid_is_null
#undef task_t

#endif
