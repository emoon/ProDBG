#include "../dialogs.h"
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_open(char* dest)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSOpenPanel* open = [NSOpenPanel openPanel];
    [open setAllowsMultipleSelection:NO];

    long result = [open runModal];

    if (result != NSModalResponseOK)
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

    long result = [open runModal];

    if (result != NSModalResponseOK)
        return false;

    // Grab the first file

    NSURL* url = [open URL];
    const char* temp = [[url path] UTF8String];

    strcpy(dest, temp);

    [pool drain];

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dialog_showError(const char* text)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSString* message = [[[NSString alloc] initWithUTF8String:text] autorelease];
    NSAlert* alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Alert"];
    [alert setInformativeText:message];
    [alert setAlertStyle:NSCriticalAlertStyle];
    [alert runModal];
    [pool drain];
}

