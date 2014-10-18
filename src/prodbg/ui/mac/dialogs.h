#include "../Dialog.h"
#include <emgui/emgui.h>
#import <Cocoa/Cocoa.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_open(char* dest)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSOpenPanel* open = [NSOpenPanel openPanel];
	[open setAllowsMultipleSelection:NO];

	int result = [open runModal];

	if (result != NSOKButton)
		return false;

	// Grab the first file

	NSArray* selectedFiles = [open URLs];
	NSURL* url = [selectedFiles objectAtIndex:0];
	const char* temp = [[url path] UTF8String];

	strcpy(dest, temp);

	[pool drain];

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_save(char* dest)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSSavePanel* open = [NSSavePanel savePanel];

	int result = [open runModal];

	if (result != NSOKButton)
		return false;

	// Grab the first file

	NSURL* url = [open URL];
	const char* temp = [[url path] UTF8String];

	strcpy(dest, temp);

	[pool drain];

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface ColorDelegate : NSObject<NSWindowDelegate>
{
    bool m_bIsClosed;
}

// Delegate methods
- (id)init;
- (BOOL)windowShouldClose:(id)sender;
- (BOOL)isClosed;
@end

@implementation ColorDelegate : NSObject

- (id)init
{
    [super init];
    m_bIsClosed = false;

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    (void)sender;

    m_bIsClosed = true;

    [NSApp abortModal];
    [NSApp stopModal];
    return YES;
}

- (BOOL)isClosed
{
    return m_bIsClosed;
}

@end

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dialog_showColorPicker(uint32_t* color)
{
	uint32_t c = *color;

	NSAutoreleasePool *thePool;
    thePool = [[NSAutoreleasePool alloc] init];

	//Get the shared color and font panel
	[[NSColorPanel sharedColorPanel] setColor:
            [NSColor colorWithCalibratedRed:(CGFloat) (Emgui_color32_getR(c) / 255.0)
                                        green:(CGFloat) (Emgui_color32_getG(c) / 255.0)
                                        blue:(CGFloat) (Emgui_color32_getB(c) / 255.0)
                                        alpha:(CGFloat) 1.0]]; 

	NSColorPanel* theColorPanel = [NSColorPanel sharedColorPanel];

	//Create and assign the delegates (cocoa event handlers) so
	//we can tell if a window has closed/open or not
	ColorDelegate* colorDelegate = [[ColorDelegate alloc] init];
	[theColorPanel setDelegate:colorDelegate];
		NSModalSession session = [NSApp beginModalSessionForWindow:theColorPanel];
		for (;;)
		{
			[NSApp runModalSession:session];

			//If the color panel is closed, return the font panel modal loop
			if ([colorDelegate isClosed])
				break;
		}
		[NSApp endModalSession:session];

	[theColorPanel setDelegate:nil];
	[colorDelegate release];

	//Get the shared color panel along with the chosen color and set the chosen color
	NSColor* theColor = [[theColorPanel color] colorUsingColorSpaceName:NSCalibratedRGBColorSpace];

	*color = Emgui_color32((unsigned char) ([theColor redComponent] * 255.0),
						   (unsigned char) ([theColor greenComponent] * 255.0),
						   (unsigned char) ([theColor blueComponent] * 255.0),
						   255);

	[thePool release];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dialog_showError(const text_t* text)
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSString* message = [[[NSString alloc] initWithUTF8String:text] autorelease];// convert 

	NSRunAlertPanel(@"Error", message, @"Ok", @"", @"");

	[pool drain];
}

