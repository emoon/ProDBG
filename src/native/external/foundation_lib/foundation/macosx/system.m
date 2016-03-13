/* system.m  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/platform.h>
#include <foundation/apple.h>


unsigned int _system_process_info_processor_count( void );
int          _system_show_alert( const char* title, const char* message, int cancel_button );


unsigned int _system_process_info_processor_count( void )
{
	/* [[NSProcessInfo processInfo] activeProcessorCount]; */
	@autoreleasepool { return (unsigned int)[[NSProcessInfo processInfo] processorCount]; }
}


int _system_show_alert( const char* title, const char* message, int cancel_button )
{
	//TODO: Use cancel_button control
	FOUNDATION_UNUSED( cancel_button );
	@autoreleasepool
	{
#if FOUNDATION_PLATFORM_MACOSX

	NSAlert* alert = [NSAlert alertWithMessageText:[NSString stringWithCString:title encoding:NSUTF8StringEncoding]
									 defaultButton:[NSString stringWithCString:"OK" encoding:NSASCIIStringEncoding]
								   alternateButton:[NSString stringWithCString:"Cancel" encoding:NSASCIIStringEncoding]
									   otherButton:nil
						 informativeTextWithFormat:@"%@", [NSString stringWithCString:message encoding:NSUTF8StringEncoding]];

	NSInteger button = [alert runModal];

	return ( button == 1/*NSAlertDefaultReturn*/ ) ? 1 : 0;

#elif FOUNDATION_PLATFORM_IOS

	UIAlertView* alert = [[UIAlertView alloc]
						  initWithTitle:[NSString stringWithCString:title encoding:NSASCIIStringEncoding]
						  message:[NSString stringWithCString:message encoding:NSASCIIStringEncoding]
						  delegate:nil cancelButtonTitle:[NSString stringWithCString:"Cancel" encoding:NSASCIIStringEncoding] otherButtonTitles:nil];

	[alert show];

	return 1;

#endif
	}
}
