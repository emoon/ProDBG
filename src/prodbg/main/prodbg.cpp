#include "api/plugin_instance.h"
#include "core/core.h"
#include "core/log.h"
#include "core/math.h"
#include "core/plugin_handler.h"
#include "session/session.h"
#include "settings.h"
//#include <imgui.h>
#include "ui/imgui_setup.h"
#include "ui/menu.h"
#include "ui/dialogs.h"
#include "ui/cursor.h"
#include "ui/ui_render.h"
#include "ui/ui_statusbar.h"
#include "input/input_state.h"

#include <bgfx.h>
#include <bgfxplatform.h>
#include <stdio.h>
#include <stdlib.h>
#include <bx/timer.h>

#ifdef PRODBG_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <remotery.h>
#include <assert.h>

// TODO: Fix me

int Window_buildPluginMenu(PluginData** plugins, int count);
void Window_addMenu(const char* name, PDMenuItem* items, uint32_t idOffset);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Context
{
    InputState inputState;
    int width;
    int height;
    uint64_t time;
    Session* session;   // one session right now
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Context s_context;
//Remotery* s_remotery;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* s_plugins[] =
{
    "sourcecode_plugin",
    "disassembly_plugin",
    "locals_plugin",
    "callstack_plugin",
    "registers_plugin",
    "breakpoints_plugin",
    "hex_memory_plugin",
    "console_plugin",
    "c64_vice_plugin",
#ifdef PRODBG_MAC
    "lldb_plugin",
#endif
#ifdef PRODBG_WIN
    "dbgeng_plugin",
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loadLayout(Session* session, float width, float height)
{
    // TODO: Fix convesion

    if (Session_loadLayout(session, "data/current_layout.json", (int)width, (int)height))
        return;

    if (Session_loadLayout(session, "data/default_layout.json", (int)width, (int)height))
        return;

    Session_createDockingGrid(session, (int)width, (int)height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t findMenuIdRange(PDMenu* menu)
{
    uint32_t idStart = uint32_t(~0);
    uint32_t idEnd = 0;

    while (menu->name)
    {
        PDMenuItem* menuItems = menu->items;

        while (menuItems->name)
        {
            const uint16_t id = menuItems->id;

            if (id > idEnd)
                idEnd = id;

            if (id < idStart)
                idStart = id;

            menuItems++;
        }

        menu++;
    }

    return (idEnd << 16) | idStart;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void createMenusForPlugins()
{
    int count = 0;

    PluginData** plugins = PluginHandler_getPlugins(&count);

    uint32_t menuIdStart = PRODBG_MENU_PLUGINS_START;

    for (int i = 0; i < count; ++i)
    {
        PluginData* pluginData = plugins[i];

        if (!strstr(pluginData->type, PD_BACKEND_API_VERSION))
            continue;

        PDBackendPlugin* plugin = (PDBackendPlugin*)pluginData->plugin;

        if (!plugin)
            continue;

        if (!plugin->registerMenu)
            continue;

        PDMenu* menus = plugin->registerMenu();
        uint32_t menuRange = findMenuIdRange(menus);

        while (menus->name)
        {
            Window_addMenu(menus->name, menus->items, menuIdStart);
            menus++;
        }

        pluginData->menuStart = menuIdStart;
        pluginData->menuEnd = menuIdStart + (menuRange >> 16);

        menuIdStart += (menuRange >> 16);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_create(void* window, int width, int height)
{
    Context* context = &s_context;
    //Rect settingsRect;

    context->session = Session_create();
    context->time = bx::getHPCounter();

#if PRODBG_USING_DOCKING
    loadLayout(context->session, (float)width, (float)(height - g_statusBarSize));
#endif

    /*
       if (RMT_ERROR_NONE != rmt_CreateGlobalInstance(&s_remotery))
       {
        log_error("Unable to setup Remotery");
        return;
       }
     */

    //Settings_getWindowRect(&settingsRect);
    //width = settingsRect.width;
    //height = settingsRect.height;

    (void)window;

    for (uint32_t i = 0; i < sizeof_array(s_plugins); ++i)
    {
        PluginHandler_addPlugin(OBJECT_DIR, s_plugins[i]);
    }

#if BX_PLATFORM_OSX
    bgfx::osxSetNSWindow(window);
#elif BX_PLATFORM_WINDOWS
    bgfx::winSetHwnd((HWND)window);
#else
    //bgfx::winSetHwnd(0);
#endif


    bgfx::init();
    bgfx::reset(width, height);
    bgfx::setViewSeq(0, true);

    context->width = width;
    context->height = height;

    IMGUI_setup(width, height);

    Cursor_init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDock(Context* context)
{
    UIDockingGrid* grid = Session_getDockingGrid(context->session);
    const InputState* inputState = &context->inputState;

    switch (UIDock_getSizingState(grid))
    {
        case UIDockSizerDir_None:
        {
            Cunsor_setType(CursorType_Default);
            break;
        }

        case UIDockSizerDir_Horz:
        {
            Cunsor_setType(CursorType_SizeHorizontal);
            break;
        }

        case UIDockSizerDir_Vert:
        {
            Cunsor_setType(CursorType_SizeVertical);
            break;
        }

        case UIDockSizerDir_Both:
        {
            Cunsor_setType(CursorType_SizeAll);
            break;
        }
    }

    UIDock_update(grid, inputState);
    UIDock_renderSizers(grid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_update()
{
    Context* context = &s_context;

    rmt_ScopedCPUSample(ProDBG_update);

    bgfx::setViewRect(0, 0, 0, (uint16_t)context->width, (uint16_t)context->height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR_BIT | BGFX_CLEAR_DEPTH_BIT, 0x101010ff, 1.0f, 0);
    bgfx::submit(0);

    uint64_t currentTime = bx::getHPCounter();
    uint64_t deltaTick = currentTime - context->time;
    context->time = currentTime;

    float deltaTimeMs = (float)(((double)deltaTick) / (double)bx::getHPFrequency());

#if PRODBG_USING_DOCKING
    updateDock(context);
#endif
    {
        rmt_ScopedCPUSample(IMGUI_preUpdate);
        IMGUI_preUpdate(&context->inputState, deltaTimeMs);
    }

    // TODO: Support multiple sessions

    {
        rmt_ScopedCPUSample(Session_update);
        Session_update(context->session);
    }

    UIStatusBar_render();

    //renderTest();

    /*

       bool show = true;

       ImGui::Begin("ImGui Test", &show, ImVec2(550, 480), true, ImGuiWindowFlags_ShowBorders);

       if (ImGui::Button("Test0r testing!"))
       {
        printf("test\n");
       }

       ImGui::End();
     */

    {
        rmt_ScopedCPUSample(IMGUI_postUpdate);
        IMGUI_postUpdate();
    }


    {
        rmt_ScopedCPUSample(bgfx_frame);
        bgfx::frame();
    }
}

// Temprory test for monkey

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setWindowSize(int width, int height)
{
    Context* context = &s_context;

    context->width = width;
    context->height = height;

    bgfx::reset(width, height);
    IMGUI_updateSize(width, height);

#if PRODBG_USING_DOCKING
    UIDock_updateSize(Session_getDockingGrid(context->session), width, height - (int)g_statusBarSize);
#endif

    ProDBG_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_applicationLaunched()
{
    int pluginCount = 0;
    Window_buildPluginMenu(PluginHandler_getPlugins(&pluginCount), pluginCount);
    createMenusForPlugins();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_destroy()
{
    Context* context = &s_context;

    //rmt_DestroyGlobalInstance(s_remotery);

    UIDock_saveLayout(Session_getDockingGrid(context->session),
                      "data/current_layout.json", (float)context->width, (float)(context->height - g_statusBarSize));

    Settings_save();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_timedUpdate()
{
    ProDBG_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onLoadRunExec(Session* session, const char* filename)
{
    PluginData* pluginData = PluginHandler_findPlugin(0, "lldb_plugin", "LLDB Mac", true);

    if (!pluginData)
    {
        log_error("Unable to find LLDB Mac backend\n");
        return;
    }

    Session_startLocal(session, (PDBackendPlugin*)pluginData->plugin, filename);

    // Temp test
    // Session_startLocal(context->session, (PDBackendPlugin*)pluginData->plugin, "t2-output/macosx-clang-debug-default/ProDBG.app/Contents/MacOS/prodbg");
    // Session_startLocal(context->session, (PDBackendPlugin*)pluginData->plugin, OBJECT_DIR "/crashing_native");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Events

void ProDBG_event(int eventId)
{
    Context* context = &s_context;

    int count;

    PluginData** pluginsData = PluginHandler_getPlugins(&count);

    log_info("eventId 0x%x\n", eventId);

    Vec2 mousePos = context->inputState.mousePos;

#if PRODBG_USING_DOCKING
    if (eventId & PRODBG_MENU_POPUP_SPLIT_HORZ_SHIFT)
    {
        UIDockingGrid* grid = Session_getDockingGrid(context->session);
        UIDock* dockAtMouse = UIDock_getDockAt(grid, (int)mousePos.x, (int)mousePos.y);

        eventId &= (PRODBG_MENU_POPUP_SPLIT_HORZ_SHIFT - 1);

        ViewPluginInstance* instance = PluginInstance_createViewPlugin(pluginsData[eventId]);
        UIDock_splitHorizontal(Session_getDockingGrid(context->session), dockAtMouse, instance);

        Session_addViewPlugin(context->session, instance);
        return;
    }

    if (eventId & PRODBG_MENU_POPUP_SPLIT_VERT_SHIFT)
    {

        UIDockingGrid* grid = Session_getDockingGrid(context->session);
        UIDock* dockAtMouse = UIDock_getDockAt(grid, (int)mousePos.x, (int)mousePos.y);

        eventId &= (PRODBG_MENU_POPUP_SPLIT_VERT_SHIFT - 1);

        ViewPluginInstance* instance = PluginInstance_createViewPlugin(pluginsData[eventId]);
        UIDock_splitVertical(Session_getDockingGrid(context->session), dockAtMouse, instance);

        Session_addViewPlugin(context->session, instance);
        return;
    }

#endif

    // TODO: This code really needs to be made more robust.

    if (eventId >= PRODBG_MENU_PLUGIN_START && eventId < PRODBG_MENU_PLUGIN_START + 9)
    {
        ViewPluginInstance* instance = PluginInstance_createViewPlugin(pluginsData[eventId - PRODBG_MENU_PLUGIN_START]);

        UIDockingGrid* grid = Session_getDockingGrid(context->session);
        UIDock* dockAtMouse = UIDock_getDockAt(grid, 0, 0);
        UIDock_splitVertical(Session_getDockingGrid(context->session), dockAtMouse, instance);

        Session_addViewPlugin(context->session, instance);
        return;
    }


    switch (eventId)
    {
        case PRODBG_MENU_FILE_OPEN_AND_RUN_EXE:
        {
            char filename[4096];

            if (Dialog_open(filename))
            {
                onLoadRunExec(context->session, filename);
            }

            break;
        }

        case PRODBG_MENU_FILE_OPEN_SOURCE:
        {
            char filename[4096];

            if (Dialog_open(filename))
            {
                Session_loadSourceFile(context->session, filename);
            }

            break;
        }

        case PRODBG_MENU_DEBUG_BREAK:
        {
            Session_action(context->session, PDAction_break);
            log_info("trying to break...\n");
            break;
        }

        case PRODBG_MENU_DEBUG_ATTACH_TO_REMOTE:
        {
            Session_startRemote(context->session, "127.0.0.1", 1340);
            break;
        }

        case PRODBG_MENU_DEBUG_TOGGLE_BREAKPOINT:
        {
            Session_toggleBreakpointCurrentLine(context->session);
            break;
        }

        case PRODBG_MENU_DEBUG_STEP_OVER:
        {
            Session_stepOver(context->session);
            break;
        }

        case PRODBG_MENU_DEBUG_STEP_IN:
        {
            Session_stepIn(context->session);
            break;
        }
    }

    if (eventId >= PRODBG_MENU_PLUGINS_START)
    {
        Session_onMenu(context->session, eventId);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_scroll(const PDMouseWheelEvent& wheelEvent)
{
    Context* context = &s_context;
    context->inputState.scrollEvent = wheelEvent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setMousePos(float x, float y)
{
    Context* context = &s_context;

    context->inputState.mousePos.x = x;
    context->inputState.mousePos.y = y;

    IMGUI_setInputState(&context->inputState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setMouseState(int button, int state)
{
    Context* context = &s_context;
    (void)button;

    InputState* inputState = &context->inputState;

    // TODO: Proper mouse support

    inputState->mouseDown[0] = !!state;

    IMGUI_setInputState(inputState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyDown(int key, int modifier)
{
    IMGUI_setKeyDown(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyUp(int key, int modifier)
{
    IMGUI_setKeyUp(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_addChar(unsigned short c)
{
    IMGUI_addInputCharacter(c);
}




