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

typedef struct PDVec2 {
    float x, y;
} PDVec2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDVec4 {
    float x, y, z, w;
} PDVec4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDRect {
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

typedef enum PDUIStyleVar {
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

enum PDUIWindowFlags_ {
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

enum PDUIInputTextFlags_ {
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

enum PDUISelectableFlags_ {
    // Default: 0
    PDUISelectableFlags_DontClosePopups    = 1 << 0,   // Clicking this don't close parent popup window
    PDUISelectableFlags_SpanAllColumns     = 1 << 1    // Selectable frame can span all columns (text will still fit in current column)
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumeration for push_style_color() / pop_style_color()

typedef enum PDUICol {
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
// Enumeration for color_edit_mode()

typedef enum PDUIColorEditMode {
    PDUIColorEditMode_UserSelect = -2,
    PDUIColorEditMode_UserSelectShowButton = -1,
    PDUIColorEditMode_RGB = 0,
    PDUIColorEditMode_HSV = 1,
    PDUIColorEditMode_HEX = 2
} PDUIColorEditMode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumeration for get_mouse_cursor()

typedef enum PDUIMouseCursor {
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

enum PDUISetCond_ {
    PDUISetCond_Always        = 1 << 0, // Set the variable
    PDUISetCond_Once          = 1 << 1, // Only set the variable on the first call per runtime session
    PDUISetCond_FirstUseEver  = 1 << 2, // Only set the variable if the window doesn't exist in the .ini file
    PDUISetCond_Appearing     = 1 << 3  // Only set the variable if the window is appearing after being inactive (or the first time)
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUIInputTextCallbackData {
    PDUIInputTextFlags event_flag;    // One of PDUIInputTextFlags_Callback* // Read-only
    PDUIInputTextFlags flags;        // What user passed to InputText()      // Read-only
    void* user_data;      			 // What user passed to InputText()      // Read-only

    // CharFilter event:
    uint16_t event_char;    // Character input                       // Read-write (replace character or set to zero)

    // Completion,History,Always events:
    uint16_t event_key;      // Key pressed (Up/Down/TAB)            // Read-only
    char* buf;            	// Current text                         // Read-write (pointed data only)
    size_t buf_size;        	//                                      // Read-only
    bool buf_dirty;       	// Set if you modify Buf directly       // Write
    int cursor_pos;      	//                                      // Read-write
    int selection_start; 	//                                      // Read-write (== to SelectionEnd when no selection)
    int selection_end;   	//                                      // Read-write

	void (*delete_chars)(struct PDUIInputTextCallbackData* data, int pos, int byteCount);
	void (*insert_chars)(struct PDUIInputTextCallbackData* data, int pos, const char* text, const char* textEnd);

} PDUIInputTextCallbackData;

typedef int (*PDUITextEditCallback)(PDUIInputTextCallbackData* data);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUISCInterface {
	intptr_t (*send_command)(void* privData, unsigned int message, uintptr_t p0, intptr_t p1);
	void (*update)(void* privData);
	void (*draw)(void* privData);
	void* private_data;
} PDUISCInterface;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "pd_ui_autogen.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUI_collumns(uiFuncs, count, id, border) uiFuncs->columns(count, id, border)
#define PDUI_next_column(uiFuncs) uiFuncs->next_column()
#define PDUI_text(uiFuncs, format, ...) uiFuncs->text(format, __VA_ARGS__)
#define PDUI_text_wrapped(uiFuncs, format, ...) uiFuncs->text_wrapped(format, __VA_ARGS__)

#define PDUI_button(uiFuncs, label) uiFuncs->button(label)
#define PDUI_button_size(uiFuncs, label, w, h) uiFuncs->button(label, w, h)

#define PDUI_sc_send_command(funcs, msg, p0, p1) funcs->send_command(funcs->private_data, msg, p0, p1)
#define PDUI_sc_draw(funcs) funcs->draw(funcs->private_data)
#define PDUI_sc_update(funcs) funcs->update(funcs->private_data)

#define PDUI_set_title(funcs, title) funcs->set_title(funcs->private_data, title)

#define PDUI_begin_popup_context(funcs) funcs->begin_popup_context(funcs->private_data)
#define PDUI_end_popup_context(funcs) funcs->end_popup_context(funcs->private_data)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif


