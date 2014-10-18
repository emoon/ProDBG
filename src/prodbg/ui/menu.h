#pragma once

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    // File

    PRODBG_MENU_NEW = 0x1000,
    PRODBG_MENU_SUB_MENU,
    PRODBG_MENU_SEPARATOR,
    PRODBG_MENU_FILE_OPEN_AND_RUN_EXE,

    // Debug

    PRODBG_MENU_DEBUG_STEP_INTO,
    PRODBG_MENU_DEBUG_STEP_OVER,
    PRODBG_MENU_DEBUG_STEP_OUT,

    // Plugins

    PRODBG_MENU_PLUGIN_START,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    PRODBG_KEY_COMMAND = 1 << 1,
    PRODBG_KEY_CTRL = 1 << 2,
    PRODBG_KEY_SHIFT = 1 << 3,
    PRODBG_KEY_ALT = 1 << 8
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
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

typedef struct MenuDescriptor
{
    const char* name;
    int id;
    int key;
    int macMod;
    int winMod;
} MenuDescriptor;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern MenuDescriptor g_debugMenu[];
extern MenuDescriptor g_fileMenu[];

#ifdef __cplusplus
}
#endif

