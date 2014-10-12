#include "plugin.h"
#include "api/include/pd_ui.h"
#include "api/include/pd_view.h"
#include "api/plugin_instance.h"
#include "core/alloc.h"
#include "core/log.h"
#include "imgui/imgui.h"
#include <string.h>
#include <stdio.h>

struct ImGuiWindow;
static int windowIdCount = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivateData
{
    ImGuiWindow* window;
    const char name[16];
    bool showWindow;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void columns(int count, const char* id, int border)
{
    ImGui::Columns(count, id, !!border);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void nextColumn()
{
    ImGui::NextColumn();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int button(const char* label)
{
    return ImGui::Button(label);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    ImGui::TextV(format, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int buttonSize(const char* label, int width, int height, int repeatWhenHeld)
{
    return ImGui::Button(label, ImVec2(width, height), !!repeatWhenHeld);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_init(ViewPluginInstance* pluginInstance)
{
	PrivateData* data = (PrivateData*)alloc_zero(sizeof(PrivateData));
	PDUI* uiInstance = &pluginInstance->ui;

	data->showWindow = true;

    memset(uiInstance, 0, sizeof(PDUI));

    uiInstance->columns = columns;
    uiInstance->nextColumn = nextColumn;
    uiInstance->text = text;
    uiInstance->button = button;
    uiInstance->buttonSize = buttonSize;

    uiInstance->privateData = alloc_zero(sizeof(PrivateData));

    // TODO: Must have in data here if user splited window or just created a new one
	// TODO: Do something better here when assigning names to windows
	sprintf((char*)data->name, "%d\n", windowIdCount++);

    data->window = ImGui::FindOrCreateWindow(data->name, ImVec2(400, 400), 0);

	ImGui::SetWindowRect(data->window, ImVec2(10, 20), ImVec2(100, 100));

	uiInstance->privateData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginUIState PluginUI_updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer)
{
	PDUI* uiInstance = &instance->ui;
	PrivateData* data = (PrivateData*)uiInstance->privateData;

	ImGui::BeginWithWindow(data->window, data->name, &data->showWindow, ImVec2(0, 0), true, 0);

	instance->plugin->update(instance->userData, uiInstance, reader, writer);

	ImGui::End();

	if (!data->showWindow)
		return PluginUIState_CloseView;

	return PluginUIState_None;
}



