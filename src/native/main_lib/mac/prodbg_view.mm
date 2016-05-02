#import "prodbg_view.h"
#include "../prodbg.h"
#include "../prodbg_version.h"
#include "ui/menu.h"
//#include "core/alloc.h"
//#include "core/input_state.h"
//#include "core/plugin_handler.h"
//#include <core/log.h>
#include <bgfx.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <pd_common.h>
#include <pd_keys.h>
#include <AppKit/NSOpenGL.h>

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

-(void) updateMain {
    prodbg_timed_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self == nil)
        return nil;

    prodbg_create(0, (int)frame.size.width, (int)frame.size.height);
    const float framerate = 60;
    const float frequency = 1.0f / framerate;
    [NSTimer scheduledTimerWithTimeInterval:frequency target:self selector:@selector(updateMain) userInfo:nil repeats:YES];

    return self;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)lockFocus {
    bgfx::bgfx_internal_data_t *internalData = bgfx::getInternalData();
    NSOpenGLContext* context = (NSOpenGLContext*)internalData->context;

    [super lockFocus];

    if ([context view] != self)
        [context setView:self];

    [context makeCurrentContext];
    [[self window] setTitle:PRODBG_VERSION];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)windowDidResize:(NSNotification*)notification {
    (void)notification;
    printf("resize\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)drawRect:(NSRect)frame {
    (void)frame;

    prodbg_set_window_size((int)frame.size.width, (int)frame.size.height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int getModifierFlags(int flags) {
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

static int translateKey(unsigned int key) {
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

- (void)keyDown:(NSEvent*)event {
    const int key = translateKey([event keyCode]);
    const int mods = getModifierFlags([event modifierFlags]);

    prodbg_key_down(key, mods);

    NSString* characters = [event characters];
    NSUInteger i, length = [characters length];

    for (i = 0; i < length; i++)
        prodbg_add_char([characters characterAtIndex:i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)flagsChanged:(NSEvent *)event {
    bool release = false;
    const unsigned int modifierFlags = [event modifierFlags] & NSDeviceIndependentModifierFlagsMask;
    const int key = translateKey([event keyCode]);
    const int mods = getModifierFlags(modifierFlags);

/*
    InputState* state = InputState_getState();

    if (modifierFlags == state->modifierFlags) {
        if (state->keysDown[key])
            release = true;
        else
            release = false;
    }else if (modifierFlags > state->modifierFlags)
        release = false;
    else
        release = true;

    state->modifierFlags = modifierFlags;
*/

    if (release)
        prodbg_key_up(key, mods);
    else
        prodbg_key_down(key, mods);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)keyUp:(NSEvent*)event {
    const int key = translateKey([event keyCode]);
    const int mods = getModifierFlags([event modifierFlags]);

    prodbg_key_up(key, mods);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (BOOL)acceptsFirstResponder {
    return YES;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-(void) viewWillMoveToWindow:(NSWindow*)newWindow {
    NSTrackingArea* trackingArea = [[NSTrackingArea alloc] initWithRect:[self frame]
                                    options: (NSTrackingInVisibleRect | NSTrackingMouseMoved | NSTrackingActiveAlways) owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
    (void)newWindow;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mouseMoved:(NSEvent*)event {
    NSWindow* window = [self window];
    NSRect originalFrame = [window frame];
    NSPoint location = [window mouseLocationOutsideOfEventStream];
    NSRect adjustFrame = [NSWindow contentRectForFrameRect: originalFrame styleMask: NSTitledWindowMask];

    prinf("%f %f - %f %f\n", 
   		adjustFrame.origin.x, 
   		adjustFrame.origin.y, 
   		adjustFrame.size.width, 
   		adjustFrame.size.height);

    prodbg_set_mouse_pos(location.x, adjustFrame.size.height - location.y);

    (void)event;

    prodbg_set_mouse_pos(location.x, adjustFrame.size.height - location.y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mouseDragged:(NSEvent*)event {
    NSWindow* window = [self window];
    NSRect originalFrame = [window frame];
    NSPoint location = [window mouseLocationOutsideOfEventStream];
    NSRect adjustFrame = [NSWindow contentRectForFrameRect: originalFrame styleMask: NSTitledWindowMask];

    (void)event;

    prodbg_set_mouse_pos(location.x, adjustFrame.size.height - location.y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)scrollWheel:(NSEvent *)event {
    float x = (float)[event deltaX];
    float y = (float)[event deltaY];
    //int flags = getModifierFlags([theEvent modifierFlags]);
    prodbg_set_scroll(x, y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mouseUp:(NSEvent*)event {
    (void)event;
    prodbg_set_mouse_state(0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mouseDown:(NSEvent*)event {
    NSWindow* window = [self window];
    NSRect originalFrame = [window frame];
    NSPoint location = [window mouseLocationOutsideOfEventStream];
    NSRect adjustFrame = [NSWindow contentRectForFrameRect: originalFrame styleMask: NSTitledWindowMask];

    //[theMenu insertItemWithTitle:@"Beep" action:@selector(beep:) keyEquivalent:@"" atIndex:0];
    //[theMenu insertItemWithTitle:@"Honk" action:@selector(honk:) keyEquivalent:@"" atIndex:1];


    (void)event;

    prodbg_set_mouse_pos(location.x, adjustFrame.size.height - location.y);
    prodbg_set_mouse_state(0, 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-(BOOL) isOpaque {
    return YES;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)onMenuPress:(id)sender {
    int id = (int)((NSButton*)sender).tag;
    prodbg_event(id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void buildSubMenu(NSMenu* menu, PDMenuItem menuDesc[], int idOffset) {
    PDMenuItem* desc = &menuDesc[0];
    //[menu removeAllItems];

    while (desc->name) {
        NSString* name = [NSString stringWithUTF8String: desc->name];

        int menuId = desc->id + idOffset;

        if (menuId == PRODBG_MENU_SEPARATOR) {
            [menu addItem:[NSMenuItem separatorItem]];
        }else if (menuId == PRODBG_MENU_SUB_MENU) {
            NSMenuItem* newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:name action:NULL keyEquivalent:@""];
            NSMenu* newMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:name];
            [newItem setSubmenu:newMenu];
            [newMenu release];
            [menu addItem:newItem];
            [newItem release];
        }else {
            int mask = 0;
            NSMenuItem* newItem = [[NSMenuItem alloc] initWithTitle:name action:@selector(onMenuPress:) keyEquivalent:@""];
            [newItem setTag:(menuId)];

            if (desc->mac_mod & PRODBG_KEY_COMMAND)
                mask |= NSCommandKeyMask;
            if (desc->mac_mod & PRODBG_KEY_SHIFT)
                mask |= NSShiftKeyMask;
            if (desc->mac_mod & PRODBG_KEY_CTRL)
                mask |= NSControlKeyMask;
            if (desc->mac_mod & PRODBG_KEY_ALT)
                mask |= NSAlternateKeyMask;

            NSString* key = 0;

            if (desc->key >= 256) {
                unichar c = 0;

                switch (desc->key) {
                    case PRODBG_KEY_F1:
                        c = NSF1FunctionKey; break;
                    case PRODBG_KEY_F2:
                        c = NSF2FunctionKey; break;
                    case PRODBG_KEY_F3:
                        c = NSF3FunctionKey; break;
                    case PRODBG_KEY_F4:
                        c = NSF4FunctionKey; break;
                    case PRODBG_KEY_F5:
                        c = NSF5FunctionKey; break;
                    case PRODBG_KEY_F6:
                        c = NSF6FunctionKey; break;
                    case PRODBG_KEY_F7:
                        c = NSF7FunctionKey; break;
                    case PRODBG_KEY_F8:
                        c = NSF8FunctionKey; break;
                    case PRODBG_KEY_F9:
                        c = NSF9FunctionKey; break;
                    case PRODBG_KEY_F10:
                        c = NSF10FunctionKey; break;
                    case PRODBG_KEY_F11:
                        c = NSF11FunctionKey; break;
                    case PRODBG_KEY_F12:
                        c = NSF12FunctionKey; break;
                }

                key = [NSString stringWithCharacters:&c length:1];
            }else {
                key = [NSString stringWithFormat:@"%c", desc->key];
            }

            assert(key);

            if (key) {
                [newItem setKeyEquivalentModifierMask: mask];
                [newItem setKeyEquivalent:key];
            }else {
                fprintf(stderr, "Unable to map keyboard shortcut for %s\n", desc->name);
            }

            [newItem setOnStateImage: newItem.offStateImage];
            [menu addItem:newItem];
            [newItem release];
        }

        desc++;
    }
}

static NSMenu* s_popupMenu;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)rightMouseDown:(NSEvent*)event {
	buildPopupMenu(0, 0);

    if (!s_popupMenu)
        return;

    printf("right mouse down...\n");

    [NSMenu popUpContextMenu:s_popupMenu withEvent:event forView:self];

    printf("end menu...\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_buildMenu() {
    NSMenu* mainMenu = [NSApp mainMenu];
    NSMenu* fileMenu = [[mainMenu itemWithTitle:@"File"] submenu];
    NSMenu* debugMenu = [[mainMenu itemWithTitle:@"Debug"] submenu];

    buildSubMenu(fileMenu, g_fileMenu, 0);
    buildSubMenu(debugMenu, g_debugMenu, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_addMenu(const char* inName, PDMenuItem* items, uint32_t idOffset) {
    (void)idOffset;
    //NSMenu* mainMenu = [NSApp mainMenu];
    NSString* name = [NSString stringWithUTF8String: inName];

    NSMenu* newMenu;
    NSMenuItem* newItem;

    // Add the submenu
    newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:name action:NULL keyEquivalent:@""];
    newMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:name];

    buildSubMenu(newMenu, items, idOffset);

    [newItem setSubmenu:newMenu];
    [newMenu release];
    [[NSApp mainMenu] insertItem:newItem atIndex:4];
    [newItem release];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
PDMenuItem* buildPluginsMenu(PluginData** plugins, int count) {
    PDMenuItem* menu = (PDMenuItem*)alloc_zero(sizeof(PDMenuItem) * (count + 1)); // + 1 as array needs to end with zeros

    for (int i = 0; i < count; ++i) {
        PluginData* pluginData = plugins[i];
        PDPluginBase* pluginBase = (PDPluginBase*)pluginData->plugin;
        PDMenuItem* entry = &menu[i];

        // TODO: Hack hack!

        if (!strstr(pluginData->type, "View"))
            continue;

        entry->name = pluginBase->name;

        // TODO: Only shortcuts for the first range but we should really have this in a config instead.

        if (i < 10) {
            entry->id = PRODBG_MENU_PLUGIN_START + i;
            entry->key = '1' + i;
        }

        entry->mac_mod = PRODBG_KEY_COMMAND;
        entry->win_mod = PRODBG_KEY_CTRL;
    }

    return menu;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static void buildPopupSubmenu(NSMenu* popupMenu, const char* inName, PDMenuItem* pluginsMenu, int count, uint32_t startId, uint32_t idMask) {
    NSString* name = [NSString stringWithUTF8String: inName];

    NSMenuItem* newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:name action:NULL keyEquivalent:@""];
    NSMenu* newMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:name];

    NSMenuItem* startItem = [[NSMenuItem alloc] initWithTitle:name action:@selector(onMenuPress:) keyEquivalent:@""];
    [startItem setTag:startId];

    [newMenu addItem:startItem];
    [newMenu addItem:[NSMenuItem separatorItem]];

    for (int i = 0; i < count; ++i) {
        pluginsMenu[i].key = 0;
        pluginsMenu[i].id = (uint32_t)i | idMask;
    }

    buildSubMenu(newMenu, pluginsMenu, 0);

    [newItem setSubmenu:newMenu];
    [popupMenu addItem:newItem];

    [name release];
}
*/


// thoeu


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void buildPopupMenu(PDMenuItem* pluginsMenu, int count) {
    // TODO: Support rebuild of this menu

	(void)pluginsMenu;
	(void)count;

    if (s_popupMenu)
        return;

    s_popupMenu = [[NSMenu alloc] initWithTitle:@"Contextual Menu"];

    [s_popupMenu insertItemWithTitle:@"Beep" action:@selector(onMenuPress:) keyEquivalent:@"" atIndex:0];
    [s_popupMenu insertItemWithTitle:@"Honk" action:@selector(onMenuPress:) keyEquivalent:@"" atIndex:1];

/*

    buildPopupSubmenu(s_popupMenu, "Split Horizontally", pluginsMenu, count,
                      PRODBG_MENU_POPUP_SPLIT_HORZ, PRODBG_MENU_POPUP_SPLIT_HORZ_SHIFT);

    buildPopupSubmenu(s_popupMenu, "Split Vertically", pluginsMenu, count,
                      PRODBG_MENU_POPUP_SPLIT_VERT, PRODBG_MENU_POPUP_SPLIT_VERT_SHIFT);
*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
int Window_buildPluginMenu(PluginData** plugins, int count) {
    NSMenu* mainMenu = [NSApp mainMenu];
    NSMenu* pluginsMenu = [[mainMenu itemWithTitle:@"Plugins"] submenu];

    PDMenuItem* menu = buildPluginsMenu(plugins, count);

    buildSubMenu(pluginsMenu, menu, 0);

    buildPopupMenu(menu, count);

    return PRODBG_MENU_PLUGIN_START;
}
*/

@end

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   void Window_setTitle(const char* title)
   {
    [g_window setTitle:[NSString stringWithUTF8String:title]];
   }
 */


