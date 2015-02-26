#include "ui_statusbar.h"
#include <stdio.h>
#include <stdarg.h>
#include <imgui.h>

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
	/*
    ImGui::SetNextWindowPos(ImVec2(instance->rect.x, instance->rect.y));
    ImGui::SetNextWindowSize(ImVec2(instance->rect.width - 4, instance->rect.height - 4));

    ImGui::Begin(data->name, &data->showWindow, ImVec2(0, 0), true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::End();
    */
}

