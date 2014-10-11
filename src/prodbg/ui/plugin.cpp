#include "plugin.h"
#include "api/include/pd_ui.h"
#include "api/plugin_instance.h"
#include "core/log.h"
#include "core/alloc.h"
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

static int buttonSize(const char* label, int width, int height, int repeatWhenHeld)
{
    return ImGui::Button(label, ImVec2(width, height), !!repeatWhenHeld);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_init(ViewPluginInstance* pluginInstance)
{
	PrivateData* data = (PrivateData*)alloc_zero(sizeof(PrivateData));
	PDUI* uiInstance = &pluginInstance->ui;

    memset(uiInstance, 0, sizeof(PDUI));

    uiInstance->columns = columns;
    uiInstance->nextColumn = nextColumn;
    uiInstance->button = button;
    uiInstance->buttonSize = buttonSize;

    uiInstance->privateData = alloc_zero(sizeof(PrivateData));

    // TODO: Must have in data here if user splited window or just created a new one
	// TODO: Do something better here when assigning names to windows
	sprintf((char*)data->name, "%d\n", windowIdCount++);

    data->window = ImGui::FindOrCreateWindow(data->name, ImVec2(0, 0), 0);

	uiInstance->privateData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


