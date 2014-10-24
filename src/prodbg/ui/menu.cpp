#include "menu.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MenuDescriptor g_fileMenu[] =
{
    { "Open and run executable...", PRODBG_MENU_FILE_OPEN_AND_RUN_EXE, 'd', PRODBG_KEY_COMMAND, PRODBG_KEY_CTRL },
    { "Open source...", PRODBG_MENU_FILE_OPEN_SOURCE, 'o', PRODBG_KEY_COMMAND, PRODBG_KEY_CTRL },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MenuDescriptor g_debugMenu[] =
{
    { "Attach to Remote...", PRODBG_MENU_DEBUG_ATTACH_TO_REMOTE, PRODBG_KEY_F6, 0, 0 },
    { "Break", PRODBG_MENU_DEBUG_BREAK, PRODBG_KEY_F5, 0, 0 },
    { "Step In", PRODBG_MENU_DEBUG_STEP_IN, PRODBG_KEY_F11, 0, 0 },
    { "Step Over", PRODBG_MENU_DEBUG_STEP_OVER, PRODBG_KEY_F10, 0, 0 },
    { "Toggle Breakpoint", PRODBG_MENU_DEBUG_TOGGLE_BREAKPOINT, PRODBG_KEY_F9, 0, 0 },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

