#import "prodbg_view.h"
#include "../prodbg.h"
#include "ui/menu.h"
#include "core/alloc.h"
#include "core/plugin_handler.h"
#include <core/log.h>
#include <bgfx.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <pd_common.h>

//NSWindow* g_window = 0;

void Window_setTitle(const char* title);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scan codes on Mac taken from http://boredzo.org/blog/archives/2007-05-22/virtual-key-codes

//#define KEY_RETURN 36
//#define KEY_TAB 48
//#define KEY_DELETE 51
//#define KEY_ESCAPE 53

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MyMenuItem : NSMenuItem
{
}
- (BOOL)isHighlighted;
@end

@implementation MyMenuItem
- (BOOL)isHighlighted
{
    return NO;
}
@end

@implementation ProDBGView


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-(void) updateMain
{
    ProDBG_timedUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self == nil)
        return nil;

    ProDBG_create(0, (int)frame.size.width, (int)frame.size.height);
    const float framerate = 60;
    const float frequency = 1.0f / framerate;
    [NSTimer scheduledTimerWithTimeInterval:frequency target:self selector:@selector(updateMain) userInfo:nil repeats:YES];

    return self;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)lockFocus
{
    NSOpenGLContext* context = (NSOpenGLContext*)bgfx::nativeContext();

    [super lockFocus];

    if ([context view] != self)
        [context setView:self];

    [context makeCurrentContext];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)windowDidResize:(NSNotification*)notification
{
    (void)notification;
    printf("resize\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)drawRect:(NSRect)frame
{
    (void)frame;

    ProDBG_setWindowSize((int)frame.size.width, (int)frame.size.height);
    ProDBG_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int getModifierFlags(int flags)
{
    int specialKeys = 0;

    (void)flags;

    /*
       if (flags & NSShiftKeyMask)
        specialKeys |= PRODBG_KEY_SHIFT;

       if (flags & NSAlternateKeyMask)
        specialKeys |= PRODBG_KEY_ALT;

       if (flags & NSControlKeyMask)
        specialKeys |= PRODBG_KEY_CTRL;

       if (flags & NSCommandKeyMask)
        specialKeys |= PRODBG_KEY_COMMAND;
     */

    return specialKeys;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)keyDown:(NSEvent*)theEvent
{
    (void)theEvent;
    //NSString* key = [theEvent charactersIgnoringModifiers];
    //unichar keyChar = 0;
    //if ([key length] == 0)
    //	return;

    //keyChar = [key characterAtIndex:0];

    //int keyCode = keyChar;
    //int specialKeys = getModifierFlags([theEvent modifierFlags]);

    /*

       if ([theEvent modifierFlags] & NSNumericPadKeyMask)
       {
        switch (keyChar)
        {
            case NSLeftArrowFunctionKey: keyCode = PRODBG_KEY_ARROW_LEFT; break;
            case NSRightArrowFunctionKey: keyCode = PRODBG_KEY_ARROW_RIGHT; break;
            case NSUpArrowFunctionKey: keyCode = PRODBG_KEY_ARROW_UP; break;
            case NSDownArrowFunctionKey: keyCode = PRODBG_KEY_ARROW_DOWN; break;
        }
       }
       else
       {
        switch ([theEvent keyCode])
        {
            case KEY_TAB : keyCode = PRODBG_KEY_TAB; break;
            case KEY_DELETE : keyCode = PRODBG_KEY_BACKSPACE; break;
            case KEY_RETURN : keyCode = PRODBG_KEY_ENTER; break;
            case KEY_ESCAPE : keyCode = PRODBG_KEY_ESC; break;
            case NSPageDownFunctionKey: keyCode = PRODBG_KEY_PAGE_DOWN; break;
            case NSPageUpFunctionKey: keyCode = PRODBG_KEY_PAGE_UP; break;
        }
       }

       Emgui_sendKeyinput(keyCode, specialKeys);

       if (!Editor_keyDown(keyCode, [theEvent keyCode], specialKeys))
        [super keyDown:theEvent];

       Editor_update();
     */

    [super keyDown:theEvent];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (BOOL)acceptsFirstResponder
{
    return YES;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-(void) viewWillMoveToWindow:(NSWindow*)newWindow
{
    NSTrackingArea* trackingArea = [[NSTrackingArea alloc] initWithRect:[self frame]
                                    options: (NSTrackingMouseMoved | NSTrackingActiveAlways) owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
    (void)newWindow;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mouseMoved:(NSEvent*)event
{
    NSWindow* window = [self window];
    NSRect originalFrame = [window frame];
    NSPoint location = [window mouseLocationOutsideOfEventStream];
    NSRect adjustFrame = [NSWindow contentRectForFrameRect: originalFrame styleMask: NSTitledWindowMask];

    ProDBG_setMousePos(location.x, adjustFrame.size.height - location.y);

    (void)event;

    ProDBG_setMousePos(location.x, adjustFrame.size.height - location.y);
    ProDBG_update();

    //Emgui_setMousePos((int)location.x, (int)adjustFrame.size.height - (int)location.y);
    //Editor_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mouseDragged:(NSEvent*)event
{
    NSWindow* window = [self window];
    NSRect originalFrame = [window frame];
    NSPoint location = [window mouseLocationOutsideOfEventStream];
    NSRect adjustFrame = [NSWindow contentRectForFrameRect: originalFrame styleMask: NSTitledWindowMask];

    (void)event;

    ProDBG_setMousePos(location.x, adjustFrame.size.height - location.y);
    ProDBG_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)scrollWheel:(NSEvent*)theEvent
{
    (void)theEvent;
    //float x = (float)[theEvent deltaX];
    //float y = (float)[theEvent deltaY];
    //int flags = getModifierFlags([theEvent modifierFlags]);

    //printf("%f %f %d\n", x, y, flags);
    //Editor_scroll(-x, -y, flags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mouseUp:(NSEvent*)event
{
    (void)event;
    ProDBG_setMouseState(0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mouseDown:(NSEvent*)event
{
    NSWindow* window = [self window];
    NSRect originalFrame = [window frame];
    NSPoint location = [window mouseLocationOutsideOfEventStream];
    NSRect adjustFrame = [NSWindow contentRectForFrameRect: originalFrame styleMask: NSTitledWindowMask];

    (void)event;

    ProDBG_setMousePos(location.x, adjustFrame.size.height - location.y);
    ProDBG_setMouseState(0, 1);
    ProDBG_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-(BOOL) isOpaque
{
    return YES;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)onMenuPress:(id)sender
{
    int id = (int)((NSButton*)sender).tag;
    ProDBG_event(id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void buildSubMenu(NSMenu* menu, MenuDescriptor menuDesc[])
{
    MenuDescriptor* desc = &menuDesc[0];
    [menu removeAllItems];

    while (desc->name)
    {
        NSString* name = [NSString stringWithUTF8String: desc->name];

        if (desc->id == PRODBG_MENU_SEPARATOR)
        {
            [menu addItem:[NSMenuItem separatorItem]];
        }
        else if (desc->id == PRODBG_MENU_SUB_MENU)
        {
            MyMenuItem* newItem = [[MyMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:name action:NULL keyEquivalent:@""];
            NSMenu* newMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:name];
            [newItem setSubmenu:newMenu];
            [newMenu release];
            [menu addItem:newItem];
            [newItem release];
        }
        else
        {
            int mask = 0;
            MyMenuItem* newItem = [[MyMenuItem alloc] initWithTitle:name action:@selector(onMenuPress:) keyEquivalent:@""];
            [newItem setTag:desc->id];

            if (desc->macMod & PRODBG_KEY_COMMAND)
                mask |= NSCommandKeyMask;
            if (desc->macMod & PRODBG_KEY_SHIFT)
                mask |= NSShiftKeyMask;
            if (desc->macMod & PRODBG_KEY_CTRL)
                mask |= NSControlKeyMask;
            if (desc->macMod & PRODBG_KEY_ALT)
                mask |= NSAlternateKeyMask;

            // TODO: Support special keys

            NSString* key = [NSString stringWithFormat:@"%c", desc->key];

            if (key)
            {
                [newItem setKeyEquivalentModifierMask: mask];
                [newItem setKeyEquivalent:key];
            }
            else
            {
                fprintf(stderr, "Unable to map keyboard shortcut for %s\n", desc->name);
            }

            [newItem setOnStateImage: newItem.offStateImage];
            [menu addItem:newItem];
            [newItem release];
        }

        desc++;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_buildMenu()
{
    NSMenu* mainMenu = [NSApp mainMenu];
    NSMenu* fileMenu = [[mainMenu itemWithTitle:@"File"] submenu];

    (void)fileMenu;

    //buildSubMenu(pluginsMenu, g_pluginsMenu);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Window_buildPluginMenu(PluginData** plugins, int count)
{
    NSMenu* mainMenu = [NSApp mainMenu];
    NSMenu* pluginsMenu = [[mainMenu itemWithTitle:@"Plugins"] submenu];

	// TODO: Right now we only support up to 1 - 9 for keyboard shortcuts of the plugins but should be good
	// enough for now.

	if (count >= 10) 
		count = 9;

	MenuDescriptor* menu = (MenuDescriptor*)alloc_zero(sizeof(MenuDescriptor) * (count + 1)); // + 1 as array needs to end with zeros

	for (int i = 0; i < count; ++i)
	{
		PluginData* pluginData = plugins[i];
		PDPluginBase* pluginBase = (PDPluginBase*)pluginData->plugin;
		MenuDescriptor* entry = &menu[i];

		// TODO: Hack hack! 

		if (!strstr(pluginData->type, "View"))
			continue;

		entry->name = pluginBase->name;
		entry->id = PRODBG_MENU_PLUGIN_START + i;
		entry->key = '1' + i;
		entry->macMod = PRODBG_KEY_COMMAND;
		entry->winMod = PRODBG_KEY_CTRL;
    }

    buildSubMenu(pluginsMenu, menu);

	return PRODBG_MENU_PLUGIN_START;
}

@end

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   void Window_setTitle(const char* title)
   {
    [g_window setTitle:[NSString stringWithUTF8String:title]];
   }
 */


