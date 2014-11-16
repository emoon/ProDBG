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

static void separator()
{
    ImGui::Separator();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sameLine(int columnX, int spacingW)
{
    ImGui::SameLine(columnX, spacingW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void spacing()
{
    ImGui::Spacing();
}

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

static float getColumnOffset(int columnIndex)
{
    return ImGui::GetColumnOffset(columnIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setColumnOffset(int columnIndex, float offset)
{
    return ImGui::SetColumnOffset(columnIndex, offset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getColumnWidth(int columnIndex)
{
    return ImGui::GetColumnWidth(columnIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getCursorPos()
{
    ImVec2 t = ImGui::GetCursorPos();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorPos(PDVec2 pos)
{
    ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorPosX(float x)
{
    ImGui::SetCursorPosX(x);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorPosY(float y)
{
    ImGui::SetCursorPosY(y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getCursorScreenPos()
{
    ImVec2 t = ImGui::GetCursorScreenPos();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void alignFirstTextHeightToWidgets()
{
    ImGui::AlignFirstTextHeightToWidgets();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getTextLineSpacing()
{
    return ImGui::GetTextLineSpacing();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getTextLineHeight()
{
    return ImGui::GetTextLineHeight();
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
    return ImGui::Button(label, ImVec2((float)width, (float)height), !!repeatWhenHeld);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void fillRect(PDRect rect, unsigned int color)
{
    ImGui::FillRect(ImVec2(rect.x, rect.y), ImVec2(rect.width, rect.height), color);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getTextWidth(const char* text, const char* textEnd)
{
    return ImGui::GetTextWidth(text, textEnd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getWindowSize()
{
    ImVec2 size = ImGui::GetWindowSize();
    PDVec2 r = { size.x, size.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getFontHeight()
{
    return 12.0f;   // TODO: Fix me
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getFontWidth()
{
    return 12.0f;   // TODO: Fix me
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getMousePos()
{
    ImVec2 pos = ImGui::GetRelativeMousePos();
    PDVec2 r = { pos.x, pos.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getMouseScreenPos()
{
    ImVec2 pos = ImGui::GetMousePos();
    PDVec2 r = { pos.x, pos.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int isMouseClicked(int button, int repeat)
{
    return ImGui::IsMouseClicked(button, !!repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int isMouseDoubleClicked(int button)
{
    return ImGui::IsMouseDoubleClicked(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int isMouseHoveringBox(PDVec2 boxMin, PDVec2 boxMax)
{
    return ImGui::IsMouseHoveringBox(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMax.x, boxMax.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int isKeyDown(int key, int repeat)
{
    return ImGui::IsFocusWindowKeyDown(key, !!repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getKeyModifier()
{
    return 0;
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

    // TODO: These functions are static, we shouldn't need to do it like this

    uiInstance->columns = columns;
    uiInstance->nextColumn = nextColumn;
    uiInstance->sameLine = sameLine;
    uiInstance->text = text;
    uiInstance->button = button;
    uiInstance->buttonSize = buttonSize;

    uiInstance->separator = separator;
    uiInstance->sameLine = sameLine;
    uiInstance->spacing = spacing;
    uiInstance->columns = columns;
    uiInstance->nextColumn = nextColumn;
    uiInstance->getColumnOffset = getColumnOffset;
    uiInstance->setColumnOffset = setColumnOffset;
    uiInstance->getColumnWidth = getColumnWidth;
    uiInstance->getCursorPos = getCursorPos;
    uiInstance->setCursorPos = setCursorPos;
    uiInstance->setCursorPosX = setCursorPosX;
    uiInstance->setCursorPosY = setCursorPosY;
    uiInstance->getCursorScreenPos = getCursorScreenPos;
    uiInstance->alignFirstTextHeightToWidgets = alignFirstTextHeightToWidgets;
    uiInstance->getTextLineSpacing = getTextLineSpacing;
    uiInstance->getTextLineHeight = getTextLineHeight;
    uiInstance->fillRect = fillRect;
    uiInstance->getTextWidth = getTextWidth;
    uiInstance->getWindowSize = getWindowSize;
    uiInstance->getFontHeight = getFontHeight;
    uiInstance->getFontWidth = getFontWidth;
    uiInstance->getMousePos = getMousePos;
    uiInstance->getMouseScreenPos = getMouseScreenPos;
    uiInstance->isMouseClicked = isMouseClicked;
    uiInstance->isMouseDoubleClicked = isMouseDoubleClicked;
    uiInstance->isMouseHoveringBox = isMouseHoveringBox;
    uiInstance->isKeyDown = isKeyDown;
    uiInstance->getKeyModifier = getKeyModifier;

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginUI_isActiveWindow(ViewPluginInstance* instance)
{
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->privateData;

    return ImGui::IsActiveWindow(data->window);
}

