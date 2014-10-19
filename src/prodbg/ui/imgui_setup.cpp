#include "imgui_setup.h"
#include "imgui/imgui.h"
#include "core/core.h"
//#include <entry.h>
#include <stdio.h>
#include <pd_keys.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void imguiDummyRender(ImDrawList** const cmd_lists, int cmd_lists_count)
{
    (void)cmd_lists;
    (void)cmd_lists_count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setup(int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;
    io.PixelCenterOffset = 0.5f;

    io.RenderDrawListsFn = imguiDummyRender;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void IMGUI_preUpdate(float x, float y, int mouseLmb, int keyDown, int keyMod)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;    // TODO: Fix me
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = mouseLmb;
    io.keyDown = keyDown;
    io.keyMod = keyMod;

    ImGui::NewFrame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setMouse(float x, float y, int mouseLmb)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = mouseLmb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setKeyDown(int key, int modifier)
{
    ImGuiIO& io = ImGui::GetIO();
	assert(key >= 0 && key <= (int)sizeof_array(io.KeysDown));
	io.KeysDown[key] = true;
	io.KeyCtrl = modifier & PDKEY_CTRL;
	io.KeyShift = modifier & PDKEY_SHIFT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setKeyUp(int key, int modifier)
{
    ImGuiIO& io = ImGui::GetIO();
	assert(key >= 0 && key <= (int)sizeof_array(io.KeysDown));
	io.KeysDown[key] = false;
	io.KeyCtrl = modifier & PDKEY_CTRL;
	io.KeyShift = modifier & PDKEY_SHIFT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_postUpdate()
{
    ImGui::Render();
}



