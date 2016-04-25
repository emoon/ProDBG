#include "bgfx_plugin_ui.h"
#include "pd_ui.h"
#include "pd_view.h"
//#include "api/plugin_instance.h"
//#include "core/plugin_handler.h"
//#include "core/alloc.h"
//#include "core/log.h"
//#include "core/math.h"
//#include "core/input_state.h"
//#include "core/plugin_io.h"
//#include "core/service.h"
//#include "ui_dock.h"
#include "ui_host.h"
#include "imgui_setup.h"
#include <imgui.h>
#include <assert.h>
#include "cursor.h"

//#include <session/session.h>
//#include <foundation/apple.h>
//#include <foundation/string.h>
#include <bgfx.h>
//#include "core/input_state.h"
//#include "ui/bgfx/cursor.h"
//#include <foundation/string.h>
//#include "i3wm_docking.h"
#include "ui_render.h"
#include <jansson.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef PRODBG_UNIX
#include <X11/Xlib.h>
#endif

#include <bgfxplatform.h>

struct ImGuiWindow;

// TODO: Move to settings
const int s_borderSize = 4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Context {
    int width;
    int height;
    //InputState inputState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Context s_context;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void BgfxPluginUI::init(ViewPluginInstance* pluginInstance) {
    PrivateData* data = 0;

    PDUI* uiInstance = &pluginInstance->ui;

    *uiInstance = *s_uiFuncs;

    uiInstance->private_data = alloc_zero(sizeof(PrivateData));

    data = (PrivateData*)uiInstance->private_data;

    data->name = buildName(pluginInstance->plugin->name, pluginInstance->count);
    data->window = 0;
    data->showWindow = true;
    data->title = 0;

    pluginInstance->name = data->name;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback from the docking system

/*
void updateWindowSize(void* user_data, int x, int y, int width, int height) {
    ViewPluginInstance* instance = (ViewPluginInstance*)user_data;

    instance->rect.x = x;
    instance->rect.y = y;
    instance->rect.width = width;
    instance->rect.height = height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

static void setCursorStyle(DockSysCursor cursor) {
    switch (cursor) {
        case DockSysCursor_SizeHorizontal:
            Cunsor_setType(CursorType_SizeHorizontal); break;
        case DockSysCursor_SizeVertical:
            Cunsor_setType(CursorType_SizeVertical); break;
        default:
            Cunsor_setType(CursorType_Default); break;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Move this code?

static void saveUserData(struct json_t* item, void* user_data) {
    ViewPluginInstance* view = (ViewPluginInstance*)user_data;

    if (!view->plugin)
        return;

    PDSaveState saveFuncs;
    PluginIO_initSaveJson(&saveFuncs);

    PluginData* pluginData = PluginHandler_getPluginData(view->plugin);

    assert(pluginData);

    const char* pluginName = view->plugin->name;
    const char* filename = pluginData->filename;

    json_object_set_new(item, "plugin_name", json_string(pluginName));
    json_object_set_new(item, "plugin_file", json_string(filename));

    PDViewPlugin* viewPlugin = (PDViewPlugin*)pluginData->plugin;

    if (!viewPlugin->save_state)
        return;

    json_t* array = json_array();

    saveFuncs.priv_data = array;

    viewPlugin->save_state(view->userData, &saveFuncs);

    json_object_set_new(item, "plugin_data", array);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

static void* loadUserData(struct json_t* item) {
    ViewPluginInstance* view = 0;

    const char* pluginName = json_string_value(json_object_get(item, "plugin_name"));
    const char* filename = json_string_value(json_object_get(item, "plugin_file"));

    // if this is the case we have no plugin created (empty window)

    if (!strcmp(pluginName, "") && !strcmp(filename, "")) {
        view = (ViewPluginInstance*)alloc_zero(sizeof(ViewPluginInstance));
    }else {
        PDLoadState loadFuncs;
        PluginIO_initLoadJson(&loadFuncs);

        PluginData* pluginData = PluginHandler_findPlugin(0, filename, pluginName, true);

        if (!pluginData)
            view = (ViewPluginInstance*)alloc_zero(sizeof(ViewPluginInstance));
        else
            view = g_pluginUI->createViewPlugin(pluginData);

        PDViewPlugin* viewPlugin = (PDViewPlugin*)pluginData->plugin;

        json_t* pluginJsonData = json_object_get(item, "plugin_data");

        if (pluginJsonData && viewPlugin && viewPlugin->load_state) {
            SessionLoadState load_state = { pluginJsonData, (int)json_array_size(pluginJsonData), 0 };
            loadFuncs.priv_data = &load_state;
            viewPlugin->load_state(view->userData, &loadFuncs);
        }
    }

    // TODO: Fi this: assuming one session

    Session** sessions = Session_getSessions();

    assert(sessions);
    assert(sessions[0]);

    Session_addViewPlugin(sessions[0], view);

    return view;
}

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static DockSysCallbacks s_dockSysCallbacks =
{
    updateWindowSize,
    setCursorStyle,
    saveUserData,
    loadUserData,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginUI::State BgfxPluginUI::updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer) {
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->private_data;

    float x = (float)instance->rect.x;
    float y = (float)instance->rect.y;
    float w = (float)instance->rect.width;
    float h = (float)instance->rect.height;

    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(w - s_borderSize, h - s_borderSize));

    // TODO: Cache this?

    char title[1024];

    if (!data->title)
        strcpy(title, data->name);
    else{
        sprintf(title, "%s %d - %s###%s%d",
                instance->plugin->name, instance->count,
                data->title, instance->plugin->name, instance->count);
    }

    ImGui::Begin(title, &data->showWindow, ImVec2(0, 0), true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    instance->plugin->update(instance->userData, uiInstance, reader, writer);

    ImGui::End();

    // Draw border

    if  (!data->showWindow)
        return CloseView;

    return None;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int BgfxPluginUI::getStatusBarSize() {
    return m_statusSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void renderStatusBar(const char* text, float statusSize) {
    const ImGuiIO& io = ImGui::GetIO();
    ImVec2 size = io.DisplaySize;
    float yPos = size.y - statusSize;

    ImGui::SetNextWindowPos(ImVec2(0.0f, yPos));
    ImGui::SetNextWindowSize(ImVec2(size.x, statusSize));

    bool show = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(40, 40, 40));

    ImGui::Begin("", &show, ImVec2(0, 0), true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetCursorPos(ImVec2(2.0f, 4.0f));
    ImGui::Text("Status: %s", text);
    ImGui::End();

    ImGui::PopStyleColor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::setStatusTextNoFormat(const char* text) {
    strcpy(m_statusText, text);
}

/*

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDock(UIDockingGrid* grid) {
    switch (UIDock_getSizingState(grid)) {
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

    UIDock_update(grid, InputState_getState());
    UIDock_render(grid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDocking(Session* session) {
    InputState* state = InputState_getState();

    int mx = (int)state->mousePos.x;
    int my = (int)state->mousePos.y;

    struct ViewPluginInstance* view = Session_getViewAt(session, mx, my, 0);

    docksys_set_mouse(view, mx, my, state->mouseDown[0]);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::preUpdate() {
    const float deltaTime = 1.0f / 60.f; // TODO: Calc correct dt

    bgfx::setViewRect(0, 0, 0, (uint16_t)s_context.width, (uint16_t)s_context.height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000f0f0f, 1.0f, 0);
    bgfx::submit(0);

    IMGUI_preUpdate(deltaTime);
    //InputState_update(deltaTime);

	/*
    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i) {
        Session* session = sessions[i];
        updateDocking(session);
    }

    docksys_update();
    */
}

