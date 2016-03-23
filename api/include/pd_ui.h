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

typedef struct PDUI {
	// data
    void* private_data;

	// Window

	void (*set_title)(void* private_data, const char* title);
	PDVec2 (*get_window_size)();
	PDVec2 (*get_window_pos)();

    void (*begin_child)(const char* stringId, PDVec2 size, bool border, int extraFlags /* PDUIWindowFlags */);
    void (*end_child)();

	float (*get_scroll_y)();
	float (*get_scroll_max_y)();
	void (*set_scroll_y)(float scrollY);
	void (*set_scroll_here)(float centerYratio);
	void (*set_scroll_from_pos_y)(float posY, float centerYratio);
	void (*set_keyboard_focus_here)(int offset);

	// Parameters stacks (shared)
	void (*push_font)(PDUIFont font);
	void (*pop_font)();
	void (*push_style_color)(PDUICol idx, PDColor col);
	void (*pop_style_color)(int count);
	void (*push_style_var)(PDUIStyleVar idx, float val);
	void (*push_style_varVec)(PDUIStyleVar idx, PDVec2 val);
	void (*pop_style_var)(int count);

	// Parameters stacks (current window)
	void (*push_item_width)(float item_width);
	void (*pop_item_width)();
	float (*calc_item_width)();
	void (*push_allow_keyboard_focus)(bool v);
	void (*pop_allow_keyboard_focus)();
	void (*push_text_wrap_pos)(float wrapPosX);
	void (*pop_text_wrap_pos)();
	void (*push_button_repeat)(bool repeat);
	void (*pop_button_repeat)();

	// Layout
	void (*begin_group)();
	void (*end_group)();
	void (*separator)();
	void (*same_line)(int columnX, int spacingW);
	void (*spacing)();
	void (*dummy)(PDVec2 size);
	void (*indent)();
	void (*un_indent)();
	void (*columns)(int count, const char* id, bool border);
	void (*next_column)();
	int (*get_column_index)();
	float(*get_column_offset)(int column_index);
	void (*set_column_offset)(int column_index, float offset_x);
	float (*get_column_width)(int column_index);
	int (*get_columns_count)();
	PDVec2 (*get_cursor_pos)();
	float (*get_cursor_pos_x)();
	float (*get_cursor_pos_y)();
	void (*set_cursor_pos)(PDVec2 pos);
	void (*set_cursor_pos_x)(float x);
	void (*set_cursor_pos_y)(float y);
	PDVec2 (*get_cursor_screen_pos)();
	void (*set_cursor_screen_pos)(PDVec2 pos);
	void (*align_first_text_height_to_widgets)();
	float (*get_text_line_height)();
	float (*get_text_line_height_with_spacing)();
	float (*get_items_line_height_with_spacing)();

	// ID scopes
	// If you are creating widgets in a loop you most likely want to push a unique identifier so PDUI can differentiate them
	// You can also use "##extra" within your widget name to distinguish them from each others (see 'Programmer Guide')
	void (*push_id_str)(const char* strId);
	void (*push_id_str_range)(const char* strBegin, const char* strEnd);
	void (*push_id_ptr)(const void* ptrId);
	void (*push_id_int)(const int intId);
	void (*pop_id)();
	PDID (*get_id_str)(const char* strId);
	PDID (*get_id_str_range)(const char* strBegin, const char* strEnd);
	PDID (*get_id_ptr)(const void* ptrId);

	// Widgets
	void (*text)(const char* fmt, ...);
	void (*text_v)(const char* fmt, va_list args);
	void (*text_colored)(const PDColor col, const char* fmt, ...);
	void (*text_colored_v)(const PDColor col, const char* fmt, va_list args);
	void (*text_disabled)(const char* fmt, ...);
	void (*text_disabled_v)(const char* fmt, va_list args);
	void (*text_wrapped)(const char* fmt, ...);
	void (*text_wrapped_v)(const char* fmt, va_list args);
	void (*text_unformatted)(const char* text, const char* text_end);
	void (*label_text)(const char* label, const char* fmt, ...);
	void (*label_text_v)(const char* label, const char* fmt, va_list args);
	void (*bullet)();
	void (*bullet_text)(const char* fmt, ...);
	void (*bullet_text_v)(const char* fmt, va_list args);
	bool (*button)(const char* label, const PDVec2 size);
	bool (*small_button)(const char* label);
	bool (*invisible_button)(const char* strId, const PDVec2 size);
	void (*image)(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, const PDColor tintColor, const PDColor borderColor);
	bool (*image_button)(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, int framePadding, const PDColor bgColor, const PDColor tintCol);
	bool (*collapsing_header)(const char* label, const char* strId, bool displayFrame, bool defaultOpen);
	bool (*checkbox)(const char* label, bool* v);
	bool (*checkbox_flags)(const char* label, unsigned int* flags, unsigned int flagsValue);
	bool (*radio_buttonBool)(const char* label, bool active);
	bool (*radio_button)(const char* label, int* v, int v_button);
	bool (*combo)(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems);
	bool (*combo2)(const char* label, int* currentItem, const char* itemsSeparatedByZeros, int heightInItems);
	bool (*combo3)(const char* label, int* currentItem, bool(*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems);
	bool (*color_button)(const PDColor col, bool smallHeight, bool outlineBorder);
	bool (*color_edit3)(const char* label, float col[3]);
	bool (*color_edit4)(const char* label, float col[4], bool showAlpha);
	void (*color_edit_mode)(PDUIColorEditMode mode);
	void (*plot_lines)(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride);
	void (*plot_lines2)(const char* label, float(*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize);
	void (*plot_histogram)(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride);
	void (*plot_histogram2)(const char* label, float(*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize);

	// Widgets: Scintilla text interface
	PDUISCInterface* (*sc_input_text)(const char* label, float xSize, float ySize, void (*callback)(void*), void* user_data);

	// Widgets: Sliders (tip: ctrl+click on a slider to input text)
	bool (*slider_float)(const char* label, float* v, float vMin, float vMax, const char* displayFormat, float power);
	bool (*slider_float2)(const char* label, float v[2], float vMin, float vMax, const char* displayFormat, float power);
	bool (*slider_float3)(const char* label, float v[3], float vMin, float vMax, const char* displayFormat, float power);
	bool (*slider_float4)(const char* label, float v[4], float vMin, float vMax, const char* displayFormat, float power);
	bool (*slider_angle)(const char* label, float* v_rad, float vDegreesMin, float vDegreesMax);
	bool (*slider_int)(const char* label, int* v, int vMin, int vMax, const char* displayFormat);
	bool (*slider_int2)(const char* label, int v[2], int vMin, int vMax, const char* displayFormat);
	bool (*slider_int3)(const char* label, int v[3], int vMin, int vMax, const char* displayFormat);
	bool (*slider_int4)(const char* label, int v[4], int vMin, int vMax, const char* displayFormat);
	bool (*vslider_float)(const char* label, const PDVec2 size, float* v, float vMin, float vMax, const char* displayFormat, float power);
	bool (*vslider_int)(const char* label, const PDVec2 size, int* v, int vMin, int vMax, const char* displayFormat);

	// Widgets: Drags (tip: ctrl+click on a drag box to input text)
	bool (*drag_float)(const char* label, float* v, float vSpeed, float vMin, float vMax, const char* displayFormat, float power);     // If vMax >= vMax we have no bound
	bool (*drag_float2)(const char* label, float v[2], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
	bool (*drag_float3)(const char* label, float v[3], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
	bool (*drag_float4)(const char* label, float v[4], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
	bool (*drag_int)(const char* label, int* v, float vSpeed, int vMin, int vMax, const char* displayFormat);                                       // If vMax >= vMax we have no bound
	bool (*drag_int2)(const char* label, int v[2], float vSpeed, int vMin, int vMax, const char* displayFormat);
	bool (*drag_int3)(const char* label, int v[3], float vSpeed, int vMin, int vMax, const char* displayFormat);
	bool (*drag_int4)(const char* label, int v[4], float vSpeed, int vMin, int vMax, const char* displayFormat);

	// Widgets: Input
    bool (*input_text)(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDUIInputTextCallbackData*), void* user_data);
	bool (*input_text_multiline)(const char* label, char* buf, size_t buf_size, const PDVec2 size, PDUIInputTextFlags flags, void (*callback)(PDUIInputTextCallbackData*), void* user_data);
	bool (*input_float)(const char* label, float* v, float step, float step_fast, int decimal_precision, PDUIInputTextFlags extraFlags);
	bool (*input_float2)(const char* label, float v[2], int decimal_precision, PDUIInputTextFlags extraFlags);
	bool (*input_float3)(const char* label, float v[3], int decimal_precision, PDUIInputTextFlags extraFlags);
	bool (*input_float4)(const char* label, float v[4], int decimal_precision, PDUIInputTextFlags extraFlags);
	bool (*input_int)(const char* label, int* v, int step, int step_fast, PDUIInputTextFlags extraFlags);
	bool (*input_int2)(const char* label, int v[2], PDUIInputTextFlags extraFlags);
	bool (*input_int3)(const char* label, int v[3], PDUIInputTextFlags extraFlags);
	bool (*input_int4)(const char* label, int v[4], PDUIInputTextFlags extraFlags);

	// Widgets: Trees
	bool (*tree_node)(const char* str_label_id);
	bool (*tree_node_str)(const char* strId, const char* fmt, ...);
	bool (*tree_node_ptr)(const void* ptrId, const char* fmt, ...);
	bool (*tree_node_str_v)(const char* strId, const char* fmt, va_list args);
	bool (*tree_node_ptr_v)(const void* ptrId, const char* fmt, va_list args);
	void (*tree_push_str)(const char* strId);
	void (*tree_push_ptr)(const void* ptrId);
	void (*tree_pop)();
	void (*set_next_tree_node_opened)(bool opened, PDUISetCond cond);

	// Widgets: Selectable / Lists
	bool (*selectable)(const char* label, bool selected, PDUISelectableFlags flags, const PDVec2 size);
	bool (*selectable_ex)(const char* label, bool* p_selected, PDUISelectableFlags flags, const PDVec2 size);
	bool (*list_box)(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems);
	bool (*list_box2)(const char* label, int* currentItem, bool(*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems);
	bool (*list_box_header)(const char* label, const PDVec2 size);
	bool (*list_box_header2)(const char* label, int itemsCount, int heightInItems);
	void (*list_box_footer)();

	// Tooltip
	void (*set_tooltip)(const char* fmt, ...);
	void (*set_tooltip_v)(const char* fmt, va_list args);
	void (*begin_tooltip)();
	void (*end_tooltip)();

	// Widgets: Menus
	bool (*begin_main_menu_bar)();
	void (*end_main_menu_bar)();
	bool (*begin_menuBar)();
	void (*end_menu_bar)();
	bool (*begin_menu)(const char* label, bool enabled);
	void (*end_menu)();
	bool (*menu_item)(const char* label, const char* shortcut, bool selected, bool enabled);
	bool (*menu_itemPtr)(const char* label, const char* shortcut, bool* p_selected, bool enabled);

	// Popup
	void (*open_popup)(const char* strId);
	bool (*begin_popup)(const char* strId);
	bool (*begin_popup_modal)(const char* name, bool* p_opened, PDUIWindowFlags extraFlags);
	bool (*begin_popup_context_item)(const char* strId, int mouse_button);
	bool (*begin_popup_context_window)(bool also_over_items, const char* strId, int mouse_button);
	bool (*begin_popupContext_void)(const char* strId, int mouse_button);
	void (*end_popup)();
	void (*close_current_popup)();

	bool (*begin_popup_context)(void* priv_data);
	void (*end_popup_context)(void* priv_data);

	// Widgets: value() Helpers. Output single value in "name: value" format
	void (*value_bool)(const char* prefix, bool b);
	void (*value_int)(const char* prefix, int v);
	void (*value_u_int)(const char* prefix, unsigned int v);
	void (*value_float)(const char* prefix, float v, const char* float_format);
	void (*color)(const char* prefix, const PDColor col);

	// Logging: all text output from interface is redirected to tty/file/clipboard. Tree nodes are automatically opened.
	void (*log_to_tty)(int maxDepth);
	void (*log_to_file)(int maxDepth, const char* filename);
	void (*log_to_clipboard)(int maxDepth);
	void (*log_finish)();
	void (*log_buttons)();
	//void (*logText)(const char* fmt, ...); -- no logTextV which PDUI needs

	// Utilities
	bool (*is_item_hovered)();
	bool (*is_item_hovered_rect)();
	bool (*is_item_active)();
	bool (*is_item_visible)();
	bool (*is_any_item_hovered)();
	bool (*is_any_item_active)();
	PDVec2 (*get_item_rect_min)();
	PDVec2 (*get_item_rect_max)();
	PDVec2 (*get_item_rect_size)();
	bool (*is_window_hovered)();
	bool (*is_window_focused)();
	bool (*is_root_window_focused)();
	bool (*is_root_window_or_any_child_focused)();
	bool (*is_rect_visible)(const PDVec2 itemSize);
	bool (*is_pos_hovering_any_window)(const PDVec2 pos);
	float (*get_time)();
	int (*get_frame_count)();
	const char* (*get_style_col_name)(PDUICol idx);
	PDVec2 (*calc_item_rect_closest_point)(const PDVec2 pos, bool on_edge, float outward);
	PDVec2 (*calc_text_size)(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width);
	void (*calc_list_clipping)(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end);

	bool (*begin_childFrame)(PDID id, const struct PDVec2 size);
	void (*end_child_frame)();

	void (*color_convert_rg_bto_hsv)(float r, float g, float b, float* out_h, float* out_s, float* out_v);
	void (*color_convert_hs_vto_rgb)(float h, float s, float v, float* out_r, float* out_g, float* out_b);

	bool (*is_key_down)(int key_index);
	bool (*is_key_pressed)(int key_index, bool repeat);
	bool (*is_key_released)(int key_index);
	bool (*is_key_down_id)(uint32_t keyId, int repeat);
	bool (*is_mouse_down)(int button);
	bool (*is_mouse_clicked)(int button, bool repeat);
	bool (*is_mouse_double_clicked)(int button);
	bool (*is_mouse_released)(int button);
	bool (*is_mouse_hovering_window)();
	bool (*is_mouse_hovering_any_window)();
	bool (*is_mouse_hovering_rect)(const PDVec2 rectMin, const PDVec2 rectMax);
	bool (*is_mouse_dragging)(int button, float lockThreshold);
	PDVec2 (*get_mouse_pos)();
	PDVec2 (*get_mouse_drag_delta)(int button, float lockThreshold);
	void (*reset_mouse_drag_delta)(int button);
	PDUIMouseCursor (*get_mouse_cursor)();
	void (*set_mouse_cursor)(PDUIMouseCursor type);

    // Rendering

	void (*fill_rect)(PDRect rect, unsigned int color);


} PDUI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUI_collumns(uiFuncs, count, id, border) uiFuncs->columns(count, id, border)
#define PDUI_next_column(uiFuncs) uiFuncs->next_column()
#define PDUI_text(uiFuncs, format, ...) uiFuncs->text(format, __VA_ARGS__)
#define PDUI_text_wrapped(uiFuncs, format, ...) uiFuncs->text_wrapped(format, __VA_ARGS__)

#define PDUI_button(uiFuncs, label) uiFuncs->button(label)
#define PDUI_button_size(uiFuncs, label, w, h) uiFuncs->button(label, w, h)

#define PDUI_sc_send_command(funcs, msg, p0, p1) funcs->send_command(funcs->private_data, msg, p0, p1)
#define PDUI_sc_draw(funcs) funcs->update(funcs->private_data)
#define PDUI_sc_update(funcs) funcs->draw(funcs->private_data)

#define PDUI_set_title(funcs, title) funcs->set_title(funcs->private_data, title)

#define PDUI_begin_popup_context(funcs) funcs->begin_popup_context(funcs->private_data)
#define PDUI_end_popup_context(funcs) funcs->end_popup_context(funcs->private_data)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif


