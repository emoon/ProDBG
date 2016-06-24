#include "bgfx_plugin_ui.h"
#include "pd_ui.h"
#include "pd_view.h"
#include "ui_host.h"
#include "imgui_setup.h"
#include <imgui.h>
#include <assert.h>

#include <bgfx/bgfx.h>
#include "ui_render.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef PRODBG_UNIX
#include <X11/Xlib.h>
#endif

#include <bgfx/bgfxplatform.h>

struct ImGuiWindow;

// TODO: Move to settings
const int s_borderSize = 4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Context {
    int width;
    int height;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Context s_context;

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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::preUpdate() {
    const float deltaTime = 1.0f / 60.f; // TODO: Calc correct dt

    bgfx::setViewRect(0, 0, 0, (uint16_t)s_context.width, (uint16_t)s_context.height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000f0f0f, 1.0f, 0);
    // TODO(marco): is 2 the correct index here??
    bgfx::submit(0, UIRender_getProgramHandle(2));

    imgui_pre_update(deltaTime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::postUpdate() {
    renderStatusBar(m_statusText, (float)m_statusSize);
    imgui_post_update();

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
    imgui_setup(width, height);

    s_context.width = width;
    s_context.height = height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::destroy() {
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_set_window_size(int width, int height) {
    Context* context = &s_context;

    if (context->width != width || context->height != height) {
        context->width = width;
        context->height = height;

        bgfx::reset((uint32_t)width, (uint32_t)height);
        imgui_update_size(width, height);
    }
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
