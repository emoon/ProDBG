#include "ui_statusbar.h"
#include <stdio.h>
#include <stdarg.h>
#include <imgui.h>

float g_statusBarSize = 18.0f;

static char s_statusText[4096];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIStatusBar_setText(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
	vsprintf(s_statusText, format, ap);
    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIStatusBar_render()
{
	const ImGuiIO& io = ImGui::GetIO();
	ImVec2 size = io.DisplaySize;
	float yPos = size.y - g_statusBarSize;

	ImGui::SetNextWindowPos(ImVec2(0.0f, yPos));
	ImGui::SetNextWindowSize(ImVec2(size.x, g_statusBarSize));

	bool show = true;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(40, 40, 40));

    ImGui::Begin("", &show, ImVec2(0, 0), true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::SetCursorPos(ImVec2(2.0f, 4.0f));
	ImGui::Text("Status: %s", s_statusText);
    ImGui::End();

	ImGui::PopStyleColor();
}

