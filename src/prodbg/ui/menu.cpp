#include "menu.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDMenuItem g_fileMenu[] =
{
    { "Open and run executable...", PRODBG_MENU_FILE_OPEN_AND_RUN_EXE, 'd', PRODBG_KEY_COMMAND, PRODBG_KEY_CTRL },
    { "Open source...", PRODBG_MENU_FILE_OPEN_SOURCE, 'o', PRODBG_KEY_COMMAND, PRODBG_KEY_CTRL },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDMenuItem g_debugMenu[] =
{
    { "Attach to Remote...", PRODBG_MENU_DEBUG_ATTACH_TO_REMOTE, PRODBG_KEY_F6, 0, 0 },
    { "Start", PRODBG_MENU_DEBUG_START, PRODBG_KEY_F5, 0, 0 },
    { "Step In", PRODBG_MENU_DEBUG_STEP_IN, PRODBG_KEY_F11, 0, 0 },
    { "Step Over", PRODBG_MENU_DEBUG_STEP_OVER, PRODBG_KEY_F10, 0, 0 },
    { "Toggle Breakpoint", PRODBG_MENU_DEBUG_TOGGLE_BREAKPOINT, PRODBG_KEY_F9, 0, 0 },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDMenuItem g_popupMenu[] =
{
    { "Change View", PRODBG_MENU_SUB_MENU, 0, 0, 0 },
    { "Split Horizontally", PRODBG_MENU_SUB_MENU, 0, 0, 0 },
    { "Split Vertically", PRODBG_MENU_SUB_MENU, 0, 0, 0 },
    // { "New Tab (not implemented)", PRODBG_MENU_SUB_MENU, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
};
