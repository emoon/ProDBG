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
#include <foundation/types.h>

//NOTE - The base of all header problems with XCode is that
//       #include <Foundation/Foundation.h>
//       in system headers will actually map to our foundation/foundation.h

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

#ifdef __OBJC__
#  import <CoreFoundation/CoreFoundation.h>
#  import <Foundation/NSObject.h>
#  import <Foundation/NSString.h>
#  import <Foundation/NSThread.h>
#  import <Foundation/NSProcessInfo.h>
#  import <Foundation/NSString.h>
#  import <Foundation/NSSet.h>
#  import <Foundation/NSArray.h>
#  import <Foundation/NSTimer.h>
#  import <Foundation/NSUndoManager.h>
#  if FOUNDATION_PLATFORM_MACOSX
#    import <Foundation/NSRunLoop.h>
#    import <Foundation/NSExtensionContext.h>
#    import <AppKit/NSApplication.h>
#    import <AppKit/NSAlert.h>
#    import <AppKit/NSWindow.h>
#    import <AppKit/NSViewController.h>
#  elif FOUNDATION_PLATFORM_IOS
#    import <Foundation/NSUUID.h>
#    import <Foundation/NSCoder.h>
#    import <Foundation/NSAttributedString.h>
#    import <Foundation/NSIndexPath.h>
#    import <Foundation/NSBundle.h>
#    import <Foundation/NSUserActivity.h>
#    import <Foundation/NSMapTable.h>
#    import <UIKit/UIApplication.h>
#    import <UIKit/UIWindow.h>
#    import <UIKit/UIScreen.h>
#    import <UIKit/UIView.h>
#    import <UIKit/UIViewController.h>
#    import <QuartzCore/QuartzCore.h>
#  endif
#else
#  include <CoreFoundation/CoreFoundation.h>
#  if FOUNDATION_PLATFORM_MACOSX
#    include <Carbon/Carbon.h>
#    include <ApplicationServices/ApplicationServices.h>
#  endif
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
