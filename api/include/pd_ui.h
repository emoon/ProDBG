#ifndef _PDUI_H_
#define _PDUI_H_

#include "pd_common.h"
#include "pd_keys.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PDUIPainter;
struct PDRect;
typedef struct PDTextureID PDTextureID;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDVec2
{
    float x, y;
} PDVec2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDVec4
{
    float x, y, z, w;
} PDVec4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDRect
{
	float x, y;
	float width, height;
} PDRect;

typedef unsigned int PDColor;

// TODO: use uint32_t?

#define PD_COLOR_32(r, g, b, a) (((unsigned int)r << 24) | ((unsigned int)g << 16) | ((unsigned int)b << 8) | (a))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDStyleVar
{
    PDStyleVar_Invalid = 0,
    PDStyleVar_Alpha,             // float
    PDStyleVar_WindowPadding,     // PDVec2
    PDStyleVar_WindowRounding,    // float
    PDStyleVar_FramePadding,      // PDVec2
    PDStyleVar_FrameRounding,     // float
    PDStyleVar_ItemSpacing,       // PDVec2
    PDStyleVar_ItemInnerSpacing,  // PDVec2
    PDStyleVar_TreeNodeSpacing,   // float
    PDStyleVar_Count
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Abstract this more

enum PDWindowFlags
{
    // Default: 0
    PDWindowFlags_NoTitleBar          = 1 << 0,   // Disable title-bar
    PDWindowFlags_NoResize            = 1 << 1,   // Disable user resizing with the lower-right grip
    PDWindowFlags_NoMove              = 1 << 2,   // Disable user moving the window
    PDWindowFlags_NoScrollbar         = 1 << 3,   // Disable scroll bar (window can still scroll with mouse or programatically)
    PDWindowFlags_NoScrollWithMouse   = 1 << 4,   // Disable user scrolling with mouse wheel
    PDWindowFlags_NoCollapse          = 1 << 5,   // Disable user collapsing window by double-clicking on it
    PDWindowFlags_AlwaysAutoResize    = 1 << 6,   // Resize every window to its content every frame
    PDWindowFlags_ShowBorders         = 1 << 7,   // Show borders around windows and items
    PDWindowFlags_NoSavedSettings     = 1 << 8,   // Never load/save settings in .ini file

    // [Internal]
    PDWindowFlags_ChildWindow         = 1 << 9,   // For internal use by BeginChild()
    PDWindowFlags_ChildWindowAutoFitX = 1 << 10,  // For internal use by BeginChild()
    PDWindowFlags_ChildWindowAutoFitY = 1 << 11,  // For internal use by BeginChild()
    PDWindowFlags_ComboBox            = 1 << 12,  // For internal use by ComboBox()
    PDWindowFlags_Tooltip             = 1 << 13   // For internal use by BeginTooltip()
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Abstract this more

enum PDInputTextFlags
{
    // Default: 0
    PDInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    PDInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    PDInputTextFlags_CharsUppercase      = 1 << 2,   // Turn a..z into A..Z
    PDInputTextFlags_CharsNoBlank        = 1 << 3,   // Filter out spaces, tabs
    PDInputTextFlags_AutoSelectAll       = 1 << 4,   // Select entire text when first taking mouse focus
    PDInputTextFlags_EnterReturnsTrue    = 1 << 5,   // Return 'true' when Enter is pressed (as opposed to when the value was modified)
    PDInputTextFlags_CallbackCompletion  = 1 << 6,   // Call user function on pressing TAB (for completion handling)
    PDInputTextFlags_CallbackHistory     = 1 << 7,   // Call user function on pressing Up/Down arrows (for history handling)
    PDInputTextFlags_CallbackAlways      = 1 << 8,   // Call user function every time
    PDInputTextFlags_CallbackCharFilter  = 1 << 9    // Call user function to filter character. Modify data->EventChar to replace/filter input, or return 1 to discard character.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDInputTextCallbackData
{
    int eventKey;            // Key pressed (Up/Down/TAB)           // Read-only   
    char* buffer;            // Current text                        // Read-write (pointed data only)
    int bufferSize;          // Read-only
    bool bufferDirty;        // Set if you modify buffer directly   // Write
    int flags;               // What user passed to inputText()     // Read-only   // PDInputTextFlags
    int cursorPos;           //                                     // Read-write
    int selectionStart;      //                                     // Read-write (== to selectionEnd when no selection)
    int selectionEnd;        //                                     // Read-write
    void* userData;          // What user passed to InputText()

	void (*deleteChars)(struct PDInputTextCallbackData* data, int pos, int byteCount);
	void (*insertChars)(struct PDInputTextCallbackData* data, int pos, const char* text, const char* textEnd);
} PDInputTextCallbackData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSCInterface
{
	intptr_t (*sendCommand)(void* privData, unsigned int message, uintptr_t p0, intptr_t p1);
	void (*update)(void* privData);
	void (*draw)(void* privData);
	void* privateData;
} PDSCInterface;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUI
{
	// Window
	
	void (*setTitle)(void* privateData, const char* title);
	PDVec2 (*getWindowSize)();
	PDVec2 (*getWindowPos)();

    void (*beginChild)(const char* stringId, PDVec2 size, bool border, int extraFlags /* PDWindowFlags */);
    void (*endChild)();

	float (*getScrollPosY)();
	float (*getScrollMaxY)();
	void (*setScrollPosHere)();
	void (*setKeyboardFocusHere)(int offset);

	// Parameters stacks (shared)
	void (*pushFont)(ImFont* font);
	void (*popFont)();
	void (*pushStyleColor)(ImGuiCol idx, CONST struct ImVec4 col);
	void (*popStyleColor)(int count);
	void (*pushStyleVar)(ImGuiStyleVar idx, float val);
	void (*pushStyleVarVec)(ImGuiStyleVar idx, CONST struct ImVec2 val);
	void (*popStyleVar)(int count);

	// Parameters stacks (current window)
	void (*pushItemWidth)(float item_width);
	void (*popItemWidth)();
	float (*calcItemWidth)();
	void (*pushAllowKeyboardFocus)(bool v);
	void (*popAllowKeyboardFocus)();
	void (*pushTextWrapPos)(float wrap_pos_x);
	void (*popTextWrapPos)();
	void (*pushButtonRepeat)(bool repeat);
	void (*popButtonRepeat)();


	// Tooltip
	void (*setTooltip)(const char* fmt, ...);
	void (*setTooltipV)(const char* fmt, va_list args);
	void (*beginTooltip)();
	void (*endTooltip)();

	// Popup
	void (*openPopup)(const char* strId);
	bool (*beginPopup)(const char* strId);
	bool (*beginPopupModal)(const char* name, bool* p_opened, ImGuiWindowFlags extraFlags);
	bool (*beginPopupContextItem)(const char* strId, int mouse_button);
	bool (*beginPopupContextWindow)(bool also_over_items, const char* strId, int mouse_button);
	bool (*beginPopupContextVoid)(const char* strId, int mouse_button);
	void (*endPopup)();
	void (*closeCurrentPopup)();

	// Layout
	void (*beginGroup)();
	void (*endGroup)();
	void (*separator)();
	void (*sameLine)(int column_x, int spacing_w);
	void (*spacing)();
	void (*dummy)(const ImVec2* size);
	void (*indent)();
	void (*unindent)();
	void (*columns)(int count, const char* id, bool border);
	void (*nextColumn)();
	int  (*getColumnIndex)();
	float(*getColumnOffset)(int column_index);
	void (*setColumnOffset)(int column_index, float offset_x);
	float (*getColumnWidth)(int column_index);
	int (*getColumnsCount)();
	void (*getCursorPos)(PDVec2* pOut);
	float (*getCursorPosX)();
	float (*getCursorPosY)();
	void (*setCursorPos)(const PDVec2 pos);
	void (*setCursorPosX)(float x);
	void (*setCursorPosY)(float y);
	void (*getCursorScreenPos)(PDVec2* pOut);
	void (*setCursorScreenPos)(const PDVec2 pos);
	void (*alignFirstTextHeightToWidgets)();
	float(*getTextLineHeight)();
	float (*getTextLineHeightWithSpacing)();
	float (*getItemsLineHeightWithSpacing)();

	// ID scopes
	// If you are creating widgets in a loop you most likely want to push a unique identifier so ImGui can differentiate them
	// You can also use "##extra" within your widget name to distinguish them from each others (see 'Programmer Guide')
	void (*pushIdStr)(const char* strId);
	void (*pushIdStrRange)(const char* str_begin, const char* str_end);
	void (*pushIdPtr)(const void* ptr_id);
	void (*pushIdInt)(const int int_id);
	void (*popId)();
	ImGuiID (*getIdStr)(const char* strId);
	ImGuiID (*getIdStrRange)(const char* str_begin,const char* str_end);
	ImGuiID (*getIdPtr)(const void* ptr_id);

	// Widgets
	void (*text)(const char* fmt, ...);
	void (*textV)(const char* fmt, va_list args);
	void (*textColored)(const PDColor col, const char* fmt, ...);
	void (*textColoredV)(const PDColor col, const char* fmt, va_list args);
	void (*textDisabled)(const char* fmt, ...);
	void (*textDisabledV)(const char* fmt, va_list args);
	void (*textWrapped)(const char* fmt, ...);
	void (*textWrappedV)(const char* fmt, va_list args);
	void (*textUnformatted)(const char* text, const char* text_end);
	void (*labelText)(const char* label, const char* fmt, ...);
	void (*labelTextV)(const char* label, const char* fmt, va_list args);
	void (*bullet)();
	void (*bulletText)(const char* fmt, ...);
	void (*bulletTextV)(const char* fmt, va_list args);
	bool (*button)(const char* label, const PDVec2 size);
	bool (*smallButton)(const char* label);
	bool (*invisibleButton)(const char* strId, const PDVec2 size);
	void (*image)(PDTextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, const PDColor tintColor, const PDColor borderColor);
	bool (*imageButton)(PDTextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, int framePadding, const PDColor bgColor, const PDColor tintCol);
	bool (*collapsingHeader)(const char* label, const char* strId, bool displayFrame, bool defaultOpen);
	bool (*checkbox)(const char* label, bool* v);
	bool (*checkboxFlags)(const char* label, unsigned int* flags, unsigned int flagsValue);
	bool (*radioButtonBool)(const char* label, bool active);
	bool (*radioButton)(const char* label, int* v, int v_button);
	bool (*combo)(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems);
	bool (*combo2)(const char* label, int* currentItem, const char* itemsSeparatedByZeros, int heightInItems);
	bool (*combo3)(const char* label, int* currentItem, bool(*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems);
	bool (*colorButton)(const PDVec4 col, bool smallHeight, bool outlineBorder);
	bool (*colorEdit3)(const char* label, float col[3]);
	bool (*colorEdit4)(const char* label, float col[4], bool showAlpha);
	void (*colorEditMode)(ImGuiColorEditMode mode);
	void (*plotLines)(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride);
	void (*plotLines2)(const char* label, float(*values_getter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize);
	void (*plotHistogram)(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride);
	void (*plotHistogram2)(const char* label, float(*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize);

	// Widgets: Scintilla text interface
	PDSCInterface* (*scInputText)(const char* label, float xSize, float ySize, void (*callback)(void), void* userData);

	// Widgets: Sliders (tip: ctrl+click on a slider to input text)
	bool (*sliderFloat)(const char* label, float* v, float vMin, float vMax, const char* displayFormat, float power);
	bool (*sliderFloat2)(const char* label, float v[2], float vMin, float vMax, const char* displayFormat, float power);
	bool (*sliderFloat3)(const char* label, float v[3], float vMin, float vMax, const char* displayFormat, float power);
	bool (*sliderFloat4)(const char* label, float v[4], float vMin, float vMax, const char* displayFormat, float power);
	bool (*sliderAngle)(const char* label, float* v_rad, float vDegreesMin, float vDegreesMax);
	bool (*sliderInt)(const char* label, int* v, int vMin, int vMax, const char* displayFormat);
	bool (*sliderInt2)(const char* label, int v[2], int vMin, int vMax, const char* displayFormat);
	bool (*sliderInt3)(const char* label, int v[3], int vMin, int vMax, const char* displayFormat);
	bool (*sliderInt4)(const char* label, int v[4], int vMin, int vMax, const char* displayFormat);
	bool (*vsliderFloat)(const char* label, const PDVec2 size, float* v, float vMin, float vMax, const char* displayFormat, float power);
	bool (*vsliderInt)(const char* label, const PDVec2 size, int* v, int vMin, int vMax, const char* displayFormat);

	// Widgets: Drags (tip: ctrl+click on a drag box to input text)
	bool (*dragFloat)(const char* label, float* v, float vSpeed, float vMin, float vMax, const char* displayFormat, float power);     // If vMax >= vMax we have no bound
	bool (*dragFloat2)(const char* label, float v[2], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
	bool (*dragFloat3)(const char* label, float v[3], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
	bool (*dragFloat4)(const char* label, float v[4], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
	bool (*dragInt)(const char* label, int* v, float vSpeed, int vMin, int vMax, const char* displayFormat);                                       // If vMax >= vMax we have no bound
	bool (*dragInt2)(const char* label, int v[2], float vSpeed, int vMin, int vMax, const char* displayFormat);
	bool (*dragInt3)(const char* label, int v[3], float vSpeed, int vMin, int vMax, const char* displayFormat);
	bool (*dragInt4)(const char* label, int v[4], float vSpeed, int vMin, int vMax, const char* displayFormat);

	// Widgets: Input
    bool (*inputText)(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDInputTextCallbackData), void* userData);
	bool (*inputTextMultiline)(const char* label, char* buf, size_t buf_size, const ImVec2 size, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* userData);
	bool (*inputFloat)(const char* label, float* v, float step, float step_fast, int decimal_precision, ImGuiInputTextFlags extraFlags);
	bool (*inputFloat2)(const char* label, float v[2], int decimal_precision, ImGuiInputTextFlags extraFlags);
	bool (*inputFloat3)(const char* label, float v[3], int decimal_precision, ImGuiInputTextFlags extraFlags);
	bool (*inputFloat4)(const char* label, float v[4], int decimal_precision, ImGuiInputTextFlags extraFlags);
	bool (*inputInt)(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags extraFlags);
	bool (*inputInt2)(const char* label, int v[2], ImGuiInputTextFlags extraFlags);
	bool (*inputInt3)(const char* label, int v[3], ImGuiInputTextFlags extraFlags);
	bool (*inputInt4)(const char* label, int v[4], ImGuiInputTextFlags extraFlags);

	// Widgets: Trees
	bool (*treeNode)(const char* str_label_id);
	bool (*treeNodeStr)(const char* strId, const char* fmt, ...);
	bool (*treeNodePtr)(const void* ptr_id, const char* fmt, ...);
	bool (*treeNodeStrV)(const char* strId, const char* fmt, va_list args);
	bool (*treeNodePtrV)(const void* ptr_id, const char* fmt, va_list args);
	void (*treePushStr)(const char* strId);
	void (*treePushPtr)(const void* ptr_id);
	void (*treePop)();
	void (*setNextTreeNodeOpened)(bool opened, ImGuiSetCond cond);

	// Widgets: Selectable / Lists
	bool (*selectable)(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2 size);
	bool (*selectableEx)(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2 size);
	bool (*listBox)(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems);
	bool (*listBox2)(const char* label, int* currentItem, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems);
	bool (*listBoxHeader)(const char* label, const PDVec2 size);
	bool (*listBoxHeader2)(const char* label, int itemsCount, int heightInItems);
	void (*listBoxFooter)();

	// Widgets: Menus
	bool (*beginMainMenuBar)();
	void (*endMainMenuBar)();
	bool (*beginMenuBar)();
	void (*endMenuBar)();
	bool (*beginMenu)(const char* label, bool enabled);
	void (*endMenu)();
	bool (*menuItem)(const char* label, const char* shortcut, bool selected, bool enabled);
	bool (*menuItemPtr)(const char* label, const char* shortcut, bool* p_selected, bool enabled);

	// Widgets: Value() Helpers. Output single value in "name: value" format (tip: freely declare your own within the ImGui namespace!)
	void (*valueBool)(const char* prefix, bool b);
	void (*valueInt)(const char* prefix, int v);
	void (*valueUInt)(const char* prefix, unsigned int v);
	void (*valueFloat)(const char* prefix, float v, const char* float_format);
	void (*color)(const char* prefix, const PDVec4 v);
	void (*color2)(const char* prefix, unsigned int v);

	// Logging: all text output from interface is redirected to tty/file/clipboard. Tree nodes are automatically opened.
	void (*logToTTY)(int max_depth);
	void (*logToFile)(int max_depth, const char* filename);
	void (*logToClipboard)(int max_depth);
	void (*logFinish)();
	void (*logButtons)();
	void (*logText)(const char* fmt, ...);



    // Text


    // Scintilla Editor Widget
	
	PDSCInterface* (*scInputText)(const char* label, float xSize, float ySize, void (*callback)(void), void* userData);

    // Widgets

    int (*checkbox)(const char* label, bool* v);
    int (*buttonSmall)(const char* label);

    int (*selectableFixed)(const char* label, int selected, int flags, PDVec2 size); 
    int (*selectable)(const char* label, int* selected, int flags, PDVec2 size);

    // Misc

	PDRect (*getCurrentClipRect)();

    // Mouse

	PDVec2 (*getMousePos)();
	PDVec2 (*getMouseScreenPos)();
	int (*isMouseClicked)(int button, int repeat);
	int (*isMouseDoubleClicked)(int button);
	int (*isMouseHoveringBox)(PDVec2 boxMin, PDVec2 boxMax);
    int (*isItemHovered)();

	// Keyboard
	
	int (*isKeyDownId)(uint32_t keyId, int repeat);
	int (*isKeyDown)(int key, int repeat);
	int (*getKeyModifier)();
    void (*setKeyboardFocusHere)(int offset);

    // Styles

    void (*pushStyleVarV)(int styleVar /* PDStyleVar */, PDVec2 value);
    void (*pushStyleVarF)(int styleVar /* PDStyleVar */, float value);
    void (*popStyleVar)(int count);
	
    // Rendering

	void (*fillRect)(PDRect rect, unsigned int color); 

	// Id

	void (*pushIdPtr)(void* id);
	void (*pushIdInt)(int id);
	void (*popId)();

	//test
    void* privateData;

} PDUI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUI_collumns(uiFuncs, count, id, border) uiFuncs->columns(count, id, border)
#define PDUI_nextColumn(uiFuncs) uiFuncs->nextColumn()
#define PDUI_text(uiFuncs, format, ...) uiFuncs->text(format, __VA_ARGS__)
#define PDUI_textWrapped(uiFuncs, format, ...) uiFuncs->textWrapped(format, __VA_ARGS__)

#define PDUI_button(uiFuncs, label) uiFuncs->button(label)
#define PDUI_buttonSmall(uiFuncs, label) uiFuncs->buttonSmall(label)
#define PDUI_buttonSize(uiFuncs, label, w, h) uiFuncs->button(label, w, h)

#define PDUI_SCSendCommand(funcs, msg, p0, p1) funcs->sendCommand(funcs->privateData, msg, p0, p1)
#define PDUI_SCDraw(funcs) funcs->update(funcs->privateData)
#define PDUI_SCUpdate(funcs) funcs->draw(funcs->privateData)

#define PDUI_setTitle(funcs, title) funcs->setTitle(funcs->privateData, title)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif


