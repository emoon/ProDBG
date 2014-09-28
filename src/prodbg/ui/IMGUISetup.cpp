#include "IMGUISetup.h"
#include "imgui/imgui.h"
//#include <entry.h>
#include <stdio.h>

namespace prodbg
{

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
	io.DeltaTime = 1.0f/60.0f;
	io.PixelCenterOffset = 0.5f;

	io.RenderDrawListsFn = imguiDummyRender;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
void IMGUI_preUpdate(entry::MouseState* mouseState)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = 1.0f / 60.0f;	// TODO: Fix me 
	io.MousePos = ImVec2(mouseState->m_mx, mouseState->m_my);
	io.MouseDown[0] = mouseState->m_buttons[1];
	io.MouseDown[1] = mouseState->m_buttons[0];

	//printf("mousePos %f %f - %d %d\n", io.MousePos.x, io.MousePos.y, io.MouseDown[0], io.MouseDown[1]);

	ImGui::NewFrame();
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_postUpdate()
{
	ImGui::Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
