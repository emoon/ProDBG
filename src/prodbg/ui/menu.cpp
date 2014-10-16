#include "menu.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MenuDescriptor g_fileMenu[] =
{
    { "Open and run executable...", PRODBG_MENU_FILE_OPEN_AND_RUN_EXE, 'o', PRODBG_KEY_COMMAND, PRODBG_KEY_CTRL },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MenuDescriptor g_debugMenu[] =
{
    { "Step Into", PRODBG_MENU_DEBUG_STEP_INTO, '8', PRODBG_KEY_COMMAND, PRODBG_KEY_CTRL },
    { "Step Over", PRODBG_MENU_DEBUG_STEP_OVER, '9', PRODBG_KEY_COMMAND, PRODBG_KEY_CTRL },
    { "Step Out", PRODBG_MENU_DEBUG_STEP_OUT, '0', PRODBG_KEY_COMMAND, PRODBG_KEY_CTRL },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

