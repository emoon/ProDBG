#ifndef _PDUIUI_H_
#define _PDUIUI_H_

#include "pd_common.h"
#include "pd_keys.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PDUIUIPainter;
struct PDRect;
typedef void* PDUITextureID;

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef uint32_t PDColor;
typedef uint32_t PDUIInputTextFlags;    // enum PDUIInputTextFlags_
typedef uint32_t PDID;
typedef uint32_t PDUISetCond; 
typedef uint32_t PDUIWindowFlags; 
typedef uint32_t PDUISelectableFlags;
typedef void* PDUIFont;

#define PDUI_COLOR(r, g, b, a) (((uint32_t)a << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (r))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDUIStyleVar
{
    PDUIStyleVar_Invalid = 0,
    PDUIStyleVar_Alpha,             // float
    PDUIStyleVar_WindowPadding,     // PDVec2
    PDUIStyleVar_WindowRounding,    // float
    PDUIStyleVar_FramePadding,      // PDVec2
    PDUIStyleVar_FrameRounding,     // float
    PDUIStyleVar_ItemSpacing,       // PDVec2
    PDUIStyleVar_ItemInnerSpacing,  // PDVec2
    PDUIStyleVar_TreeNodeSpacing,   // float
    PDUIStyleVar_Count
} PDUIStyleVar;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDUIWindowFlags_
{
    // Default: 0
    PDUIWindowFlags_NoTitleBar             = 1 << 0,   // Disable title-bar
    PDUIWindowFlags_NoResize               = 1 << 1,   // Disable user resizing with the lower-right grip
    PDUIWindowFlags_NoMove                 = 1 << 2,   // Disable user moving the window
    PDUIWindowFlags_NoScrollbar            = 1 << 3,   // Disable scrollbar (window can still scroll with mouse or programatically)
    PDUIWindowFlags_NoScrollWithMouse      = 1 << 4,   // Disable user scrolling with mouse wheel
    PDUIWindowFlags_NoCollapse             = 1 << 5,   // Disable user collapsing window by double-clicking on it
    PDUIWindowFlags_AlwaysAutoResize       = 1 << 6,   // Resize every window to its content every frame
    PDUIWindowFlags_ShowBorders            = 1 << 7,   // Show borders around windows and items
    PDUIWindowFlags_NoSavedSettings        = 1 << 8,   // Never load/save settings in .ini file
    PDUIWindowFlags_NoInputs               = 1 << 9,   // Disable catching mouse or keyboard inputs
    PDUIWindowFlags_MenuBar                = 1 << 10,  // Has a menu-bar
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDUIInputTextFlags_
{
    // Default: 0
    PDUIInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    PDUIInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    PDUIInputTextFlags_CharsUppercase      = 1 << 2,   // Turn a..z into A..Z
    PDUIInputTextFlags_CharsNoBlank        = 1 << 3,   // Filter out spaces, tabs
    PDUIInputTextFlags_AutoSelectAll       = 1 << 4,   // Select entire text when first taking mouse focus
    PDUIInputTextFlags_EnterReturnsTrue    = 1 << 5,   // Return 'true' when Enter is pressed (as opposed to when the value was modified)
    PDUIInputTextFlags_CallbackCompletion  = 1 << 6,   // Call user function on pressing TAB (for completion handling)
    PDUIInputTextFlags_CallbackHistory     = 1 << 7,   // Call user function on pressing Up/Down arrows (for history handling)
    PDUIInputTextFlags_CallbackAlways      = 1 << 8,   // Call user function every time
    PDUIInputTextFlags_CallbackCharFilter  = 1 << 9,   // Call user function to filter character. Modify data->EventChar to replace/filter input, or return 1 to discard character.
    PDUIInputTextFlags_AllowTabInput       = 1 << 10,  // Pressing TAB input a '\t' character into the text field
    PDUIInputTextFlags_CtrlEnterForNewLine = 1 << 11,  // In multi-line mode, allow exiting edition by pressing Enter. Ctrl+Enter to add new line (by default adds new lines with Enter).
    PDUIInputTextFlags_NoHorizontalScroll  = 1 << 12,  // Disable following the cursor horizontally
    PDUIInputTextFlags_AlwaysInsertMode    = 1 << 13,  // Insert mode
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Flags for selectable()

enum PDUISelectableFlags_
{
    // Default: 0
    PDUISelectableFlags_DontClosePopups    = 1 << 0,   // Clicking this don't close parent popup window
    PDUISelectableFlags_SpanAllColumns     = 1 << 1    // Selectable frame can span all columns (text will still fit in current column)
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumeration for pushStyleColor() / popStyleColor()

typedef enum PDUICol
{
    PDUICol_Text,
    PDUICol_TextDisabled,
    PDUICol_WindowBg,
    PDUICol_ChildWindowBg,
    PDUICol_Border,
    PDUICol_BorderShadow,
    PDUICol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    PDUICol_FrameBgHovered,
    PDUICol_FrameBgActive,
    PDUICol_TitleBg,
    PDUICol_TitleBgCollapsed,
    PDUICol_TitleBgActive,
    PDUICol_MenuBarBg,
    PDUICol_ScrollbarBg,
    PDUICol_ScrollbarGrab,
    PDUICol_ScrollbarGrabHovered,
    PDUICol_ScrollbarGrabActive,
    PDUICol_ComboBg,
    PDUICol_CheckMark,
    PDUICol_SliderGrab,
    PDUICol_SliderGrabActive,
    PDUICol_Button,
    PDUICol_ButtonHovered,
    PDUICol_ButtonActive,
    PDUICol_Header,
    PDUICol_HeaderHovered,
    PDUICol_HeaderActive,
    PDUICol_Column,
    PDUICol_ColumnHovered,
    PDUICol_ColumnActive,
    PDUICol_ResizeGrip,
    PDUICol_ResizeGripHovered,
    PDUICol_ResizeGripActive,
    PDUICol_CloseButton,
    PDUICol_CloseButtonHovered,
    PDUICol_CloseButtonActive,
    PDUICol_PlotLines,
    PDUICol_PlotLinesHovered,
    PDUICol_PlotHistogram,
    PDUICol_PlotHistogramHovered,
    PDUICol_TextSelectedBg,
    PDUICol_TooltipBg,
    PDUICol_ModalWindowDarkening,  // darken entire screen when a modal window is active
    PDUICol_COUNT
} PDUICol;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumeration for colorEditMode()

typedef enum PDUIColorEditMode
{
    PDUIColorEditMode_UserSelect = -2,
    PDUIColorEditMode_UserSelectShowButton = -1,
    PDUIColorEditMode_RGB = 0,
    PDUIColorEditMode_HSV = 1,
    PDUIColorEditMode_HEX = 2
} PDUIColorEditMode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumeration for getMouseCursor()

typedef enum PDUIMouseCursor
{
    PDUIMouseCursor_Arrow = 0,
    PDUIMouseCursor_TextInput,         // When hovering over InputText, etc.
    PDUIMouseCursor_Move,              // Unused
    PDUIMouseCursor_ResizeNS,          // Unused
    PDUIMouseCursor_ResizeEW,          // When hovering over a column
    PDUIMouseCursor_ResizeNESW,        // Unused
    PDUIMouseCursor_ResizeNWSE,        // When hovering over the bottom-right corner of a window
} PDUIMouseCursor;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Condition flags for setWindow***(), setNextWindow***(), setNextTreeNode***() functions
// All those functions treat 0 as a shortcut to PDUISetCond_Always

enum PDUISetCond_
{
    PDUISetCond_Always        = 1 << 0, // Set the variable
    PDUISetCond_Once          = 1 << 1, // Only set the variable on the first call per runtime session
    PDUISetCond_FirstUseEver  = 1 << 2, // Only set the variable if the window doesn't exist in the .ini file
    PDUISetCond_Appearing     = 1 << 3  // Only set the variable if the window is appearing after being inactive (or the first time)
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUIInputTextCallbackData
{
    PDUIInputTextFlags eventFlag;    // One of PDUIInputTextFlags_Callback* // Read-only
    PDUIInputTextFlags flags;        // What user passed to InputText()      // Read-only
    void* userData;      			 // What user passed to InputText()      // Read-only

    // CharFilter event:
    uint16_t eventChar;    // Character input                       // Read-write (replace character or set to zero)

    // Completion,History,Always events:
    uint16_t eventKey;      // Key pressed (Up/Down/TAB)            // Read-only
    char* buf;            	// Current text                         // Read-write (pointed data only)
    size_t bufSize;        	//                                      // Read-only
    bool bufDirty;       	// Set if you modify Buf directly       // Write
    int cursorPos;      	//                                      // Read-write
    int selectionStart; 	//                                      // Read-write (== to SelectionEnd when no selection)
    int selectionEnd;   	//                                      // Read-write

	void (*deleteChars)(struct PDUIInputTextCallbackData* data, int pos, int byteCount);
	void (*insertChars)(struct PDUIInputTextCallbackData* data, int pos, const char* text, const char* textEnd);

} PDUIInputTextCallbackData;

typedef int (*PDUITextEditCallback)(PDUIInputTextCallbackData* data);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUISCInterface
{
	intptr_t (*sendCommand)(void* privData, unsigned int message, uintptr_t p0, intptr_t p1);
	void (*update)(void* privData);
	void (*draw)(void* privData);
	void* privateData;
} PDUISCInterface;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUI
{
	// Window
	
	void (*setTitle)(void* privateData, const char* title);
	PDVec2 (*getWindowSize)();
	PDVec2 (*getWindowPos)();

    void (*beginChild)(const char* stringId, PDVec2 size, bool border, int extraFlags /* PDUIWindowFlags */);
    void (*endChild)();

	float (*getScrollY)();
	float (*getScrollMaxY)();
	void (*setScrollY)(float scrollY);
	void (*setScrollHere)(float centerYratio);
	void (*setScrollFromPosY)(float posY, float centerYratio);
	void (*setKeyboardFocusHere)(int offset);

	// Parameters stacks (shared)
	void (*pushFont)(PDUIFont font);
	void (*popFont)();
	void (*pushStyleColor)(PDUICol idx, PDColor col);
	void (*popStyleColor)(int count);
	void (*pushStyleVar)(PDUIStyleVar idx, float val);
	void (*pushStyleVarVec)(PDUIStyleVar idx, PDVec2 val);
	void (*popStyleVar)(int count);

	// Parameters stacks (current window)
	void (*pushItemWidth)(float item_width);
	void (*popItemWidth)();
	float (*calcItemWidth)();
	void (*pushAllowKeyboardFocus)(bool v);
	void (*popAllowKeyboardFocus)();
	void (*pushTextWrapPos)(float wrapPosX);
	void (*popTextWrapPos)();
	void (*pushButtonRepeat)(bool repeat);
	void (*popButtonRepeat)();

	// Layout
	void (*beginGroup)();
	void (*endGroup)();
	void (*separator)();
	void (*sameLine)(int columnX, int spacingW);
	void (*spacing)();
	void (*dummy)(PDVec2 size);
	void (*indent)();
	void (*unIndent)();
	void (*columns)(int count, const char* id, bool border);
	void (*nextColumn)();
	int (*getColumnIndex)();
	float(*getColumnOffset)(int column_index);
	void (*setColumnOffset)(int column_index, float offset_x);
	float (*getColumnWidth)(int column_index);
	int (*getColumnsCount)();
	PDVec2 (*getCursorPos)();
	float (*getCursorPosX)();
	float (*getCursorPosY)();
	void (*setCursorPos)(PDVec2 pos);
	void (*setCursorPosX)(float x);
	void (*setCursorPosY)(float y);
	PDVec2 (*getCursorScreenPos)();
	void (*setCursorScreenPos)(PDVec2 pos);
	void (*alignFirstTextHeightToWidgets)();
	float (*getTextLineHeight)();
	float (*getTextLineHeightWithSpacing)();
	float (*getItemsLineHeightWithSpacing)();

	// ID scopes
	// If you are creating widgets in a loop you most likely want to push a unique identifier so PDUI can differentiate them
	// You can also use "##extra" within your widget name to distinguish them from each others (see 'Programmer Guide')
	void (*pushIdStr)(const char* strId);
	void (*pushIdStrRange)(const char* strBegin, const char* strEnd);
	void (*pushIdPtr)(const void* ptrId);
	void (*pushIdInt)(const int intId);
	void (*popId)();
	PDID (*getIdStr)(const char* strId);
	PDID (*getIdStrRange)(const char* strBegin, const char* strEnd);
	PDID (*getIdPtr)(const void* ptrId);

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
	void (*image)(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, const PDColor tintColor, const PDColor borderColor);
	bool (*imageButton)(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, int framePadding, const PDColor bgColor, const PDColor tintCol);
	bool (*collapsingHeader)(const char* label, const char* strId, bool displayFrame, bool defaultOpen);
	bool (*checkbox)(const char* label, bool* v);
	bool (*checkboxFlags)(const char* label, unsigned int* flags, unsigned int flagsValue);
	bool (*radioButtonBool)(const char* label, bool active);
	bool (*radioButton)(const char* label, int* v, int v_button);
	bool (*combo)(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems);
	bool (*combo2)(const char* label, int* currentItem, const char* itemsSeparatedByZeros, int heightInItems);
	bool (*combo3)(const char* label, int* currentItem, bool(*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems);
	bool (*colorButton)(const PDColor col, bool smallHeight, bool outlineBorder);
	bool (*colorEdit3)(const char* label, float col[3]);
	bool (*colorEdit4)(const char* label, float col[4], bool showAlpha);
	void (*colorEditMode)(PDUIColorEditMode mode);
	void (*plotLines)(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride);
	void (*plotLines2)(const char* label, float(*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize);
	void (*plotHistogram)(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride);
	void (*plotHistogram2)(const char* label, float(*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize);

	// Widgets: Scintilla text interface
	PDUISCInterface* (*scInputText)(const char* label, float xSize, float ySize, void (*callback)(void*), void* userData);

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
    bool (*inputText)(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDUIInputTextCallbackData*), void* userData);
	bool (*inputTextMultiline)(const char* label, char* buf, size_t buf_size, const PDVec2 size, PDUIInputTextFlags flags, void (*callback)(PDUIInputTextCallbackData*), void* userData);
	bool (*inputFloat)(const char* label, float* v, float step, float step_fast, int decimal_precision, PDUIInputTextFlags extraFlags);
	bool (*inputFloat2)(const char* label, float v[2], int decimal_precision, PDUIInputTextFlags extraFlags);
	bool (*inputFloat3)(const char* label, float v[3], int decimal_precision, PDUIInputTextFlags extraFlags);
	bool (*inputFloat4)(const char* label, float v[4], int decimal_precision, PDUIInputTextFlags extraFlags);
	bool (*inputInt)(const char* label, int* v, int step, int step_fast, PDUIInputTextFlags extraFlags);
	bool (*inputInt2)(const char* label, int v[2], PDUIInputTextFlags extraFlags);
	bool (*inputInt3)(const char* label, int v[3], PDUIInputTextFlags extraFlags);
	bool (*inputInt4)(const char* label, int v[4], PDUIInputTextFlags extraFlags);

	// Widgets: Trees
	bool (*treeNode)(const char* str_label_id);
	bool (*treeNodeStr)(const char* strId, const char* fmt, ...);
	bool (*treeNodePtr)(const void* ptrId, const char* fmt, ...);
	bool (*treeNodeStrV)(const char* strId, const char* fmt, va_list args);
	bool (*treeNodePtrV)(const void* ptrId, const char* fmt, va_list args);
	void (*treePushStr)(const char* strId);
	void (*treePushPtr)(const void* ptrId);
	void (*treePop)();
	void (*setNextTreeNodeOpened)(bool opened, PDUISetCond cond);

	// Widgets: Selectable / Lists
	bool (*selectable)(const char* label, bool selected, PDUISelectableFlags flags, const PDVec2 size);
	bool (*selectableEx)(const char* label, bool* p_selected, PDUISelectableFlags flags, const PDVec2 size);
	bool (*listBox)(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems);
	bool (*listBox2)(const char* label, int* currentItem, bool(*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems);
	bool (*listBoxHeader)(const char* label, const PDVec2 size);
	bool (*listBoxHeader2)(const char* label, int itemsCount, int heightInItems);
	void (*listBoxFooter)();

	// Tooltip
	void (*setTooltip)(const char* fmt, ...);
	void (*setTooltipV)(const char* fmt, va_list args);
	void (*beginTooltip)();
	void (*endTooltip)();

	// Widgets: Menus
	bool (*beginMainMenuBar)();
	void (*endMainMenuBar)();
	bool (*beginMenuBar)();
	void (*endMenuBar)();
	bool (*beginMenu)(const char* label, bool enabled);
	void (*endMenu)();
	bool (*menuItem)(const char* label, const char* shortcut, bool selected, bool enabled);
	bool (*menuItemPtr)(const char* label, const char* shortcut, bool* p_selected, bool enabled);

	// Popup
	void (*openPopup)(const char* strId);
	bool (*beginPopup)(const char* strId);
	bool (*beginPopupModal)(const char* name, bool* p_opened, PDUIWindowFlags extraFlags);
	bool (*beginPopupContextItem)(const char* strId, int mouse_button);
	bool (*beginPopupContextWindow)(bool also_over_items, const char* strId, int mouse_button);
	bool (*beginPopupContextVoid)(const char* strId, int mouse_button);
	void (*endPopup)();
	void (*closeCurrentPopup)();

	// Widgets: value() Helpers. Output single value in "name: value" format
	void (*valueBool)(const char* prefix, bool b);
	void (*valueInt)(const char* prefix, int v);
	void (*valueUInt)(const char* prefix, unsigned int v);
	void (*valueFloat)(const char* prefix, float v, const char* float_format);
	void (*color)(const char* prefix, const PDColor col);

	// Logging: all text output from interface is redirected to tty/file/clipboard. Tree nodes are automatically opened.
	void (*logToTTY)(int maxDepth);
	void (*logToFile)(int maxDepth, const char* filename);
	void (*logToClipboard)(int maxDepth);
	void (*logFinish)();
	void (*logButtons)();
	//void (*logText)(const char* fmt, ...); -- no logTextV which PDUI needs
	
	// Utilities
	bool (*isItemHovered)();
	bool (*isItemHoveredRect)();
	bool (*isItemActive)();
	bool (*isItemVisible)();
	bool (*isAnyItemHovered)();
	bool (*isAnyItemActive)();
	PDVec2 (*getItemRectMin)();
	PDVec2 (*getItemRectMax)();
	PDVec2 (*getItemRectSize)();
	bool (*isWindowHovered)();
	bool (*isWindowFocused)();
	bool (*isRootWindowFocused)();
	bool (*isRootWindowOrAnyChildFocused)();
	bool (*isRectVisible)(const PDVec2 itemSize);
	bool (*isPosHoveringAnyWindow)(const PDVec2 pos);
	float (*getTime)();
	int (*getFrameCount)();
	const char* (*getStyleColName)(PDUICol idx);
	PDVec2 (*calcItemRectClosestPoint)(const PDVec2 pos, bool on_edge, float outward);
	PDVec2 (*calcTextSize)(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width);
	void (*calcListClipping)(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end);

	bool (*beginChildFrame)(PDID id, const struct PDVec2 size);
	void (*endChildFrame)();

	void (*colorConvertRGBtoHSV)(float r, float g, float b, float* out_h, float* out_s, float* out_v);
	void (*colorConvertHSVtoRGB)(float h, float s, float v, float* out_r, float* out_g, float* out_b);

	bool (*isKeyDown)(int key_index);
	bool (*isKeyPressed)(int key_index, bool repeat);
	bool (*isKeyReleased)(int key_index);
	bool (*isKeyDownId)(uint32_t keyId, int repeat);
	bool (*isMouseDown)(int button);
	bool (*isMouseClicked)(int button, bool repeat);
	bool (*isMouseDoubleClicked)(int button);
	bool (*isMouseReleased)(int button);
	bool (*isMouseHoveringWindow)();
	bool (*isMouseHoveringAnyWindow)();
	bool (*isMouseHoveringRect)(const PDVec2 rectMin, const PDVec2 rectMax);
	bool (*isMouseDragging)(int button, float lockThreshold);
	PDVec2 (*getMousePos)();
	PDVec2 (*getMouseDragDelta)(int button, float lockThreshold);
	void (*resetMouseDragDelta)(int button);
	PDUIMouseCursor (*getMouseCursor)();
	void (*setMouseCursor)(PDUIMouseCursor type);

    // Rendering

	void (*fillRect)(PDRect rect, unsigned int color); 

	// data

    void* privateData;

} PDUI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUI_collumns(uiFuncs, count, id, border) uiFuncs->columns(count, id, border)
#define PDUI_nextColumn(uiFuncs) uiFuncs->nextColumn()
#define PDUI_text(uiFuncs, format, ...) uiFuncs->text(format, __VA_ARGS__)
#define PDUI_textWrapped(uiFuncs, format, ...) uiFuncs->textWrapped(format, __VA_ARGS__)

#define PDUI_button(uiFuncs, label) uiFuncs->button(label)
//#define PDUI_buttonSmall(uiFuncs, label) uiFuncs->buttonSmall(label)
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


