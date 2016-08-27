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

typedef uint32_t PDColor;
typedef uint32_t PDUIInputTextFlags;    // enum PDUIInputTextFlags_
typedef uint32_t PDID;
typedef uint32_t PDUISetCond;
typedef uint32_t PDUIWindowFlags;
typedef uint32_t PDUISelectableFlags;  // enum PDUISelectableFlags__ // additional underscore to avoid conflicts with C++ code  
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

typedef struct PDUISCInterface {
	intptr_t (*send_command)(void* privData, unsigned int message, uintptr_t p0, intptr_t p1);
	void (*update)(void* privData);
	void (*draw)(void* privData);
	void* private_data;
} PDUISCInterface;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "pd_ui_autogen.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int (*PDUITextEditCallback)(PDUIInputTextCallbackData* data);

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


