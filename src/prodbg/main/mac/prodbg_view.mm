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
#include <pd_keys.h>

//NSWindow* g_window = 0;

void Window_setTitle(const char* title);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scan codes on Mac taken from http://boredzo.org/blog/archives/2007-05-22/virtual-key-codes

//#define KEY_RETURN 36
//#define KEY_TAB 48
//#define KEY_DELETE 51
//#define KEY_ESCAPE 53

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	if (flags & NSShiftKeyMask)
		specialKeys |= PDKEY_SHIFT;

	if (flags & NSAlternateKeyMask)
		specialKeys |= PDKEY_ALT;

	if (flags & NSControlKeyMask)
		specialKeys |= PDKEY_CTRL;

	if (flags & NSCommandKeyMask)
		specialKeys |= PDKEY_SUPER;

    return specialKeys;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int translateKey(unsigned int key)
{
    // Keyboard symbol translation table

    static const unsigned int table[128] =
    {
        /* 00 */ PDKEY_A,
        /* 01 */ PDKEY_S,
        /* 02 */ PDKEY_D,
        /* 03 */ PDKEY_F,
        /* 04 */ PDKEY_H,
        /* 05 */ PDKEY_G,
        /* 06 */ PDKEY_Z,
        /* 07 */ PDKEY_X,
        /* 08 */ PDKEY_C,
        /* 09 */ PDKEY_V,
        /* 0a */ PDKEY_GRAVE_ACCENT,
        /* 0b */ PDKEY_B,
        /* 0c */ PDKEY_Q,
        /* 0d */ PDKEY_W,
        /* 0e */ PDKEY_E,
        /* 0f */ PDKEY_R,
        /* 10 */ PDKEY_Y,
        /* 11 */ PDKEY_T,
        /* 12 */ PDKEY_1,
        /* 13 */ PDKEY_2,
        /* 14 */ PDKEY_3,
        /* 15 */ PDKEY_4,
        /* 16 */ PDKEY_6,
        /* 17 */ PDKEY_5,
        /* 18 */ PDKEY_EQUAL,
        /* 19 */ PDKEY_9,
        /* 1a */ PDKEY_7,
        /* 1b */ PDKEY_MINUS,
        /* 1c */ PDKEY_8,
        /* 1d */ PDKEY_0,
        /* 1e */ PDKEY_RIGHT_BRACKET,
        /* 1f */ PDKEY_O,
        /* 20 */ PDKEY_U,
        /* 21 */ PDKEY_LEFT_BRACKET,
        /* 22 */ PDKEY_I,
        /* 23 */ PDKEY_P,
        /* 24 */ PDKEY_ENTER,
        /* 25 */ PDKEY_L,
        /* 26 */ PDKEY_J,
        /* 27 */ PDKEY_APOSTROPHE,
        /* 28 */ PDKEY_K,
        /* 29 */ PDKEY_SEMICOLON,
        /* 2a */ PDKEY_BACKSLASH,
        /* 2b */ PDKEY_COMMA,
        /* 2c */ PDKEY_SLASH,
        /* 2d */ PDKEY_N,
        /* 2e */ PDKEY_M,
        /* 2f */ PDKEY_PERIOD,
        /* 30 */ PDKEY_TAB,
        /* 31 */ PDKEY_SPACE,
        /* 32 */ PDKEY_UNKNOWN,
        /* 33 */ PDKEY_BACKSPACE,
        /* 34 */ PDKEY_UNKNOWN,
        /* 35 */ PDKEY_ESCAPE,
        /* 36 */ PDKEY_RIGHT_SUPER,
        /* 37 */ PDKEY_LEFT_SUPER,
        /* 38 */ PDKEY_LEFT_SHIFT,
        /* 39 */ PDKEY_CAPS_LOCK,
        /* 3a */ PDKEY_LEFT_ALT,
        /* 3b */ PDKEY_LEFT_CONTROL,
        /* 3c */ PDKEY_RIGHT_SHIFT,
        /* 3d */ PDKEY_RIGHT_ALT,
        /* 3e */ PDKEY_RIGHT_CONTROL,
        /* 3f */ PDKEY_UNKNOWN,
        /* 40 */ PDKEY_F17,
        /* 41 */ PDKEY_KP_DECIMAL,
        /* 42 */ PDKEY_UNKNOWN,
        /* 43 */ PDKEY_KP_MULTIPLY,
        /* 44 */ PDKEY_UNKNOWN,
        /* 45 */ PDKEY_KP_ADD,
        /* 46 */ PDKEY_UNKNOWN,
        /* 47 */ PDKEY_NUM_LOCK,
        /* 48 */ PDKEY_UNKNOWN,
        /* 49 */ PDKEY_UNKNOWN,
        /* 4a */ PDKEY_UNKNOWN,
        /* 4b */ PDKEY_KP_DIVIDE,
        /* 4c */ PDKEY_KP_ENTER,
        /* 4d */ PDKEY_UNKNOWN,
        /* 4e */ PDKEY_KP_SUBTRACT,
        /* 4f */ PDKEY_F18,
        /* 50 */ PDKEY_F19,
        /* 51 */ PDKEY_KP_EQUAL,
        /* 52 */ PDKEY_KP_0,
        /* 53 */ PDKEY_KP_1,
        /* 54 */ PDKEY_KP_2,
        /* 55 */ PDKEY_KP_3,
        /* 56 */ PDKEY_KP_4,
        /* 57 */ PDKEY_KP_5,
        /* 58 */ PDKEY_KP_6,
        /* 59 */ PDKEY_KP_7,
        /* 5a */ PDKEY_F20,
        /* 5b */ PDKEY_KP_8,
        /* 5c */ PDKEY_KP_9,
        /* 5d */ PDKEY_UNKNOWN,
        /* 5e */ PDKEY_UNKNOWN,
        /* 5f */ PDKEY_UNKNOWN,
        /* 60 */ PDKEY_F5,
        /* 61 */ PDKEY_F6,
        /* 62 */ PDKEY_F7,
        /* 63 */ PDKEY_F3,
        /* 64 */ PDKEY_F8,
        /* 65 */ PDKEY_F9,
        /* 66 */ PDKEY_UNKNOWN,
        /* 67 */ PDKEY_F11,
        /* 68 */ PDKEY_UNKNOWN,
        /* 69 */ PDKEY_PRINT_SCREEN,
        /* 6a */ PDKEY_F16,
        /* 6b */ PDKEY_F14,
        /* 6c */ PDKEY_UNKNOWN,
        /* 6d */ PDKEY_F10,
        /* 6e */ PDKEY_UNKNOWN,
        /* 6f */ PDKEY_F12,
        /* 70 */ PDKEY_UNKNOWN,
        /* 71 */ PDKEY_F15,
        /* 72 */ PDKEY_INSERT,
        /* 73 */ PDKEY_HOME,
        /* 74 */ PDKEY_PAGE_UP,
        /* 75 */ PDKEY_DELETE,
        /* 76 */ PDKEY_F4,
        /* 77 */ PDKEY_END,
        /* 78 */ PDKEY_F2,
        /* 79 */ PDKEY_PAGE_DOWN,
        /* 7a */ PDKEY_F1,
        /* 7b */ PDKEY_LEFT,
        /* 7c */ PDKEY_RIGHT,
        /* 7d */ PDKEY_DOWN,
        /* 7e */ PDKEY_UP,
        /* 7f */ PDKEY_UNKNOWN,
    };

    if (key >= 128)
        return PDKEY_UNKNOWN;

    return table[key];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)keyDown:(NSEvent*)event
{
    const int key = translateKey([event keyCode]);
    const int mods = getModifierFlags([event modifierFlags]);

    printf("keyDown %d\n", key);

    ProDBG_keyDown(key, mods);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)keyUp:(NSEvent*)event
{
    const int key = translateKey([event keyCode]);
    const int mods = getModifierFlags([event modifierFlags]);

    printf("keyUp %d\n", key);

    ProDBG_keyUp(key, mods);
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
            NSMenuItem* newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:name action:NULL keyEquivalent:@""];
            NSMenu* newMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:name];
            [newItem setSubmenu:newMenu];
            [newMenu release];
            [menu addItem:newItem];
            [newItem release];
        }
        else
        {
            int mask = 0;
            NSMenuItem* newItem = [[NSMenuItem alloc] initWithTitle:name action:@selector(onMenuPress:) keyEquivalent:@""];
            [newItem setTag:desc->id];

            if (desc->macMod & PRODBG_KEY_COMMAND)
                mask |= NSCommandKeyMask;
            if (desc->macMod & PRODBG_KEY_SHIFT)
                mask |= NSShiftKeyMask;
            if (desc->macMod & PRODBG_KEY_CTRL)
                mask |= NSControlKeyMask;
            if (desc->macMod & PRODBG_KEY_ALT)
                mask |= NSAlternateKeyMask;

            NSString* key = 0;

            if (desc->key >= 256)
            {
            	unichar c = 0;

            	switch (desc->key)
            	{
					case PRODBG_KEY_F1 : c = NSF1FunctionKey; break;
					case PRODBG_KEY_F2 : c = NSF2FunctionKey; break;
					case PRODBG_KEY_F3 : c = NSF3FunctionKey; break;
					case PRODBG_KEY_F4 : c = NSF4FunctionKey; break;
					case PRODBG_KEY_F5 : c = NSF5FunctionKey; break;
					case PRODBG_KEY_F6 : c = NSF6FunctionKey; break;
					case PRODBG_KEY_F7 : c = NSF7FunctionKey; break;
					case PRODBG_KEY_F8 : c = NSF8FunctionKey; break;
					case PRODBG_KEY_F9 : c = NSF9FunctionKey; break;
					case PRODBG_KEY_F10 : c = NSF10FunctionKey; break;
					case PRODBG_KEY_F11 : c = NSF11FunctionKey; break;
					case PRODBG_KEY_F12 : c = NSF12FunctionKey; break;
            	}

            	key = [NSString stringWithCharacters:&c length:1];
            }
            else
            {
            	key = [NSString stringWithFormat:@"%c", desc->key];
            }

            assert(key);

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
    NSMenu* debugMenu = [[mainMenu itemWithTitle:@"Debug"] submenu];

    buildSubMenu(fileMenu, g_fileMenu);
    buildSubMenu(debugMenu, g_debugMenu);
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