/*

static PosColorVertex* fill_rectBorder(PosColorVertex* verts, IntRect* rect, uint32_t color) {
    const float x0 = (float)rect->x;
    const float y0 = (float)rect->y;
    const float x1 = (float)rect->width + x0;
    const float y1 = (float)rect->height + y0;

    // First triangle

    verts[0].x = x0;
    verts[0].y = y0;
    verts[0].color = color;

    verts[1].x = x1;
    verts[1].y = y0;
    verts[1].color = color;

    verts[2].x = x1;
    verts[2].y = y1;
    verts[2].color = color;

    // Second triangle

    verts[3].x = x0;
    verts[3].y = y0;
    verts[3].color = color;

    verts[4].x = x1;
    verts[4].y = y1;
    verts[4].color = color;

    verts[5].x = x0;
    verts[5].y = y1;
    verts[5].color = color;

    verts += 6;

    return verts;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void renderBorders(Session* session) {
    int count = 0;
    ViewPluginInstance** views = Session_getViewPlugins(session, &count);

    bgfx::TransientVertexBuffer tvb;

    const uint32_t vertexCount = (uint32_t)count * 2 * 6;

    UIRender_allocPosColorTb(&tvb, vertexCount);
    PosColorVertex* verts = (PosColorVertex*)tvb.data;

    // TODO: Use settings for colors

    const uint32_t colorDefalut = (0x40 << 16) | (0x40 << 8) | 0x40;
    const uint32_t colorHigh = (0x60 << 16) | (0x60 << 8) | 0x60;

    for (int i = 0; i < count; ++i) {
        IntRect t = views[i]->rect;

        IntRect t0 = {{{ t.x + t.width - s_borderSize, t.y, s_borderSize, t.height }}};
        IntRect t1 = {{{ t.x, t.y + t.height - s_borderSize, t.width, s_borderSize }}};

        verts = fill_rectBorder(verts, &t0, colorDefalut);
        verts = fill_rectBorder(verts, &t1, colorDefalut);
    }

    bgfx::setState(0
                   | BGFX_STATE_RGB_WRITE
                   | BGFX_STATE_ALPHA_WRITE
                   | BGFX_STATE_MSAA);

    UIRender_posColor(&tvb, 0, vertexCount);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::postUpdate() {
    renderStatusBar(m_statusText, (float)m_statusSize);
    IMGUI_postUpdate();

	/*
    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i) {
        Session* session = sessions[i];
        renderBorders(session);
    }
    */

    bgfx::frame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::create(void* windowHandle, int width, int height) {
    //docksys_set_callbacks(&s_dockSysCallbacks);
	cursor_init();
#ifdef PRODBG_WIN
    bgfx::winSetHwnd((HWND)windowHandle);
#elif PRODBG_MAC
    bgfx::osxSetNSWindow(windowHandle);
#elif PRODBG_UNIX
    bgfx::x11SetDisplayWindow(XOpenDisplay(0), (uint32_t)(uintptr_t)windowHandle);
#endif
    bgfx::init();
    bgfx::reset((uint32_t)width, (uint32_t)height);
    bgfx::setViewSeq(0, true);
    IMGUI_setup(width, height);

    s_context.width = width;
    s_context.height = height;

    //Service_register(&g_serviceMessageFuncs, PDMESSAGEFUNCS_GLOBAL);
    //Service_register(&g_dialogFuncs, PDDIALOGS_GLOBAL);

    //Cursor_init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::destroy() {
}

