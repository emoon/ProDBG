#include "api/plugin_instance.h"
#include "core/core.h"
#include "core/log.h"
#include "core/math.h"
#include "core/plugin_handler.h"
#include "session/session.h"
#include "settings.h"
#include "ui/imgui/imgui.h"
#include "ui/imgui_setup.h"
#include "ui/ui_layout.h"
#include "ui/menu.h"
#include "ui/dialogs.h"

#include <bgfx.h>
#include <bgfxplatform.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PRODBG_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// TODO: Fix me

int Window_buildPluginMenu(PluginData** plugins, int count);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Context
{
    int width;
    int height;
    float mouseX;
    float mouseY;
    int mouseLmb;
    int keyDown;
    int keyMod;
    Session* session;   // one session right now
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Context s_context;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* s_plugins[] =
{
    "sourcecode_plugin",
    "disassembly_plugin",
    "locals_plugin",
    "callstack_plugin",
    "registers_plugin",
    "breakpoints_plugin",
#ifdef PRODBG_MAC
    "lldb_plugin",
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setLayout(UILayout* layout)
{
    Context* context = &s_context;
    IMGUI_preUpdate(context->mouseX, context->mouseY, context->mouseLmb, context->keyDown, context->keyMod);
    Session_setLayout(context->session, layout, context->width, context->height);
    IMGUI_postUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void loadLayout()
{
    UILayout layout;

    if (UILayout_loadLayout(&layout, "data/current_layout.yaml"))
    {
        setLayout(&layout);
        return;
    }

    if (UILayout_loadLayout(&layout, "data/default_layout.yaml"))
        setLayout(&layout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_create(void* window, int width, int height)
{
    Context* context = &s_context;
    //Rect settingsRect;

    log_info("create\n");

    context->session = Session_create();

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

    loadLayout();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_update()
{
    Context* context = &s_context;

    bgfx::setViewRect(0, 0, 0, (uint16_t)context->width, (uint16_t)context->height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR_BIT | BGFX_CLEAR_DEPTH_BIT, 0x101010ff, 1.0f, 0);
    bgfx::submit(0);

    IMGUI_preUpdate(context->mouseX, context->mouseY, context->mouseLmb, context->keyDown, context->keyMod);

    // TODO: Support multiple sessions

    Session_update(context->session);

    /*

       bool show = true;

       ImGui::Begin("ImGui Test", &show, ImVec2(550, 480), true, ImGuiWindowFlags_ShowBorders);

       if (ImGui::Button("Test0r testing!"))
       {
        printf("test\n");
       }

       ImGui::End();
     */

    IMGUI_postUpdate();

    bgfx::frame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setWindowSize(int width, int height)
{
    Context* context = &s_context;

    context->width = width;
    context->height = height;

    bgfx::reset(width, height);
    IMGUI_setup(width, height);
    ProDBG_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_applicationLaunched()
{
#ifdef PRODBG_MAC
    int pluginCount = 0;
    printf("building menu!\n");
    Window_buildPluginMenu(PluginHandler_getPlugins(&pluginCount), pluginCount);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_destroy()
{
    UILayout layout;
    Context* context = &s_context;

    Session_getLayout(context->session, &layout, context->width, context->height);
    UILayout_saveLayout(&layout, "data/current_layout.yaml");

    Settings_save();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_timedUpdate()
{
    ProDBG_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onLoadRunExec()
{
    Context* context = &s_context;
    PluginData* pluginData = PluginHandler_findPlugin(0, "lldb_plugin", "LLDB Mac", true);

    if (!pluginData)
    {
        log_error("Unable to find LLDB Mac backend\n");
        return;
    }

    // Hacky hack

    Session_startLocal(context->session, (PDBackendPlugin*)pluginData->plugin, "t2-output/macosx-clang-debug-default/ProDBG.app/Contents/MacOS/prodbg");
    //Session_startLocal(context->session, (PDBackendPlugin*)pluginData->plugin, OBJECT_DIR "/crashing_native");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Events

void ProDBG_event(int eventId)
{
    Context* context = &s_context;

    int count;

    PluginData** pluginsData = PluginHandler_getPlugins(&count);

    // TODO: This code really needs to be made more robust.

    if (eventId >= PRODBG_MENU_PLUGIN_START && eventId < PRODBG_MENU_PLUGIN_START + 9)
    {
        ViewPluginInstance* instance = PluginInstance_createViewPlugin(pluginsData[eventId - PRODBG_MENU_PLUGIN_START]);
        Session_addViewPlugin(context->session, instance);
        return;
    }

    switch (eventId)
    {
        case PRODBG_MENU_FILE_OPEN_AND_RUN_EXE:
        {
            onLoadRunExec();
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

        case PRODBG_MENU_DEBUG_TOGGLE_BREAKPOINT:
        {
        	Session_toggleBreakpointCurrentLine(context->session);
            break;
        }

        case PRODBG_MENU_DEBUG_STEP_OVER:
        {
        	log_info("stepOver\n");
            break;
        }

        case PRODBG_MENU_DEBUG_STEP_IN:
        {
        	Session_stepIn(context->session);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_scroll(float deltaX, float deltaY, int flags)
{
    (void)deltaX;
    (void)deltaY;
    (void)flags;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setMousePos(float x, float y)
{
    Context* context = &s_context;

    context->mouseX = x;
    context->mouseY = y;

	IMGUI_setMouse(x, y, context->mouseLmb);

    ProDBG_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setMouseState(int button, int state)
{
    Context* context = &s_context;
    (void)button;

    context->mouseLmb = state;

	IMGUI_setMouse(context->mouseX, context->mouseY, state);

    ProDBG_update();
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



