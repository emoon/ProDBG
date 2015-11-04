#pragma once

#include "pd_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    // File

    PRODBG_MENU_START = 0,
    PRODBG_MENU_NEW,
    PRODBG_MENU_SUB_MENU,
    PRODBG_MENU_SEPARATOR,
    PRODBG_MENU_FILE_OPEN_AND_RUN_EXE,
    PRODBG_MENU_FILE_OPEN_SOURCE,

    // Debug

    PRODBG_MENU_DEBUG_ATTACH_TO_REMOTE,
    PRODBG_MENU_DEBUG_START,
    PRODBG_MENU_DEBUG_STEP_IN,
    PRODBG_MENU_DEBUG_STEP_OVER,
    PRODBG_MENU_DEBUG_TOGGLE_BREAKPOINT,

    // Popup

    PRODBG_MENU_POPUP_SPLIT_HORZ,
    PRODBG_MENU_POPUP_SPLIT_VERT,

    // Plugins

    PRODBG_MENU_PLUGIN_START,

    PRODBG_MENU_PLUGINS_START = PRODBG_MENU_PLUGIN_START + 512,

    // End

    PRODBG_MENU_END = PRODBG_MENU_PLUGINS_START + 0x100,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    PRODBG_MENU_POPUP_SPLIT_HORZ_SHIFT = 1 << 13,
    PRODBG_MENU_POPUP_SPLIT_VERT_SHIFT = 1 << 14,
    PRODBG_MENU_POPUP_SPLIT_TAB_SHIFT = 1 << 15,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    PRODBG_KEY_COMMAND = 1 << 1,
    PRODBG_KEY_CTRL = 1 << 2,
    PRODBG_KEY_SHIFT = 1 << 3,
    PRODBG_KEY_ALT = 1 << 4
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    PRODBG_KEY_F1 = 256,
    PRODBG_KEY_F2,
    PRODBG_KEY_F3,
    PRODBG_KEY_F4,
    PRODBG_KEY_F5,
    PRODBG_KEY_F6,
    PRODBG_KEY_F7,
    PRODBG_KEY_F8,
    PRODBG_KEY_F9,
    PRODBG_KEY_F10,
    PRODBG_KEY_F11,
    PRODBG_KEY_F12,
    PRODBG_KEY_ARROW_DOWN,
    PRODBG_KEY_ARROW_UP,
    PRODBG_KEY_ARROW_LEFT,
    PRODBG_KEY_ARROW_RIGHT,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern PDMenuItem g_debugMenu[];
extern PDMenuItem g_fileMenu[];
extern PDMenuItem g_popupMenu[];

#ifdef __cplusplus
}
#endif

