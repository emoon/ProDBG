#include "plugin.h"
#include "api/include/pd_ui.h"
#include "api/include/pd_view.h"
#include "api/plugin_instance.h"
#include "core/alloc.h"
#include "core/log.h"
#include "core/math.h"
#include "imgui/imgui.h"
#include <string.h>
#include <stdio.h>

struct ImGuiWindow;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivateData
{
    ImGuiWindow* window;
    const char* name;
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

char* buildName(const char* pluginName, int id)
{
    char idBuffer[32];
    int nameLen = (int)strlen(pluginName);

    sprintf(idBuffer, "%d", id);

    char* name = (char*)alloc_zero(nameLen + (int)strlen(idBuffer) + 2); // + 2 for space and end marker

    sprintf(name, "%s %s", pluginName, idBuffer);

    return name;
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
    data->name = buildName(pluginInstance->plugin->name, pluginInstance->count);

    data->window = ImGui::FindOrCreateWindow(data->name, ImVec2(400, 400), 0);

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_getWindowRect(ViewPluginInstance* instance, FloatRect* rect)
{
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->privateData;

    ImVec2 pos;
    ImVec2 size;

    ImGui::GetWindowRect(data->window, &pos, &size);

    rect->x = pos.x;
    rect->y = pos.y;
    rect->width = size.x;
    rect->height = size.y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_setWindowRect(ViewPluginInstance* instance, FloatRect* rect)
{
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->privateData;

    ImVec2 pos(rect->x, rect->y);
    ImVec2 size(rect->width, rect->height);

    ImGui::SetWindowRect(data->window, pos, size);
}