// It's a bit weird to have the code like this here. To be cleaned up

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_set_mouse_pos(float x, float y) {
    IMGUI_setMousePos(x, y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_set_mouse_state(int button, int state) {
    IMGUI_setMouseState(state);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_set_scroll(float x, float y) {
    (void)x;
    IMGUI_setScroll(y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_key_down(int key, int modifier) {
    //InputState* state = InputState_getState();

    //state->keysDown[key] = true;
    //state->modifiers = modifier;

    IMGUI_setKeyDown(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyDownMods(int modifier) {
    //InputState* state = InputState_getState();
    //state->modifiers = modifier;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_key_up(int key, int modifier) {
	/*
    InputState* state = InputState_getState();

    state->keysDown[key] = false;
    state->modifiers = modifier;
    */

    IMGUI_setKeyUp(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_add_char(unsigned short c) {
    IMGUI_addInputCharacter(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_set_window_size(int width, int height) {
    Context* context = &s_context;

    context->width = width;
    context->height = height;

    bgfx::reset((uint32_t)width, (uint32_t)height);
    IMGUI_updateSize(width, height);

	/*
    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i) {
        Session* session = sessions[i];
        docksys_update_size(width, height - (int)g_pluginUI->getStatusBarSize());
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_create() {
    g_pluginUI = new BgfxPluginUI;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_create_window(void* window, int width, int height) {
    g_pluginUI->create(window, width, height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_pre_update() {
    g_pluginUI->preUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_post_update() {
    g_pluginUI->postUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_destroy() {
    g_pluginUI->destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_imgui_set_window_pos(float x, float y) {
    ImGui::SetNextWindowPos(ImVec2(x, y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_imgui_set_window_size(float w, float h) {
    ImGui::SetNextWindowSize(ImVec2(w - s_borderSize, h - s_borderSize));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_test_menu(int show) {
	if (show)
		ImGui::OpenPopup("select");

	if (ImGui::BeginPopup("select"))
	{
		ImGui::Text("Aquarium");

		if (ImGui::BeginMenu("Sub-menu"))
		{
			ImGui::MenuItem("Click me");
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* s_temp;

extern "C" void bgfx_set_context(void* context) {
	s_temp = context;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void* bgfx_get_context() {
	return s_temp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" float bgfx_get_screen_width() {
	return (float)s_context.width;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" float bgfx_get_screen_height() {
	return (float)s_context.height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
