#include "bgfx_plugin_ui.h"
#include "pd_ui.h"
#include "pd_view.h"
#include "api/plugin_instance.h"
#include "core/alloc.h"
#include "core/log.h"
#include "core/math.h"
#include "ui_dock.h"
#include "imgui_setup.h"
#include <imgui.h>
#include <assert.h>

#include <session/session.h>
#include <foundation/apple.h>
#include <foundation/string.h>
#include <bgfx.h>
#include "core/input_state.h"
#include "ui/bgfx/cursor.h"

struct ImGuiWindow;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Context
{
    int width;
    int height;
    //InputState inputState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Context s_context;

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

typedef struct PDSCFuncs
{
    intptr_t (* sendCommand)(void* privData, unsigned int message, uintptr_t p0, intptr_t p1);
    void (* update)(void* privData);
    void (* draw)(void* privData);
    void* privateData;
} PDSCFuns;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static intptr_t scSendCommand(void* privData, unsigned int message, uintptr_t p0, intptr_t p1)
{
    ImScEditor* editor = (ImScEditor*)privData;
    return editor->SendCommand(message, p0, p1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void scUpdate(void* privData)
{
    ImScEditor* editor = (ImScEditor*)privData;
    editor->Draw();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void scDraw(void* privData)
{
    ImScEditor* editor = (ImScEditor*)privData;
    return editor->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDSCInterface* scEditText(const char* label, float xSize, float ySize,
                                 void (* callback)(void*), void* userData)
{
    ImScEditor* ed = ImGui::ScInputText(label, xSize, ySize, callback, userData);

    if (!ed->userData)
    {
        PDSCInterface* funcs = (PDSCInterface*)malloc(sizeof(PDSCInterface));
        funcs->sendCommand = scSendCommand;
        funcs->update = scUpdate;
        funcs->draw = scDraw;
        funcs->privateData = ed;

        ed->userData = (void*)funcs;
    }

    return (PDSCInterface*)ed->userData;
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

static bool inputText(const char* label, char* buf, int buf_size, int flags, void (* callback)(PDInputTextCallbackData*), void* userData)
{
    PDInputTextUserData wrappedUserData;
    wrappedUserData.callback = callback;
    wrappedUserData.userData = userData;
    return ImGui::InputText(label, buf, (size_t)buf_size, ImGuiInputTextFlags(flags), &textEditCallbackStub, &wrappedUserData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int buttonSize(const char* label, int width, int height)
{
    // TODO(marco): interface has changed, do we still need repeatWhenHeld?
    return ImGui::Button(label, ImVec2((float)width, (float)height));
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
    ImGuiStyleVar_IndentSpacing,    // PDStyleVar_TreeNodeSpacing,   // float
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

void BgfxPluginUI::init(ViewPluginInstance* pluginInstance)
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
    pluginInstance->name = data->name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginUI::State BgfxPluginUI::updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer)
{
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->privateData;

    ImGui::SetNextWindowPos(ImVec2((float)instance->rect.x, (float)instance->rect.y));
    ImGui::SetNextWindowSize(ImVec2((float)instance->rect.width - 4, (float)instance->rect.height - 4));

    ImGui::Begin(data->name, &data->showWindow, ImVec2(0, 0), true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    instance->plugin->update(instance->userData, uiInstance, reader, writer);

    ImGui::End();

    if (!data->showWindow)
        return CloseView;

    return None;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float BgfxPluginUI::getStatusBarSize()
{
    return m_statusSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void renderStatusBar(const char* text, float statusSize)
{
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

void BgfxPluginUI::setStatusTextNoFormat(const char* text)
{
    string_copy(m_statusText, text, sizeof(m_statusText));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDock(UIDockingGrid* grid)
{
    switch (UIDock_getSizingState(grid))
    {
        case UIDockSizerDir_None:
        {
            Cunsor_setType(CursorType_Default);
            break;
        }

        case UIDockSizerDir_Horz:
        {
            Cunsor_setType(CursorType_SizeHorizontal);
            break;
        }

        case UIDockSizerDir_Vert:
        {
            Cunsor_setType(CursorType_SizeVertical);
            break;
        }

        case UIDockSizerDir_Both:
        {
            Cunsor_setType(CursorType_SizeAll);
            break;
        }
    }

    UIDock_update(grid, InputState_getState());
    UIDock_render(grid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::preUpdate()
{
    bgfx::setViewRect(0, 0, 0, (uint16_t)s_context.width, (uint16_t)s_context.height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x101010ff, 1.0f, 0);
    bgfx::submit(0);
    IMGUI_preUpdate(1.0f / 60.0f);

    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i)
    {
        Session* session = sessions[i];
        UIDockingGrid* grid = Session_getDockingGrid(session);
        updateDock(grid);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::postUpdate()
{
    renderStatusBar(m_statusText, m_statusSize);
    IMGUI_postUpdate();

    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i)
    {
        Session* session = sessions[i];
        UIDockingGrid* grid = Session_getDockingGrid(session);
        UIDock_render(grid);
    }

    bgfx::frame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::create(int width, int height)
{
    bgfx::init();
    bgfx::reset((uint32_t)width, (uint32_t)height);
    bgfx::setViewSeq(0, true);
    IMGUI_setup(width, height);

    s_context.width = width;
    s_context.height = height;

    Cursor_init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::destroy()
{
}

// It's a bit weird to have the code like this here. To be cleaned up

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setMousePos(float x, float y)
{
    InputState* state = InputState_getState();

    state->mousePos.x = x;
    state->mousePos.y = y;

    IMGUI_setMousePos(x, y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setMouseState(int button, int state)
{
    InputState* inputState = InputState_getState();
    inputState->mouseDown[0] = !!state;

    IMGUI_setMouseState(state);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyDown(int key, int modifier)
{
    IMGUI_setKeyDown(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyUp(int key, int modifier)
{
    InputState* state = InputState_getState();

    IMGUI_setKeyUp(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_addChar(unsigned short c)
{
    IMGUI_addInputCharacter(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setWindowSize(int width, int height)
{
    Context* context = &s_context;

    context->width = width;
    context->height = height;

    bgfx::reset((uint32_t)width, (uint32_t)height);
    IMGUI_updateSize(width, height);

    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i)
    {
        Session* session = sessions[i];
        UIDockingGrid* grid = Session_getDockingGrid(session);

        updateDock(grid);
        UIDock_updateSize(grid, width, height - (int)g_pluginUI->getStatusBarSize());
    }
}





