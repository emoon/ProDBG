/* delegate.m  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#include <foundation/delegate.h>
#include <foundation/main.h>


extern int app_main( void* arg );

extern volatile int _delegate_dummy;
volatile int _delegate_dummy;

FOUNDATION_NOINLINE void delegate_reference_classes( void )
{
	_delegate_dummy = 1;
	[FoundationAppDelegate referenceClass];
}

@interface FoundationMainThread : NSObject
+ (void)startMainThread:(void*)arg;
@end

static volatile bool _delegate_main_thread_running = false;
static volatile bool _delegate_received_start = false;
static volatile bool _delegate_received_terminate = false;

@implementation FoundationMainThread

+ (void)startMainThread:(void*)arg
{
	FOUNDATION_UNUSED( arg );
	if( _delegate_main_thread_running )
		return;

	_delegate_main_thread_running = true;

	log_debug( 0, "Started main thread" );

    if( !_delegate_received_start )
    {
		log_debug( 0, "Waiting for application init" );
        while( !_delegate_received_start )
            thread_sleep( 50 );
        thread_sleep( 1 );
    }

	@autoreleasepool
	{
		log_debug( 0, "Application init done, launching main" );
		if( ![NSThread isMultiThreaded] )
			log_warn( 0, WARNING_SUSPICIOUS, "Application is STILL not multithreaded!" );
    }

	{
		char* name = 0;
		const application_t* app = environment_application();
		{
			const char* aname = app->short_name;
			name = string_clone( aname ? aname : "unknown" );
			name = string_append( name, "-" );
			name = string_append( name, string_from_version_static( app->version ) );
		}

		if( app->dump_callback )
			crash_guard_set( app->dump_callback, name );

		crash_guard( main_run, 0, app->dump_callback, name );

		string_deallocate( name );
	}

	main_shutdown();

	@autoreleasepool
	{
#if FOUNDATION_PLATFORM_MACOSX
		log_debug( 0, "Calling application terminate" );
		[NSApp terminate:nil];
#endif
		log_debug( 0, "Main thread exiting" );

		_delegate_main_thread_running = false;

#if FOUNDATION_PLATFORM_IOS
		if( !_delegate_received_terminate )
		{
			if( ( environment_application()->flags & APPLICATION_UTILITY ) == 0 )
				log_warnf( 0, WARNING_SUSPICIOUS, "Main loop terminated without applicationWillTerminate - force exit process" );
			exit( -1 );
		}
#endif
	}
}

@end

void delegate_start_main_ns_thread( void )
{
	delegate_reference_classes();

	log_debug( 0, "Starting main thread" );
	@autoreleasepool { [NSThread detachNewThreadSelector:@selector(startMainThread:) toTarget:[FoundationMainThread class] withObject:nil]; }
}


#if FOUNDATION_PLATFORM_MACOSX


static __weak NSApplication*           _delegate_app;
static __weak FoundationAppDelegate*   _delegate;


void* delegate_nswindow( void )
{
	return (__bridge void*)(_delegate ? [_delegate window] : 0);
}


@implementation FoundationAppDelegate

@synthesize window;


+ (void)referenceClass
{
	_delegate_dummy = 2;
}


- (void)applicationDidFinishLaunching:(NSApplication*)application
{
	FOUNDATION_UNUSED( application );
	_delegate = self;
    _delegate_received_start = true;

	log_debug( 0, "Application finished launching" );
	system_post_event( FOUNDATIONEVENT_START );
}

- (void)applicationWillResignActive:(NSApplication*)application
{
	FOUNDATION_UNUSED( application );
	log_debug( 0, "Application will resign active" );
	system_post_event( FOUNDATIONEVENT_PAUSE );
}

- (void)applicationDidBecomeActive:(NSApplication*)application
{
	FOUNDATION_UNUSED( application );
	log_debug( 0, "Application became active" );
	_delegate_app = application;
	system_post_event( FOUNDATIONEVENT_RESUME );
}

- (void)applicationWillTerminate:(NSApplication*)application
{
	FOUNDATION_UNUSED( application );
	_delegate_received_terminate = true;

	if( foundation_is_initialized() )
	{
		log_debug( 0, "Application will terminate" );
		system_post_event( FOUNDATIONEVENT_TERMINATE );
	}
}

- (void) dealloc
{
	log_debug( 0, "Application dealloc" );

	_delegate_app = 0;
	_delegate = 0;
}

@end


#elif FOUNDATION_PLATFORM_IOS

#import <Foundation/NSNotification.h>


static __weak UIApplication*          _delegate_app;
static __weak FoundationAppDelegate*  _delegate;


void* delegate_uiwindow( void )
{
	return (__bridge void*)(_delegate ? [_delegate window] : 0);
}


@implementation FoundationAppDelegate

@synthesize window;


+ (void)referenceClass
{
	_delegate_dummy = 2;
}


- (void)applicationDidFinishLaunching:(UIApplication*)application
{
	FOUNDATION_UNUSED( application );
	_delegate = self;
    _delegate_received_start = true;

	log_debug( HASH_FOUNDATION, "Application finished launching" );
	system_post_event( FOUNDATIONEVENT_START );

    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(deviceOrientationDidChange:) name:UIDeviceOrientationDidChangeNotification object:nil];
}


- (void)applicationWillResignActive:(UIApplication*)application
{
	FOUNDATION_UNUSED( application );
	log_debug( HASH_FOUNDATION, "Application will resign active" );
	system_post_event( FOUNDATIONEVENT_FOCUS_LOST );
	system_post_event( FOUNDATIONEVENT_PAUSE );
}


- (void)applicationDidBecomeActive:(UIApplication*)application
{
	FOUNDATION_UNUSED( application );
	log_debug( HASH_FOUNDATION, "Application became active" );
	_delegate_app = application;
	system_post_event( FOUNDATIONEVENT_RESUME );
	system_post_event( FOUNDATIONEVENT_FOCUS_GAIN );
}


- (void)applicationWillTerminate:(UIApplication*)application
{
	FOUNDATION_UNUSED( application );
	_delegate_received_terminate = true;

	log_debug( HASH_FOUNDATION, "Application will terminate" );
	system_post_event( FOUNDATIONEVENT_TERMINATE );

	[[NSNotificationCenter defaultCenter] removeObserver:self];
    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];

	while( _delegate_main_thread_running )
		thread_sleep( 1 );
}


- (void)applicationDidEnterBackground:(UIApplication *)application
{
	FOUNDATION_UNUSED( application );
	log_debug( HASH_FOUNDATION, "Application entered background" );
}


- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	FOUNDATION_UNUSED( application );
	log_warnf( 0, WARNING_MEMORY, "Application received memory warning" );
	system_post_event( FOUNDATIONEVENT_LOW_MEMORY_WARNING );

#if BUILD_DEBUG || BUILD_RELEASE
	@autoreleasepool
	{
		CGRect flash_frame = [(__bridge UIWindow*)delegate_uiwindow() frame];
		flash_frame.size.height = 60;

		float duration = 1.0f;
		UIView* flash = [[UIView alloc] initWithFrame:flash_frame];
		flash.backgroundColor = [UIColor redColor];
		[(__bridge UIWindow*)delegate_uiwindow() addSubview:flash];

		dispatch_after( dispatch_time( DISPATCH_TIME_NOW, (int64_t)( (double)( duration + 0.1 ) * (double)NSEC_PER_SEC ) ), dispatch_get_main_queue(), ^{
			[flash removeFromSuperview];
		});

		[UIView beginAnimations:@"FoundationMemoryWarningFlash" context:0];
		[UIView setAnimationDuration:duration];
		flash.alpha = 0.0;
		[UIView commitAnimations];
	}
#endif

}


- (void)deviceOrientationDidChange:(NSNotification*)notification
{
	FOUNDATION_UNUSED( notification );
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];

	log_debugf( 0, "Device orientation changed to %d", (int)orientation );
	system_set_device_orientation( (device_orientation_t)orientation );
}


- (void) dealloc
{
	log_debug( 0, "Application dealloc" );

	_delegate_app = 0;
	_delegate = 0;
}

@end


#endif
