#include "bgfx_plugin_ui.h"
#include "pd_ui.h"
#include "pd_view.h"
//#include "api/plugin_instance.h"
//#include "core/plugin_handler.h"
//#include "core/alloc.h"
//#include "core/log.h"
//#include "core/math.h"
//#include "core/input_state.h"
//#include "core/plugin_io.h"
//#include "core/service.h"
//#include "ui_dock.h"
#include "ui_host.h"
#include "imgui_setup.h"
#include <imgui.h>
#include <assert.h>

//#include <session/session.h>
//#include <foundation/apple.h>
//#include <foundation/string.h>
#include <bgfx.h>
//#include "core/input_state.h"
//#include "ui/bgfx/cursor.h"
//#include <foundation/string.h>
//#include "i3wm_docking.h"
#include "ui_render.h"
#include <jansson.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <bgfxplatform.h>

struct ImGuiWindow;

// TODO: Move to settings
const int s_borderSize = 4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Context {
    int width;
    int height;
    //InputState inputState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Context s_context;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivateData {
    ImGuiWindow* window;
    const char* name;
    const char* title;
    bool showWindow;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImVec4 pdColorToImVec4(uint32_t color) {
    float r = ((color >> 24) & 0xff) * 1.0f / 255.0f;
    float g = ((color >> 16) & 0xff) * 1.0f / 255.0f;
    float b = ((color >> 8) & 0xff) * 1.0f / 255.0f;
    float a = ((color >> 0) & 0xff) * 1.0f / 255.0f;
    return ImVec4(r, g, b, a);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSCFuncs {
    intptr_t (*send_command)(void* privData, unsigned int message, uintptr_t p0, intptr_t p1);
    void (*update)(void* privData);
    void (*draw)(void* privData);
    void* private_data;
} PDSCFuns;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static intptr_t scSendCommand(void* privData, unsigned int message, uintptr_t p0, intptr_t p1) {
    ImScEditor* editor = (ImScEditor*)privData;
    return editor->SendCommand(message, p0, p1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void scUpdate(void* privData) {
    ImScEditor* editor = (ImScEditor*)privData;
    editor->Draw();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void scDraw(void* privData) {
    ImScEditor* editor = (ImScEditor*)privData;
    return editor->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_title(void* private_data, const char* title) {
    PrivateData* data = (PrivateData*)private_data;

    (void)data;

    if (!strcmp(data->title, title))
        return;

    if (data->title)
        free((void*)data->title);

    data->title = strdup(title);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_window_size() {
    ImVec2 size = ImGui::GetWindowSize();
    PDVec2 r = { size.x, size.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_window_pos() {
    ImVec2 pos = ImGui::GetWindowPos();
    PDVec2 r = { pos.x, pos.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void begin_child(const char* stringId, PDVec2 size, bool border, int extraFlags) {
    ImGui::BeginChild(stringId, ImVec2(size.x, size.y), border, ImGuiWindowFlags(extraFlags));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end_child() {
    ImGui::EndChild();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_scroll_y() {
    return ImGui::GetScrollY();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_scroll_max_y() {
    return ImGui::GetScrollMaxY();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_scroll_y(float scrollY) {
    ImGui::SetScrollY(scrollY);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_scroll_here(float centerYratio) {
    ImGui::SetScrollHere(centerYratio);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_scroll_from_pos_y(float posY, float centerYratio) {
    ImGui::SetScrollFromPosY(posY, centerYratio);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_keyboard_focus_here(int offset) {
    ImGui::SetKeyboardFocusHere(offset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_font(PDUIFont font) {
    ImGui::PushFont((ImFont*)font);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pop_font() {
    ImGui::PopFont();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_style_color(PDUICol idx, PDColor col) {
    ImGui::PushStyleColor(idx, pdColorToImVec4(col));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pop_style_color(int count) {
    ImGui::PopStyleVar(count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_style_var(PDUIStyleVar idx, float val) {
    ImGui::PushStyleVar(ImGuiStyleVar(idx), val);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_style_varVec(PDUIStyleVar idx, PDVec2 val) {
    ImGui::PushStyleVar(ImGuiStyleVar(idx), ImVec2(val.x, val.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pop_style_var(int count) {
    ImGui::PopStyleVar(count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_item_width(float itemWidth) {
    ImGui::PushItemWidth(itemWidth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pop_item_width() {
    ImGui::PopItemWidth();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float calc_item_width() {
    return ImGui::CalcItemWidth();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_allow_keyboard_focus(bool v) {
    ImGui::PushAllowKeyboardFocus(v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pop_allow_keyboard_focus() {
    ImGui::PopAllowKeyboardFocus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_text_wrap_pos(float wrapPosX) {
    ImGui::PushTextWrapPos(wrapPosX);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pop_text_wrap_pos() {
    ImGui::PopTextWrapPos();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_button_repeat(bool repeat) {
    ImGui::PushButtonRepeat(repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pop_button_repeat() {
    ImGui::PopButtonRepeat();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void begin_group() {
    ImGui::BeginGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end_group() {
    ImGui::EndGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void separator() {
    ImGui::Separator();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void same_line(int columnX, int spacingW) {
    ImGui::SameLine((float)columnX, (float)spacingW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void spacing() {
    ImGui::Spacing();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummy(PDVec2 size) {
    ImGui::Dummy(ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void indent() {
    ImGui::Indent();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void un_indent() {
    ImGui::Unindent();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void columns(int count, const char* id, bool border) {
    ImGui::Columns(count, id, border);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void next_column() {
    ImGui::NextColumn();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int get_column_index() {
    return ImGui::GetColumnIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_column_offset(int columnIndex) {
    return ImGui::GetColumnOffset(columnIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_column_offset(int columnIndex, float offset) {
    return ImGui::SetColumnOffset(columnIndex, offset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_column_width(int columnIndex) {
    return ImGui::GetColumnWidth(columnIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int get_columns_count() {
    return ImGui::GetColumnsCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_cursor_pos() {
    ImVec2 t = ImGui::GetCursorPos();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_cursor_pos_x() {
    return ImGui::GetCursorPosX();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_cursor_pos_y() {
    return ImGui::GetCursorPosY();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_cursor_pos(PDVec2 pos) {
    ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_cursor_pos_x(float x) {
    ImGui::SetCursorPosX(x);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_cursor_pos_y(float y) {
    ImGui::SetCursorPosY(y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_cursor_screen_pos() {
    ImVec2 t = ImGui::GetCursorScreenPos();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_cursor_screen_pos(PDVec2 pos) {
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void align_first_text_height_to_widgets() {
    ImGui::AlignFirstTextHeightToWidgets();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_text_line_height() {
    return ImGui::GetTextLineHeight();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_text_line_height_with_spacing() {
    return ImGui::GetTextLineHeightWithSpacing();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_items_line_height_with_spacing() {
    return ImGui::GetItemsLineHeightWithSpacing();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_id_str(const char* strId) {
    ImGui::PushID(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_id_str_range(const char* strBegin, const char* strEnd) {
    ImGui::PushID(strBegin, strEnd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_id_ptr(const void* ptrId) {
    ImGui::PushID(ptrId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_id_int(const int intId) {
    ImGui::PushID(intId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pop_id() {
    ImGui::PopID();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDID get_id_str(const char* strId) {
    return (PDID)ImGui::GetID(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDID get_id_str_range(const char* strBegin, const char* strEnd) {
    return (PDID)ImGui::GetID(strBegin, strEnd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDID get_id_ptr(const void* ptrId) {
    return (PDID)ImGui::GetID(ptrId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text(const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    ImGui::TextV(format, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text_v(const char* fmt, va_list args) {
    ImGui::TextV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text_colored(const PDColor col, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    ImGui::TextColoredV(pdColorToImVec4(col), fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text_colored_v(const PDColor col, const char* fmt, va_list args) {
    ImGui::TextColoredV(pdColorToImVec4(col), fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text_disabled(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    ImGui::TextDisabledV(fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text_disabledV(const char* fmt, va_list args) {
    ImGui::TextDisabledV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text_wrapped(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    ImGui::TextWrappedV(fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text_wrapped_v(const char* fmt, va_list args) {
    ImGui::TextWrappedV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text_unformatted(const char* text, const char* text_end) {
    ImGui::TextUnformatted(text, text_end);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void label_text(const char* label, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    ImGui::LabelTextV(label, fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void label_textV(const char* label, const char* fmt, va_list args) {
    ImGui::LabelTextV(label, fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void bullet() {
    ImGui::Bullet();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void bullet_text(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    ImGui::BulletTextV(fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void bullet_text_v(const char* fmt, va_list args) {
    ImGui::BulletTextV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool button(const char* label, const PDVec2 size) {
    return ImGui::Button(label, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool small_button(const char* label) {
    return ImGui::SmallButton(label);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool invisible_button(const char* strId, const PDVec2 size) {
    return ImGui::InvisibleButton(strId, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void image(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, const PDColor tintColor, const PDColor borderColor) {
    ImGui::Image((ImTextureID)user_texture_id, ImVec2(size.x, size.y), ImVec2(uv0.x, uv0.y), ImVec2(uv1.x, uv1.y), pdColorToImVec4(tintColor), pdColorToImVec4(borderColor));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool image_button(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, int framePadding, const PDColor bgColor, const PDColor tintCol) {
    return ImGui::ImageButton(user_texture_id, ImVec2(size.x, size.y), ImVec2(uv0.x, uv1.y), ImVec2(uv1.x, uv1.y), framePadding, pdColorToImVec4(bgColor), pdColorToImVec4(tintCol));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool collapsing_header(const char* label, const char* strId, bool displayFrame, bool defaultOpen) {
    return ImGui::CollapsingHeader(label, strId, displayFrame, defaultOpen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool checkbox(const char* label, bool* v) {
    return ImGui::Checkbox(label, v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool checkbox_flags(const char* label, unsigned int* flags, unsigned int flagsValue) {
    return ImGui::CheckboxFlags(label, flags, flagsValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool radio_buttonBool(const char* label, bool active) {
    return ImGui::RadioButton(label, active);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool radio_button(const char* label, int* v, int v_button) {
    return ImGui::RadioButton(label, v, v_button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool combo(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems) {
    return ImGui::Combo(label, currentItem, items, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool combo2(const char* label, int* currentItem, const char* itemsSeparatedByZeros, int heightInItems) {
    return ImGui::Combo(label, currentItem, itemsSeparatedByZeros, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool combo3(const char* label, int* currentItem, bool (*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems) {
    return ImGui::Combo(label, currentItem, itemsGetter, data, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool color_button(const PDColor col, bool smallHeight, bool outlineBorder) {
    return ImGui::ColorButton(pdColorToImVec4(col), smallHeight, outlineBorder);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool color_edit3(const char* label, float col[3]) {
    return ImGui::ColorEdit3(label, col);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool color_edit4(const char* label, float col[4], bool showAlpha) {
    return ImGui::ColorEdit4(label, col, showAlpha);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void color_edit_mode(PDUIColorEditMode mode) {
    ImGui::ColorEditMode(mode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plot_lines(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride) {
    ImGui::PlotLines(label, values, valuesCount, valuesOffset, overlayText, scaleMin, scaleMax, ImVec2(graphSize.x, graphSize.y), (int)stride);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plot_lines2(const char* label, float (*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize) {
    ImGui::PlotLines(label, valuesGetter, data, valuesCount, valuesOffset, overlayText, scaleMin, scaleMax, ImVec2(graphSize.x, graphSize.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plot_histogram(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride) {
    ImGui::PlotHistogram(label, values, valuesCount, valuesOffset, overlayText, scaleMin, scaleMax, ImVec2(graphSize.x, graphSize.y), (int)stride);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plot_histogram2(const char* label, float (*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize) {
    ImGui::PlotHistogram(label, valuesGetter, data, valuesCount, valuesOffset, overlayText, scaleMin, scaleMax, ImVec2(graphSize.x, graphSize.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDUISCInterface* sc_input_text(const char* label, float xSize, float ySize, void (*callback)(void*), void* user_data) {
    ImScEditor* ed = ImGui::ScInputText(label, xSize, ySize, callback, user_data);

    if (!ed->userData) {
        PDUISCInterface* funcs = (PDUISCInterface*)malloc(sizeof(PDUISCInterface));
        funcs->send_command = scSendCommand;
        funcs->update = scUpdate;
        funcs->draw = scDraw;
        funcs->private_data = ed;

        ed->userData = (void*)funcs;
    }

    return (PDUISCInterface*)ed->userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_float(const char* label, float* v, float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::SliderFloat(label, v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_float2(const char* label, float v[2], float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::SliderFloat2(label, v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_float3(const char* label, float v[3], float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::SliderFloat3(label, v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_float4(const char* label, float v[4], float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::SliderFloat4(label, v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_angle(const char* label, float* v_rad, float vDegreesMin, float vDegreesMax) {
    return ImGui::SliderAngle(label, v_rad, vDegreesMin, vDegreesMax);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_int(const char* label, int* v, int vMin, int vMax, const char* displayFormat) {
    return ImGui::SliderInt(label, v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_int2(const char* label, int v[2], int vMin, int vMax, const char* displayFormat) {
    return ImGui::SliderInt2(label, v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_int3(const char* label, int v[3], int vMin, int vMax, const char* displayFormat) {
    return ImGui::SliderInt3(label, v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool slider_int4(const char* label, int v[4], int vMin, int vMax, const char* displayFormat) {
    return ImGui::SliderInt4(label, v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool vslider_float(const char* label, const PDVec2 size, float* v, float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::VSliderFloat(label, ImVec2(size.x, size.y), v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool vslider_int(const char* label, const PDVec2 size, int* v, int vMin, int vMax, const char* displayFormat) {
    return ImGui::VSliderInt(label, ImVec2(size.x, size.y), v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool drag_float(const char* label, float* v, float vSpeed, float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::DragFloat(label, v, vSpeed, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool drag_float2(const char* label, float v[2], float vSpeed, float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::DragFloat2(label, v, vSpeed, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool drag_float3(const char* label, float v[3], float vSpeed, float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::DragFloat3(label, v, vSpeed, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool drag_float4(const char* label, float v[4], float vSpeed, float vMin, float vMax, const char* displayFormat, float power) {
    return ImGui::DragFloat4(label, v, vSpeed, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool drag_int(const char* label, int* v, float vSpeed, int vMin, int vMax, const char* displayFormat) {
    return ImGui::DragInt(label, v, vSpeed, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool drag_int2(const char* label, int v[2], float vSpeed, int vMin, int vMax, const char* displayFormat) {
    return ImGui::DragInt2(label, v, vSpeed, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool drag_int3(const char* label, int v[3], float vSpeed, int vMin, int vMax, const char* displayFormat) {
    return ImGui::DragInt3(label, v, vSpeed, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool drag_int4(const char* label, int v[4], float vSpeed, int vMin, int vMax, const char* displayFormat) {
    return ImGui::DragInt4(label, v, vSpeed, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*InputCallback)(PDUIInputTextCallbackData*);

struct PDInputTextUserData {
    InputCallback callback;
    void* user_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void input_textDeleteChars(PDUIInputTextCallbackData* data, int pos, int byteCount) {
    char* dst = data->buf + pos;
    const char* src = data->buf + pos + byteCount;
    while (char c = *src++)
        *dst++ = c;
    *dst = '\0';

    data->buf_dirty = true;
    if (data->cursor_pos + byteCount >= pos)
        data->cursor_pos -= byteCount;
    else if (data->cursor_pos >= pos)
        data->cursor_pos = pos;
    data->selection_start = data->selection_end = data->cursor_pos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void input_textInsertChars(PDUIInputTextCallbackData* data, int pos, const char* text, const char* textEnd = NULL) {
    const int textLen = int(strlen(data->buf));
    if (!textEnd)
        textEnd = text + strlen(text);

    const int newTextLen = (int)(textEnd - text);

    if (newTextLen + textLen + 1 >= data->buf_size)
        return;

    size_t upos = (size_t)pos;
    if ((size_t)textLen != upos)
        memmove(data->buf + upos + newTextLen, data->buf + upos, (size_t)textLen - upos);
    memcpy(data->buf + upos, text, (size_t)newTextLen * sizeof(char));
    data->buf[textLen + newTextLen] = '\0';

    data->buf_dirty = true;
    if (data->cursor_pos >= pos)
        data->cursor_pos += (int)newTextLen;
    data->selection_start = data->selection_end = data->cursor_pos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int textEditCallbackStub(ImGuiTextEditCallbackData* data) {
    PDInputTextUserData* wrappedUserData = (PDInputTextUserData*)data->UserData;
    PDUIInputTextCallbackData callbackData = { 0 };

    // Transfer over ImGui callback data into our generic wrapper version
    callbackData.user_data = wrappedUserData->user_data;
    callbackData.buf = data->Buf;
    callbackData.buf_size = int(data->BufSize);
    callbackData.buf_dirty = data->BufDirty;
    callbackData.flags = PDUIInputTextFlags(data->Flags);
    callbackData.cursor_pos = data->CursorPos;
    callbackData.selection_start = data->SelectionStart;
    callbackData.selection_end  = data->SelectionEnd;
    callbackData.delete_chars = input_textDeleteChars;
    callbackData.insert_chars = input_textInsertChars;

    // Translate ImGui event key into our own PDKey mapping
    ImGuiIO& io = ImGui::GetIO();
    callbackData.event_key = io.KeyMap[data->EventKey];

    // Invoke the callback (synchronous)
    wrappedUserData->callback(&callbackData);

    // We need to mirror any changes to the callback wrapper into the actual ImGui version
    data->UserData = callbackData.user_data;
    data->Buf = callbackData.buf;
    data->BufSize = (int)callbackData.buf_size;
    data->BufDirty = callbackData.buf_dirty;
    data->Flags = ImGuiInputTextFlags(callbackData.flags);
    data->CursorPos = callbackData.cursor_pos;
    data->SelectionStart = callbackData.selection_start;
    data->SelectionEnd   = callbackData.selection_end;

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_text(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDUIInputTextCallbackData*), void* user_data) {
    PDInputTextUserData wrappedUserData;
    wrappedUserData.callback = callback;
    wrappedUserData.user_data = user_data;
    return ImGui::InputText(label, buf, (size_t)buf_size, ImGuiInputTextFlags(flags), &textEditCallbackStub, &wrappedUserData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_text_multiline(const char* label, char* buf, size_t buf_size, const PDVec2 size, PDUIInputTextFlags flags, void (*callback)(PDUIInputTextCallbackData*), void* user_data) {
    PDInputTextUserData wrappedUserData;
    wrappedUserData.callback = callback;
    wrappedUserData.user_data = user_data;
    return ImGui::InputTextMultiline(label, buf, buf_size, ImVec2(size.x, size.y), flags, &textEditCallbackStub, &wrappedUserData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_float(const char* label, float* v, float step, float step_fast, int decimal_precision, PDUIInputTextFlags extraFlags) {
    return ImGui::InputFloat(label, v, step, step_fast, decimal_precision, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_float2(const char* label, float v[2], int decimal_precision, PDUIInputTextFlags extraFlags) {
    return ImGui::InputFloat2(label, v, decimal_precision, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_float3(const char* label, float v[3], int decimal_precision, PDUIInputTextFlags extraFlags) {
    return ImGui::InputFloat3(label, v, decimal_precision, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_float4(const char* label, float v[4], int decimal_precision, PDUIInputTextFlags extraFlags) {
    return ImGui::InputFloat4(label, v, decimal_precision, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_int(const char* label, int* v, int step, int step_fast, PDUIInputTextFlags extraFlags) {
    return ImGui::InputInt(label, v, step, step_fast, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_int2(const char* label, int v[2], PDUIInputTextFlags extraFlags) {
    return ImGui::InputInt2(label, v, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_int3(const char* label, int v[3], PDUIInputTextFlags extraFlags) {
    return ImGui::InputInt3(label, v, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool input_int4(const char* label, int v[4], PDUIInputTextFlags extraFlags) {
    return ImGui::InputInt4(label, v, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool tree_node(const char* str_label_id) {
    return ImGui::TreeNode(str_label_id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool tree_node_str(const char* strId, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    bool ret = ImGui::TreeNodeV(strId, fmt, ap);

    va_end(ap);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool tree_node_ptr(const void* ptrId, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    bool ret = ImGui::TreeNodeV(ptrId, fmt, ap);

    va_end(ap);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool tree_node_str_v(const char* strId, const char* fmt, va_list args) {
    return ImGui::TreeNodeV(strId, fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool tree_node_ptr_v(const void* ptrId, const char* fmt, va_list args) {
    return ImGui::TreeNodeV(ptrId, fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void tree_push_str(const char* strId) {
    ImGui::TreePush(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void tree_push_ptr(const void* ptrId) {
    ImGui::TreePush(ptrId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void tree_pop() {
    ImGui::TreePop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_next_tree_node_opened(bool opened, PDUISetCond cond) {
    ImGui::SetNextTreeNodeOpened(opened, cond);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool selectable(const char* label, bool selected, PDUISelectableFlags flags, const PDVec2 size) {
    return ImGui::Selectable(label, selected, flags, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool selectable_ex(const char* label, bool* p_selected, PDUISelectableFlags flags, const PDVec2 size) {
    return ImGui::Selectable(label, p_selected, flags, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool list_box(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems) {
    return ImGui::ListBox(label, currentItem, items, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool list_box2(const char* label, int* currentItem, bool (*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems) {
    return ImGui::ListBox(label, currentItem, itemsGetter, data, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool list_box_header(const char* label, const PDVec2 size) {
    return ImGui::ListBoxHeader(label, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool list_box_header2(const char* label, int itemsCount, int heightInItems) {
    return ImGui::ListBoxHeader(label, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void list_box_footer() {
    ImGui::ListBoxFooter();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_tooltip(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    ImGui::SetTooltipV(fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_tooltipV(const char* fmt, va_list args) {
    ImGui::SetTooltipV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void begin_tooltip() {
    ImGui::BeginTooltip();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end_tooltip() {
    ImGui::EndTooltip();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_main_menu_bar() {
    return ImGui::BeginMainMenuBar();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end_main_menu_bar() {
    ImGui::EndMainMenuBar();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_menuBar() {
    return ImGui::BeginMenuBar();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end_menu_bar() {
    ImGui::EndMenuBar();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_menu(const char* label, bool enabled) {
    return ImGui::BeginMenu(label, enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end_menu() {
    ImGui::EndMenu();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool menu_item(const char* label, const char* shortcut, bool selected, bool enabled) {
    return ImGui::MenuItem(label, shortcut, selected, enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool menu_item_ptr(const char* label, const char* shortcut, bool* p_selected, bool enabled) {
    return ImGui::MenuItem(label, shortcut, p_selected, enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void open_popup(const char* strId) {
    ImGui::OpenPopup(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_popup(const char* strId) {
    return ImGui::BeginPopup(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_popup_modal(const char* name, bool* p_opened, PDUIWindowFlags extraFlags) {
    return ImGui::BeginPopupModal(name, p_opened, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_popup_context_item(const char* strId, int mouse_button) {
    return ImGui::BeginPopupContextItem(strId, mouse_button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_popup_context_window(bool also_over_items, const char* strId, int mouse_button) {
    return ImGui::BeginPopupContextWindow(also_over_items, strId, mouse_button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_popup_context_void(const char* strId, int mouse_button) {
    return ImGui::BeginPopupContextVoid(strId, mouse_button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end_popup() {
    ImGui::EndPopup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void close_current_popup() {
    ImGui::CloseCurrentPopup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void value_bool(const char* prefix, bool b) {
    ImGui::Value(prefix, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void value_int(const char* prefix, int v) {
    ImGui::Value(prefix, v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void value_u_int(const char* prefix, unsigned int v) {
    ImGui::Value(prefix, v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void value_float(const char* prefix, float v, const char* float_format) {
    ImGui::Value(prefix, v, float_format);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void color(const char* prefix, const PDColor col) {
    ImGui::ValueColor(prefix, pdColorToImVec4(col));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_to_tty(int maxDepth) {
    ImGui::LogToTTY(maxDepth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_to_file(int maxDepth, const char* filename) {
    ImGui::LogToFile(maxDepth, filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_to_clipboard(int maxDepth) {
    ImGui::LogToClipboard(maxDepth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_finish() {
    ImGui::LogFinish();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_buttons() {
    ImGui::LogButtons();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

// We need a LogTextV version here.

static void logText(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    ImGui::SetTooltipV(fmt, ap);
    ImGui::LogTextV(fmt, ...);

    va_end(ap);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_item_hovered() {
    return ImGui::IsItemHovered();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_item_hovered_rect() {
    return ImGui::IsItemHoveredRect();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_item_active() {
    return ImGui::IsItemActive();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_item_visible() {
    return ImGui::IsItemVisible();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_any_item_hovered() {
    return ImGui::IsAnyItemHovered();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_any_item_active() {
    return ImGui::IsAnyItemActive();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_item_rect_min() {
    ImVec2 t = ImGui::GetItemRectMin();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_item_rect_max() {
    ImVec2 t = ImGui::GetItemRectMax();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_item_rect_size() {
    ImVec2 t = ImGui::GetItemRectSize();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_window_hovered() {
    return ImGui::IsWindowHovered();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_window_focused() {
    return ImGui::IsWindowFocused();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_root_window_focused() {
    return ImGui::IsRootWindowFocused();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_root_window_or_any_child_focused() {
    return ImGui::IsRootWindowOrAnyChildFocused();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_rect_visible(const PDVec2 itemSize) {
    return ImGui::IsRectVisible(ImVec2(itemSize.x, itemSize.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_pos_hovering_any_window(const PDVec2 pos) {
    return ImGui::IsPosHoveringAnyWindow(ImVec2(pos.x, pos.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float get_time() {
    return ImGui::GetTime();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int get_frame_count() {
    return ImGui::GetFrameCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* get_style_col_name(PDUICol idx) {
    return ImGui::GetStyleColName(ImGuiCol(idx));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 calc_item_rect_closest_point(const PDVec2 pos, bool onEdge, float outward) {
    ImVec2 t = ImGui::CalcItemRectClosestPoint(ImVec2(pos.x, pos.y), onEdge, outward);
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 calc_text_size(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width) {
    ImVec2 t = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void calc_list_clipping(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end) {
    ImGui::CalcListClipping(items_count, items_height, out_items_display_start, out_items_display_end);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool begin_childFrame(PDID id, const struct PDVec2 size) {
    return ImGui::BeginChildFrame(id, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end_child_frame() {
    ImGui::EndChildFrame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void color_convert_rg_bto_hsv(float r, float g, float b, float* out_h, float* out_s, float* out_v) {
    ImGui::ColorConvertRGBtoHSV(r, g, b, *out_h, *out_s, *out_v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void color_convert_hs_vto_rgb(float h, float s, float v, float* out_r, float* out_g, float* out_b) {
    ImGui::ColorConvertHSVtoRGB(h, s, v, *out_r, *out_g, *out_b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_key_down(int key_index) {
    return ImGui::IsKeyDown(key_index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_key_pressed(int key_index, bool repeat) {
    return ImGui::IsKeyPressed(key_index, repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_key_released(int key_index) {
    return ImGui::IsKeyReleased(key_index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Fix me

static bool is_key_down_id(uint32_t keyId, int repeat) {
    if (!ImGui::IsWindowFocused())
        return false;

    return 0; //!!InputState_isKeyDown(keyId >> 4, keyId & 0xf, repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_mouse_down(int button) {
    return ImGui::IsMouseDown(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_mouse_clicked(int button, bool repeat) {
    return ImGui::IsMouseClicked(button, repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_mouse_double_clicked(int button) {
    return ImGui::IsMouseDoubleClicked(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_mouse_released(int button) {
    return ImGui::IsMouseReleased(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_mouse_hovering_window() {
    return ImGui::IsMouseHoveringWindow();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_mouse_hovering_any_window() {
    return ImGui::IsMouseHoveringAnyWindow();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_mouse_hovering_rect(const struct PDVec2 rectMin, const struct PDVec2 rectMax) {
    return ImGui::IsMouseHoveringRect(ImVec2(rectMin.x, rectMin.y), ImVec2(rectMax.x, rectMax.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_mouse_dragging(int button, float lockThreshold) {
    return ImGui::IsMouseDragging(button, lockThreshold);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_mouse_pos() {
    ImVec2 t = ImGui::GetMousePos();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 get_mouse_drag_delta(int button, float lockThreshold) {
    ImVec2 t = ImGui::GetMouseDragDelta(button, lockThreshold);
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void reset_mouse_drag_delta(int button) {
    ImGui::ResetMouseDragDelta(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDUIMouseCursor get_mouse_cursor() {
    return (PDUIMouseCursor)ImGui::GetMouseCursor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_mouse_cursor(PDUIMouseCursor type) {
    ImGui::SetMouseCursor(type);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void fill_rect(PDRect rect, PDColor color) {
    ImGui::FillRect(ImVec2(rect.x, rect.y), ImVec2(rect.width, rect.height), color);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* buildName(const char* pluginName, int id) {
    char name[1024];

    sprintf(name, "%s %d ###%s%d", pluginName, id, pluginName, id);

    return strdup(name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDUI s_uiFuncs[] =
{
    // Windows

    set_title,
    get_window_size,
    get_window_pos,
    begin_child,
    end_child,

    get_scroll_y,
    get_scroll_max_y,
    set_scroll_y,
    set_scroll_here,
    set_scroll_from_pos_y,
    set_keyboard_focus_here,

    // Parameters stacks (shared)

    push_font,
    pop_font,
    push_style_color,
    pop_style_color,
    push_style_var,
    push_style_varVec,
    pop_style_var,

    // Parameters stacks (current window)

    push_item_width,
    pop_item_width,
    calc_item_width,
    push_allow_keyboard_focus,
    pop_allow_keyboard_focus,
    push_text_wrap_pos,
    pop_text_wrap_pos,
    push_button_repeat,
    pop_button_repeat,

    // Layout

    begin_group,
    end_group,
    separator,
    same_line,
    spacing,
    dummy,
    indent,
    un_indent,
    columns,
    next_column,
    get_column_index,
    get_column_offset,
    set_column_offset,
    get_column_width,
    get_columns_count,
    get_cursor_pos,
    get_cursor_pos_x,
    get_cursor_pos_y,
    set_cursor_pos,
    set_cursor_pos_x,
    set_cursor_pos_y,
    get_cursor_screen_pos,
    set_cursor_screen_pos,
    align_first_text_height_to_widgets,
    get_text_line_height,
    get_text_line_height_with_spacing,
    get_items_line_height_with_spacing,

    // ID scopes
    // If you are creating widgets in a loop you most likely want to push a unique identifier so PDUI can differentiate them
    // You can also use "##extra" within your widget name to distinguish them from each others (see 'Programmer Guide')

    push_id_str,
    push_id_str_range,
    push_id_ptr,
    push_id_int,
    pop_id,
    get_id_str,
    get_id_str_range,
    get_id_ptr,

    // Widgets

    text,
    text_v,
    text_colored,
    text_colored_v,
    text_disabled,
    text_disabledV,
    text_wrapped,
    text_wrapped_v,
    text_unformatted,
    label_text,
    label_textV,
    bullet,
    bullet_text,
    bullet_text_v,
    button,
    small_button,
    invisible_button,
    image,
    image_button,
    collapsing_header,
    checkbox,
    checkbox_flags,
    radio_buttonBool,
    radio_button,
    combo,
    combo2,
    combo3,
    color_button,
    color_edit3,
    color_edit4,
    color_edit_mode,
    plot_lines,
    plot_lines2,
    plot_histogram,
    plot_histogram2,

    // Widgets: Scintilla text interface
    sc_input_text,

    // Widgets: Sliders (tip: ctrl+click on a slider to input text)
    slider_float,
    slider_float2,
    slider_float3,
    slider_float4,
    slider_angle,
    slider_int,
    slider_int2,
    slider_int3,
    slider_int4,
    vslider_float,
    vslider_int,

    // Widgets: Drags (tip: ctrl+click on a drag box to input text)
    drag_float,
    drag_float2,
    drag_float3,
    drag_float4,
    drag_int,
    drag_int2,
    drag_int3,
    drag_int4,

    // Widgets: Input
    input_text,
    input_text_multiline,
    input_float,
    input_float2,
    input_float3,
    input_float4,
    input_int,
    input_int2,
    input_int3,
    input_int4,

    // Widgets: Trees
    tree_node,
    tree_node_str,
    tree_node_ptr,
    tree_node_str_v,
    tree_node_ptr_v,
    tree_push_str,
    tree_push_ptr,
    tree_pop,
    set_next_tree_node_opened,

    // Widgets: Selectable / Lists
    selectable,
    selectable_ex,
    list_box,
    list_box2,
    list_box_header,
    list_box_header2,
    list_box_footer,

    // Tooltip
    set_tooltip,
    set_tooltipV,
    begin_tooltip,
    end_tooltip,

    // Widgets: Menus
    begin_main_menu_bar,
    end_main_menu_bar,
    begin_menuBar,
    end_menu_bar,
    begin_menu,
    end_menu,
    menu_item,
    menu_item_ptr,

    // Popup
    open_popup,
    begin_popup,
    begin_popup_modal,
    begin_popup_context_item,
    begin_popup_context_window,
    begin_popup_context_void,
    end_popup,
    close_current_popup,

    // Widgets: value() Helpers. Output single value in "name: value" format
    value_bool,
    value_int,
    value_u_int,
    value_float,
    color,

    // Logging: all text output from interface is redirected to tty/file/clipboard. Tree nodes are automatically opened.
    log_to_tty,
    log_to_file,
    log_to_clipboard,
    log_finish,
    log_buttons,
    //logText,

    // Utilities
    is_item_hovered,
    is_item_hovered_rect,
    is_item_active,
    is_item_visible,
    is_any_item_hovered,
    is_any_item_active,
    get_item_rect_min,
    get_item_rect_max,
    get_item_rect_size,
    is_window_hovered,
    is_window_focused,
    is_root_window_focused,
    is_root_window_or_any_child_focused,
    is_rect_visible,
    is_pos_hovering_any_window,
    get_time,
    get_frame_count,
    get_style_col_name,
    calc_item_rect_closest_point,
    calc_text_size,
    calc_list_clipping,

    begin_childFrame,
    end_child_frame,

    color_convert_rg_bto_hsv,
    color_convert_hs_vto_rgb,
    is_key_down,
    is_key_pressed,
    is_key_released,

    is_key_down_id,
    is_mouse_down,
    is_mouse_clicked,
    is_mouse_double_clicked,
    is_mouse_released,
    is_mouse_hovering_window,
    is_mouse_hovering_any_window,
    is_mouse_hovering_rect,
    is_mouse_dragging,
    get_mouse_pos,
    get_mouse_drag_delta,
    reset_mouse_drag_delta,
    get_mouse_cursor,
    set_mouse_cursor,

/*

    text,
    text_colored,
    text_wrapped,
    input_text,
    scEditText,
    checkbox,
    button,
    buttonSmall,
    buttonSize,
    selectableFixed,
    selectable,

    // Misc

    // Mouse

    get_mouse_pos,
    getMouseScreenPos,
    is_mouse_clicked,
    is_mouse_double_clicked,
    isMouseHoveringBox,
    is_item_hovered,

    // Keyboard

    is_key_down_id,
    is_key_down,
    getKeyModifier,
    set_keyboard_focus_here,

    // Styles

    push_style_varV,
    push_style_varF,
    pop_style_var,
 */

    // Rendering

    fill_rect,

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void BgfxPluginUI::init(ViewPluginInstance* pluginInstance) {
    PrivateData* data = 0;

    PDUI* uiInstance = &pluginInstance->ui;

    *uiInstance = *s_uiFuncs;

    uiInstance->private_data = alloc_zero(sizeof(PrivateData));

    data = (PrivateData*)uiInstance->private_data;

    data->name = buildName(pluginInstance->plugin->name, pluginInstance->count);
    data->window = 0;
    data->showWindow = true;
    data->title = 0;

    pluginInstance->name = data->name;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback from the docking system

/*
void updateWindowSize(void* user_data, int x, int y, int width, int height) {
    ViewPluginInstance* instance = (ViewPluginInstance*)user_data;

    instance->rect.x = x;
    instance->rect.y = y;
    instance->rect.width = width;
    instance->rect.height = height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

static void setCursorStyle(DockSysCursor cursor) {
    switch (cursor) {
        case DockSysCursor_SizeHorizontal:
            Cunsor_setType(CursorType_SizeHorizontal); break;
        case DockSysCursor_SizeVertical:
            Cunsor_setType(CursorType_SizeVertical); break;
        default:
            Cunsor_setType(CursorType_Default); break;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Move this code?

static void saveUserData(struct json_t* item, void* user_data) {
    ViewPluginInstance* view = (ViewPluginInstance*)user_data;

    if (!view->plugin)
        return;

    PDSaveState saveFuncs;
    PluginIO_initSaveJson(&saveFuncs);

    PluginData* pluginData = PluginHandler_getPluginData(view->plugin);

    assert(pluginData);

    const char* pluginName = view->plugin->name;
    const char* filename = pluginData->filename;

    json_object_set_new(item, "plugin_name", json_string(pluginName));
    json_object_set_new(item, "plugin_file", json_string(filename));

    PDViewPlugin* viewPlugin = (PDViewPlugin*)pluginData->plugin;

    if (!viewPlugin->save_state)
        return;

    json_t* array = json_array();

    saveFuncs.priv_data = array;

    viewPlugin->save_state(view->userData, &saveFuncs);

    json_object_set_new(item, "plugin_data", array);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

static void* loadUserData(struct json_t* item) {
    ViewPluginInstance* view = 0;

    const char* pluginName = json_string_value(json_object_get(item, "plugin_name"));
    const char* filename = json_string_value(json_object_get(item, "plugin_file"));

    // if this is the case we have no plugin created (empty window)

    if (!strcmp(pluginName, "") && !strcmp(filename, "")) {
        view = (ViewPluginInstance*)alloc_zero(sizeof(ViewPluginInstance));
    }else {
        PDLoadState loadFuncs;
        PluginIO_initLoadJson(&loadFuncs);

        PluginData* pluginData = PluginHandler_findPlugin(0, filename, pluginName, true);

        if (!pluginData)
            view = (ViewPluginInstance*)alloc_zero(sizeof(ViewPluginInstance));
        else
            view = g_pluginUI->createViewPlugin(pluginData);

        PDViewPlugin* viewPlugin = (PDViewPlugin*)pluginData->plugin;

        json_t* pluginJsonData = json_object_get(item, "plugin_data");

        if (pluginJsonData && viewPlugin && viewPlugin->load_state) {
            SessionLoadState load_state = { pluginJsonData, (int)json_array_size(pluginJsonData), 0 };
            loadFuncs.priv_data = &load_state;
            viewPlugin->load_state(view->userData, &loadFuncs);
        }
    }

    // TODO: Fi this: assuming one session

    Session** sessions = Session_getSessions();

    assert(sessions);
    assert(sessions[0]);

    Session_addViewPlugin(sessions[0], view);

    return view;
}

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static DockSysCallbacks s_dockSysCallbacks =
{
    updateWindowSize,
    setCursorStyle,
    saveUserData,
    loadUserData,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginUI::State BgfxPluginUI::updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer) {
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->private_data;

    float x = (float)instance->rect.x;
    float y = (float)instance->rect.y;
    float w = (float)instance->rect.width;
    float h = (float)instance->rect.height;

    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(w - s_borderSize, h - s_borderSize));

    // TODO: Cache this?

    char title[1024];

    if (!data->title)
        strcpy(title, data->name);
    else{
        sprintf(title, "%s %d - %s###%s%d",
                instance->plugin->name, instance->count,
                data->title, instance->plugin->name, instance->count);
    }

    ImGui::Begin(title, &data->showWindow, ImVec2(0, 0), true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    instance->plugin->update(instance->userData, uiInstance, reader, writer);

    ImGui::End();

    // Draw border

    if  (!data->showWindow)
        return CloseView;

    return None;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int BgfxPluginUI::getStatusBarSize() {
    return m_statusSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void renderStatusBar(const char* text, float statusSize) {
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

void BgfxPluginUI::setStatusTextNoFormat(const char* text) {
    strcpy(m_statusText, text);
}

/*

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDock(UIDockingGrid* grid) {
    switch (UIDock_getSizingState(grid)) {
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

static void updateDocking(Session* session) {
    InputState* state = InputState_getState();

    int mx = (int)state->mousePos.x;
    int my = (int)state->mousePos.y;

    struct ViewPluginInstance* view = Session_getViewAt(session, mx, my, 0);

    docksys_set_mouse(view, mx, my, state->mouseDown[0]);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::preUpdate() {
    const float deltaTime = 1.0f / 60.f; // TODO: Calc correct dt

    bgfx::setViewRect(0, 0, 0, (uint16_t)s_context.width, (uint16_t)s_context.height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000f0f0f, 1.0f, 0);
    bgfx::submit(0);

    IMGUI_preUpdate(deltaTime);
    //InputState_update(deltaTime);

	/*
    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i) {
        Session* session = sessions[i];
        updateDocking(session);
    }

    docksys_update();
    */
}

/*

static PosColorVertex* fill_rectBorder(PosColorVertex* verts, IntRect* rect, uint32_t color) {
    const float x0 = (float)rect->x;
    const float y0 = (float)rect->y;
    const float x1 = (float)rect->width + x0;
    const float y1 = (float)rect->height + y0;

    // First triangle

    verts[0].x = x0;
    verts[0].y = y0;
    verts[0].color = color;

    verts[1].x = x1;
    verts[1].y = y0;
    verts[1].color = color;

    verts[2].x = x1;
    verts[2].y = y1;
    verts[2].color = color;

    // Second triangle

    verts[3].x = x0;
    verts[3].y = y0;
    verts[3].color = color;

    verts[4].x = x1;
    verts[4].y = y1;
    verts[4].color = color;

    verts[5].x = x0;
    verts[5].y = y1;
    verts[5].color = color;

    verts += 6;

    return verts;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void renderBorders(Session* session) {
    int count = 0;
    ViewPluginInstance** views = Session_getViewPlugins(session, &count);

    bgfx::TransientVertexBuffer tvb;

    const uint32_t vertexCount = (uint32_t)count * 2 * 6;

    UIRender_allocPosColorTb(&tvb, vertexCount);
    PosColorVertex* verts = (PosColorVertex*)tvb.data;

    // TODO: Use settings for colors

    const uint32_t colorDefalut = (0x40 << 16) | (0x40 << 8) | 0x40;
    const uint32_t colorHigh = (0x60 << 16) | (0x60 << 8) | 0x60;

    for (int i = 0; i < count; ++i) {
        IntRect t = views[i]->rect;

        IntRect t0 = {{{ t.x + t.width - s_borderSize, t.y, s_borderSize, t.height }}};
        IntRect t1 = {{{ t.x, t.y + t.height - s_borderSize, t.width, s_borderSize }}};

        verts = fill_rectBorder(verts, &t0, colorDefalut);
        verts = fill_rectBorder(verts, &t1, colorDefalut);
    }

    bgfx::setState(0
                   | BGFX_STATE_RGB_WRITE
                   | BGFX_STATE_ALPHA_WRITE
                   | BGFX_STATE_MSAA);

    UIRender_posColor(&tvb, 0, vertexCount);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::postUpdate() {
    renderStatusBar(m_statusText, (float)m_statusSize);
    IMGUI_postUpdate();

	/*
    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i) {
        Session* session = sessions[i];
        renderBorders(session);
    }
    */

    bgfx::frame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::create(void* windowHandle, int width, int height) {
    //docksys_set_callbacks(&s_dockSysCallbacks);

#ifdef PRODBG_WIN
    bgfx::winSetHwnd((HWND)windowHandle);
#elif PRODBG_MAC
    bgfx::osxSetNSWindow(windowHandle);
#endif
    bgfx::init();
    bgfx::reset((uint32_t)width, (uint32_t)height);
    bgfx::setViewSeq(0, true);
    IMGUI_setup(width, height);

    s_context.width = width;
    s_context.height = height;

    //Service_register(&g_serviceMessageFuncs, PDMESSAGEFUNCS_GLOBAL);
    //Service_register(&g_dialogFuncs, PDDIALOGS_GLOBAL);

    //Cursor_init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::destroy() {
}

// It's a bit weird to have the code like this here. To be cleaned up

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_set_mouse_pos(float x, float y) {
    IMGUI_setMousePos(x, y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_set_mouse_state(int button, int state) {
    IMGUI_setMouseState(state);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_set_scroll(float x, float y) {
    (void)x;
    IMGUI_setScroll(y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_key_down(int key, int modifier) {
    //InputState* state = InputState_getState();

    //state->keysDown[key] = true;
    //state->modifiers = modifier;

    IMGUI_setKeyDown(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyDownMods(int modifier) {
    //InputState* state = InputState_getState();
    //state->modifiers = modifier;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_key_up(int key, int modifier) {
	/*
    InputState* state = InputState_getState();

    state->keysDown[key] = false;
    state->modifiers = modifier;
    */

    IMGUI_setKeyUp(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_add_char(unsigned short c) {
    IMGUI_addInputCharacter(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void prodbg_set_window_size(int width, int height) {
    Context* context = &s_context;

    context->width = width;
    context->height = height;

    bgfx::reset((uint32_t)width, (uint32_t)height);
    IMGUI_updateSize(width, height);

	/*
    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i) {
        Session* session = sessions[i];
        docksys_update_size(width, height - (int)g_pluginUI->getStatusBarSize());
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_create() {
    g_pluginUI = new BgfxPluginUI;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_create_window(void* window, int width, int height) {
    g_pluginUI->create(window, width, height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_pre_update() {
    g_pluginUI->preUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_post_update() {
    g_pluginUI->postUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_destroy() {
    g_pluginUI->destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void* bgfx_get_ui_funcs() {
	return s_uiFuncs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_imgui_set_window_pos(float x, float y) {
    ImGui::SetNextWindowPos(ImVec2(x, y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_imgui_set_window_size(float w, float h) {
    ImGui::SetNextWindowSize(ImVec2(w - s_borderSize, h - s_borderSize));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_imgui_begin(int show) {
	bool s = !!show;
    //ImGui::Begin("Test", &s, ImVec2(0, 0), true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Begin("Test", &s, ImVec2(500, 500), true, ImGuiWindowFlags_NoCollapse);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void bgfx_imgui_end() {
    ImGui::End();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* s_temp;

extern "C" void bgfx_set_context(void* context) {
	s_temp = context;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void* bgfx_get_context() {
	return s_temp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" float bgfx_get_screen_width() {
	return (float)s_context.width;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" float bgfx_get_screen_height() {
	return (float)s_context.height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void* bgfx_get_native_context() {
	return bgfx::nativeContext();
}






