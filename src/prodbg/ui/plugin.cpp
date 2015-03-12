#include "plugin.h"
#include "pd_ui.h"
#include "pd_view.h"
#include "api/plugin_instance.h"
#include "core/alloc.h"
#include "core/log.h"
#include "core/math.h"
#include <imgui.h>
#include <assert.h>

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
    return ImGui::GetTextLineHeightWithSpacing();
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
    
static int checkbox(const char* label, bool* v)
{
	return ImGui::Checkbox(label, v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int buttonSmall(const char* label)
{
    return ImGui::SmallButton(label);
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

static void textColored(PDVec4 col, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    ImGui::TextColoredV(ImVec4(col.x, col.y, col.z, col.w), format, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textWrapped(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    ImGui::TextWrappedV(format, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool scEditText(const char* label, char* buf, int buf_size, float xSize, float ySize, int flags,
                       void (* callback)(void*), void* userData)
{
    return ImGui::ScInputText(label, buf, (size_t)buf_size, xSize, ySize, flags, callback, userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (* InputCallback)(PDInputTextCallbackData*);
struct PDInputTextUserData
{
    InputCallback callback;
    void* userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void inputTextDeleteChars(PDInputTextCallbackData* data, int pos, int byteCount)
{
    char* dst = data->buffer + pos;
    const char* src = data->buffer + pos + byteCount;
    while (char c = *src++)
        *dst++ = c;
    *dst = '\0';

    data->bufferDirty = true;
    if (data->cursorPos + byteCount >= pos)
        data->cursorPos -= byteCount;
    else if (data->cursorPos >= pos)
        data->cursorPos = pos;
    data->selectionStart = data->selectionEnd = data->cursorPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void inputTextInsertChars(PDInputTextCallbackData* data, int pos, const char* text, const char* textEnd = NULL)
{
    const int textLen = int(strlen(data->buffer));
    if (!textEnd)
        textEnd = text + strlen(text);
    const int newTextLen = (int)(textEnd - text);

    if (newTextLen + textLen + 1 >= data->bufferSize)
        return;

    size_t upos = (size_t)pos;
    if ((size_t)textLen != upos)
        memmove(data->buffer + upos + newTextLen, data->buffer + upos, (size_t)textLen - upos);
    memcpy(data->buffer + upos, text, (size_t)newTextLen * sizeof(char));
    data->buffer[textLen + newTextLen] = '\0';

    data->bufferDirty = true;
    if (data->cursorPos >= pos)
        data->cursorPos += (int)newTextLen;
    data->selectionStart = data->selectionEnd = data->cursorPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int textEditCallbackStub(ImGuiTextEditCallbackData* data)
{
    PDInputTextUserData* wrappedUserData = (PDInputTextUserData*)data->UserData;
    PDInputTextCallbackData callbackData = { 0 };

    // Transfer over ImGui callback data into our generic wrapper version
    callbackData.userData       = wrappedUserData->userData;
    callbackData.buffer         = data->Buf;
    callbackData.bufferSize     = int(data->BufSize);
    callbackData.bufferDirty    = data->BufDirty;
    callbackData.flags          = PDInputTextFlags(data->Flags);
    callbackData.cursorPos      = data->CursorPos;
    callbackData.selectionStart = data->SelectionStart;
    callbackData.selectionEnd   = data->SelectionEnd;
    callbackData.deleteChars    = inputTextDeleteChars;
    callbackData.insertChars    = inputTextInsertChars;

    // Translate ImGui event key into our own PDKey mapping
    ImGuiIO& io = ImGui::GetIO();
    callbackData.eventKey = io.KeyMap[data->EventKey];

    // Invoke the callback (synchronous)
    wrappedUserData->callback(&callbackData);

    // We need to mirror any changes to the callback wrapper into the actual ImGui version
    data->UserData       = callbackData.userData;
    data->Buf            = callbackData.buffer;
    data->BufSize        = (size_t)callbackData.bufferSize;
    data->BufDirty       = callbackData.bufferDirty;
    data->Flags          = ImGuiInputTextFlags(callbackData.flags);
    data->CursorPos      = callbackData.cursorPos;
    data->SelectionStart = callbackData.selectionStart;
    data->SelectionEnd   = callbackData.selectionEnd;

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputText(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDInputTextCallbackData*), void* userData)
{
    PDInputTextUserData wrappedUserData;
    wrappedUserData.callback = callback;
    wrappedUserData.userData = userData;
    return ImGui::InputText(label, buf, (size_t)buf_size, ImGuiInputTextFlags(flags), &textEditCallbackStub, &wrappedUserData);
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

static void setScrollHere()
{
    ImGui::SetScrollPosHere();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushItemWidth(float itemWidth)
{
    ImGui::PushItemWidth(itemWidth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popItemWidth()
{
    ImGui::PopItemWidth();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getWindowSize()
{
    ImVec2 size = ImGui::GetWindowSize();
    PDVec2 r = { size.x, size.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getWindowPos()
{
    ImVec2 pos = ImGui::GetWindowPos();
    PDVec2 r = { pos.x, pos.y };
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

static void beginChild(const char* stringId, PDVec2 size, bool border, int extraFlags)
{
    ImGui::BeginChild(stringId, ImVec2(size.x, size.y), border, ImGuiWindowFlags(extraFlags));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endChild()
{
    ImGui::EndChild();
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

static int isItemHovered()
{
    return ImGui::IsItemHovered();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDRect getCurrentClipRect()
{
    ImVec4 t = ImGui::GetWindowDrawList()->clip_rect_stack.back();

    PDRect v = { t.x, t.y, t.z, t.w };

    return v;
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

static void setKeyboardFocusHere(int offset)
{
    ImGui::SetKeyboardFocusHere(offset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushIdPtr(void* id)
{
	ImGui::PushID(id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushIdInt(int id)
{
	ImGui::PushID(id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popId()
{
	ImGui::PopID();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ImGuiStyleVar_ styleLookup[PDStyleVar_Count] =
{
    ImGuiStyleVar_Alpha,            // PDStyleVar_Invalid
    ImGuiStyleVar_Alpha,            // PDStyleVar_Alpha,             // float
    ImGuiStyleVar_WindowPadding,    // PDStyleVar_WindowPadding,     // PDVec2
    ImGuiStyleVar_WindowRounding,   // PDStyleVar_WindowRounding,    // float
    ImGuiStyleVar_FramePadding,     // PDStyleVar_FramePadding,      // PDVec2
    ImGuiStyleVar_FrameRounding,    // PDStyleVar_FrameRounding,     // float
    ImGuiStyleVar_ItemSpacing,      // PDStyleVar_ItemSpacing,       // PDVec2
    ImGuiStyleVar_ItemInnerSpacing, // PDStyleVar_ItemInnerSpacing,  // PDVec2
    ImGuiStyleVar_TreeNodeSpacing,  // PDStyleVar_TreeNodeSpacing,   // float
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushStyleVarV(int styleVar, PDVec2 value)
{
    assert(styleVar >= 0 && styleVar < PDStyleVar_Count);
    ImVec2 vecValue(value.x, value.y);
    ImGui::PushStyleVar(styleLookup[styleVar], vecValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushStyleVarF(int styleVar, float value)
{
    assert(styleVar >= 0 && styleVar < PDStyleVar_Count);
    ImGui::PushStyleVar(styleLookup[styleVar], value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popStyleVar(int count)
{
    ImGui::PopStyleVar(count);
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
    uiInstance->textColored = textColored;
    uiInstance->textWrapped = textWrapped;
    uiInstance->scInputText = scEditText;
    uiInstance->inputText = inputText;
    uiInstance->button = button;
    uiInstance->checkbox = checkbox;
    uiInstance->buttonSmall = buttonSmall;
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
    uiInstance->getWindowPos = getWindowPos;

    uiInstance->getFontHeight = getFontHeight;
    uiInstance->getFontWidth = getFontWidth;
    uiInstance->getMousePos = getMousePos;
    uiInstance->getMouseScreenPos = getMouseScreenPos;
    uiInstance->isMouseClicked = isMouseClicked;
    uiInstance->isMouseDoubleClicked = isMouseDoubleClicked;
    uiInstance->isMouseHoveringBox = isMouseHoveringBox;
    uiInstance->isItemHovered = isItemHovered;
    uiInstance->isKeyDown = isKeyDown;
    uiInstance->getKeyModifier = getKeyModifier;
    uiInstance->setKeyboardFocusHere = setKeyboardFocusHere;
    uiInstance->setScrollHere = setScrollHere;
    uiInstance->pushItemWidth = pushItemWidth;
    uiInstance->popItemWidth = popItemWidth;

    uiInstance->beginChild = beginChild;
    uiInstance->endChild = endChild;
    uiInstance->getCurrentClipRect = getCurrentClipRect;

    uiInstance->pushStyleVarV = pushStyleVarV;
    uiInstance->pushStyleVarF = pushStyleVarF;
    uiInstance->popStyleVar = popStyleVar;

    uiInstance->pushIdPtr = pushIdPtr;
    uiInstance->pushIdInt = pushIdInt;
    uiInstance->popId = popId;

    uiInstance->privateData = alloc_zero(sizeof(PrivateData));
    data->name = buildName(pluginInstance->plugin->name, pluginInstance->count);

    data->window = 0; //ImGui::FindOrCreateWindow(data->name, ImVec2(400, 400), 0);

    uiInstance->privateData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginUIState PluginUI_updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer)
{
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->privateData;

    ImGui::SetNextWindowPos(ImVec2(instance->rect.x, instance->rect.y));
    ImGui::SetNextWindowSize(ImVec2(instance->rect.width - 4, instance->rect.height - 4));

    ImGui::Begin(data->name, &data->showWindow, ImVec2(0, 0), true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    instance->plugin->update(instance->userData, uiInstance, reader, writer);

    ImGui::End();

    if (!data->showWindow)
        return PluginUIState_CloseView;

    return PluginUIState_None;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_getWindowRect(ViewPluginInstance* instance, FloatRect* rect)
{
    rect->x = (float)instance->rect.x;
    rect->y = (float)instance->rect.y;
    rect->width = (float)instance->rect.width;
    rect->height = (float)instance->rect.height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_setWindowRect(ViewPluginInstance* instance, FloatRect* rect)
{
    instance->rect = *rect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   bool PluginUI_isActiveWindow(ViewPluginInstance* instance)
   {
    (void)instance;
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->privateData;

    return ImGui::IsActiveWindow(data->window);
   }
 */

