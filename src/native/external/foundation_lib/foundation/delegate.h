/* delegate.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/apple.h>


#if FOUNDATION_PLATFORM_MACOSX || FOUNDATION_PLATFORM_IOS

FOUNDATION_API void delegate_start_main_ns_thread( void );
FOUNDATION_API void delegate_reference_classes( void );

#endif


#if FOUNDATION_PLATFORM_MACOSX

FOUNDATION_API void* delegate_nswindow( void );

#ifdef __OBJC__

@interface FoundationAppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, retain) IBOutlet NSWindow *window;
+ (void)referenceClass;
@end

#endif

#endif


#if FOUNDATION_PLATFORM_IOS

FOUNDATION_API void* delegate_uiwindow( void );

#ifdef __OBJC__

@interface FoundationAppDelegate : NSObject <UIApplicationDelegate>
@property (nonatomic, retain) IBOutlet UIWindow *window;
+ (void)referenceClass;
@end

@interface FoundationAlertViewDelegate : NSObject <UIAlertViewDelegate>
{
}
@end

#endif

#endif

