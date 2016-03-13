#include "../dialogs.h"
//#include <foundation/apple.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_open(char* dest) {
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

int Dialog_save(char* dest) {
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

int Dialog_selectDirectory(char* dest) {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSOpenPanel* open = [NSOpenPanel openPanel];

    [open setCanChooseDirectories:YES];
    [open setCanChooseFiles:NO];
    [open setAllowsMultipleSelection:NO];

    long result = [open runModal];

    if (result != NSModalResponseOK)
        return false;

    NSArray* selectedFiles = [open URLs];
    NSURL* url = [selectedFiles objectAtIndex:0];
    const char* temp = [[url path] UTF8String];

    strcpy(dest, temp);

    [pool drain];

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void internalPanel(const char* titleText, const char* messageText, int type) {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSString* text = [[[NSString alloc] initWithUTF8String:titleText] autorelease];// convert
    NSString* message = [[[NSString alloc] initWithUTF8String:messageText] autorelease];// convert

    NSAlert* alert = [[NSAlert alloc] init];

    [alert setMessageText:text];
    [alert setInformativeText:message];
    [alert setAlertStyle:(NSAlertStyle)type];
    [alert runModal];
    [alert release];

    [pool drain];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MacDialog_errorDialog(const char* title, const char* message) {
    internalPanel(title, message, NSCriticalAlertStyle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MacDialog_infoDialog(const char* title, const char* message) {
    internalPanel(title, message, NSInformationalAlertStyle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MacDialog_warningDialog(const char* title, const char* message) {
    internalPanel(title, message, NSWarningAlertStyle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

