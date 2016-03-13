
// Disable this warning until all code has been fixed
#[allow(non_upper_case_globals)]

extern crate libc;
extern crate bitflags;

use libc::{c_char, c_uchar, c_float, c_int, c_uint, c_ulonglong, c_longlong, c_ushort, c_void};


// struct PDVec2
// (float) x
// (float) y
//
#[repr(C)]
pub struct PDVec2 {
    pub x: c_float,
    pub y: c_float,
}

// struct PDVec4
// (float) x
// (float) y
// (float) z
// (float) w
//
#[repr(C)]
pub struct PDVec4 {
    pub x: c_float,
    pub y: c_float,
    pub z: c_float,
    pub w: c_float,
}

// struct PDUISCInterface
// (intptr_t (*)(void *, unsigned int, uintptr_t, intptr_t)) send_command [long long (*)(void *, unsigned int, unsigned long long, long long)]
// (void (*)(void *)) update [void (*)(void *)]
// (void (*)(void *)) draw [void (*)(void *)]
// (void *) private_data
//
#[repr(C)]
pub struct PDUISCInterface {
    pub send_command: Option<extern "C" fn(*mut c_void,
                                           c_uint,
                                           c_ulonglong,
                                           c_longlong)
                                           -> c_longlong>,
    pub update: Option<extern "C" fn(*mut c_void)>,
    pub draw: Option<extern "C" fn(*mut c_void)>,
    pub private_data: *mut c_void,
}

// struct PDUIInputTextCallbackData
// (PDUIInputTextFlags) event_flag [unsigned int]
// (PDUIInputTextFlags) flags [unsigned int]
// (void *) user_data
// (uint16_t) event_char [unsigned short]
// (uint16_t) event_key [unsigned short]
// (char *) buf
// (int) buf_size
// (int) buf_dirty
// (int) cursor_pos
// (int) selection_start
// (int) selection_end
// (void (*)(struct PDUIInputTextCallbackData *, int, int)) delete_chars [void (*)(struct PDUIInputTextCallbackData *, int, int)]
// (void (*)(struct PDUIInputTextCallbackData *, int, const char *, const char *)) insert_chars [void (*)(struct PDUIInputTextCallbackData *, int, const char *, const char *)]
//
#[repr(C)]
pub struct PDUIInputTextCallbackData {
    pub event_flag: c_uint,
    pub flags: c_uint,
    pub user_data: *mut c_void,
    pub event_char: c_ushort,
    pub event_key: c_ushort,
    pub buf: *mut c_char,
    pub buf_size: c_int,
    pub buf_dirty: c_int,
    pub cursor_pos: c_int,
    pub selection_start: c_int,
    pub selection_end: c_int,
    pub delete_chars: Option<extern "C" fn(*mut PDUIInputTextCallbackData,
                                           c_int,
                                           c_int)>,
    pub insert_chars: Option<extern "C" fn(*mut PDUIInputTextCallbackData,
                                           c_int,
                                           *const c_char,
                                           *const c_char)>,
}

#[repr(C)]
pub struct CPdUI {
	pub set_title: extern fn(*mut c_void, *const c_char),
	pub get_window_size: extern fn () -> PDVec2,
	pub get_window_pos: extern fn () -> PDVec2,
	pub begin_child: extern fn(*const c_char, PDVec2, c_int, c_int),
	pub end_child: extern fn (),
	pub get_scroll_y: extern fn () -> c_float,
	pub get_scroll_max_y: extern fn () -> c_float,
	pub set_scroll_y: extern fn(c_float),
	pub set_scroll_here: extern fn(c_float),
	pub set_scroll_from_pos_y: extern fn(c_float, c_float),
	pub set_keyboard_focus_here: extern fn(c_int),
	pub push_font: extern fn(*mut c_void),
	pub pop_font: *mut extern fn () -> c_void,
	pub push_style_color: extern fn(c_uint, c_uint),
	pub pop_style_color: extern fn(c_int),
	pub push_style_var: extern fn(c_uint, c_float),
	pub push_style_var_vec: extern fn(c_uint, PDVec2),
	pub pop_style_var: extern fn(c_int),
	pub push_item_width: extern fn(c_float),
	pub pop_item_width: *mut extern fn () -> c_void,
	pub calc_item_width: *mut extern fn () -> c_float,
	pub push_allow_keyboard_focus: extern fn(c_int),
	pub pop_allow_keyboard_focus: *mut extern fn () -> c_void,
	pub push_text_wrap_pos: extern fn(c_float),
	pub pop_text_wrap_pos: *mut extern fn () -> c_void,
	pub push_button_repeat: extern fn(c_int),
	pub pop_button_repeat: *mut extern fn () -> c_void,
	pub begin_group: *mut extern fn () -> c_void,
	pub end_group: *mut extern fn () -> c_void,
	pub separator: *mut extern fn () -> c_void,
	pub same_line: extern fn(c_int, c_int),
	pub spacing: *mut extern fn () -> c_void,
	pub dummy: extern fn(PDVec2),
	pub indent: *mut extern fn () -> c_void,
	pub un_indent: *mut extern fn () -> c_void,
	pub columns: extern fn(c_int, *const c_char, c_int),
	pub next_column: extern fn(),
	pub get_column_index: extern fn() -> c_int,
	pub get_column_offset: extern fn(c_int) -> c_float,
	pub set_column_offset: extern fn(c_int, c_float),
	pub get_column_width: extern fn(c_int) -> c_float,
	pub get_columns_count: extern fn () -> c_int,
	pub get_cursor_pos: extern fn () -> PDVec2,
	pub get_cursor_pos_x: extern fn () -> c_float,
	pub get_cursor_pos_y: extern fn () -> c_float,
	pub set_cursor_pos: extern fn(PDVec2),
	pub set_cursor_pos_x: extern fn(c_float),
	pub set_cursor_pos_y: extern fn(c_float),
	pub get_cursor_screen_pos: *mut extern fn () -> PDVec2,
	pub set_cursor_screen_pos: extern fn(PDVec2),
	pub align_first_text_height_to_widgets: *mut extern fn () -> c_void,
	pub get_text_line_height: *mut extern fn () -> c_float,
	pub get_text_line_height_with_spacing: *mut extern fn () -> c_float,
	pub get_items_line_height_with_spacing: *mut extern fn () -> c_float,
	pub push_id_str: extern fn(*const c_char),
	pub push_id_str_range: extern fn(*const c_char, *const c_char),
	pub push_id_ptr: extern fn(*const c_void),
	pub push_id_int: extern fn(c_int),
	pub pop_id: *mut extern fn () -> c_void,
	pub get_id_str: extern fn(*const c_char) -> c_uint,
	pub get_id_str_range: extern fn(*const c_char, *const c_char) -> c_uint,
	pub get_id_ptr: extern fn(*const c_void) -> c_uint,
	pub text: extern fn(*const c_char),
	pub text_v: extern fn(*const c_char, c_int),
	pub text_colored: extern fn(c_uint, *const c_char),
	pub text_colored_v: extern fn(c_uint, *const c_char, c_int),
	pub text_disabled: extern fn(*const c_char),
	pub text_disabled_v: extern fn(*const c_char, c_int),
	pub text_wrapped: extern fn(*const c_char),
	pub text_wrapped_v: extern fn(*const c_char, c_int),
	pub text_unformatted: extern fn(*const c_char, *const c_char),
	pub label_text: extern fn(*const c_char, *const c_char),
	pub label_text_v: extern fn(*const c_char, *const c_char, c_int),
	pub bullet: *mut extern fn () -> c_void,
	pub bullet_text: extern fn(*const c_char),
	pub bullet_text_v: extern fn(*const c_char, c_int),
	pub button: extern fn(*const c_char, PDVec2) -> c_int,
	pub small_button: extern fn(*const c_uchar) -> c_int,
	pub invisible_button: extern fn(*const c_char, PDVec2) -> c_int,
	pub image: extern fn(*mut c_void, PDVec2, PDVec2, PDVec2, c_uint, c_uint),
	pub image_button: extern fn(*mut c_void, PDVec2, PDVec2, PDVec2, c_int, c_uint, c_uint) -> c_int,
	pub collapsing_header: extern fn(*const c_char, *const c_char, c_int, c_int) -> c_int,
	pub checkbox: extern fn(*const c_char, *mut c_int) -> c_int,
	pub checkbox_flags: extern fn(*const c_char, *mut c_uint, c_uint) -> c_int,
	pub radio_buttonint: extern fn(*const c_char, c_int) -> c_int,
	pub radio_button: extern fn(*const c_char, *mut c_int, c_int) -> c_int,
	pub combo: extern fn(*const c_char, *mut c_int, *mut *const c_char, c_int, c_int) -> c_int,
	pub combo2: extern fn(*const c_char, *mut c_int, *const c_char, c_int) -> c_int,
	pub combo3: extern fn(*const c_char, *mut c_int, extern fn(*mut c_void, c_int, *mut *const c_char) -> c_int, *mut c_void, c_int, c_int) -> c_int,
	pub color_button: extern fn(c_uint, c_int, c_int) -> c_int,
	pub color_edit3: extern fn(*const c_char, *mut c_float) -> c_int,
	pub color_edit4: extern fn(*const c_char, *mut c_float, c_int) -> c_int,
	pub color_edit_mode: extern fn(c_uint),
	pub plot_lines: extern fn(*const c_char, *const c_float, c_int, c_int, *const c_char, c_float, c_float, PDVec2, c_int),
	pub plot_lines2: extern fn(*const c_char, extern fn(*mut c_void, c_int) -> c_float, *mut c_void, c_int, c_int, *const c_char, c_float, c_float, PDVec2),
	pub plot_histogram: extern fn(*const c_char, *const c_float, c_int, c_int, *const c_char, c_float, c_float, PDVec2, c_int),
	pub plot_histogram2: extern fn(*const c_char, extern fn(*mut c_void, c_int) -> c_float, *mut c_void, c_int, c_int, *const c_char, c_float, c_float, PDVec2),
	pub sc_input_text: extern fn(*const c_char, c_float, c_float, extern fn(*mut c_void), *mut c_void) -> *mut PDUISCInterface,
	pub slider_float: extern fn(*const c_char, *mut c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub slider_float2: extern fn(*const c_char, *mut c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub slider_float3: extern fn(*const c_char, *mut c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub slider_float4: extern fn(*const c_char, *mut c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub slider_angle: extern fn(*const c_char, *mut c_float, c_float, c_float) -> c_int,
	pub slider_int: extern fn(*const c_char, *mut c_int, c_int, c_int, *const c_char) -> c_int,
	pub slider_int2: extern fn(*const c_char, *mut c_int, c_int, c_int, *const c_char) -> c_int,
	pub slider_int3: extern fn(*const c_char, *mut c_int, c_int, c_int, *const c_char) -> c_int,
	pub slider_int4: extern fn(*const c_char, *mut c_int, c_int, c_int, *const c_char) -> c_int,
	pub vslider_float: extern fn(*const c_char, PDVec2, *mut c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub vslider_int: extern fn(*const c_char, PDVec2, *mut c_int, c_int, c_int, *const c_char) -> c_int,
	pub drag_float: extern fn(*const c_char, *mut c_float, c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub drag_float2: extern fn(*const c_char, *mut c_float, c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub drag_float3: extern fn(*const c_char, *mut c_float, c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub drag_float4: extern fn(*const c_char, *mut c_float, c_float, c_float, c_float, *const c_char, c_float) -> c_int,
	pub drag_int: extern fn(*const c_char, *mut c_int, c_float, c_int, c_int, *const c_char) -> c_int,
	pub drag_int2: extern fn(*const c_char, *mut c_int, c_float, c_int, c_int, *const c_char) -> c_int,
	pub drag_int3: extern fn(*const c_char, *mut c_int, c_float, c_int, c_int, *const c_char) -> c_int,
	pub drag_int4: extern fn(*const c_char, *mut c_int, c_float, c_int, c_int, *const c_char) -> c_int,
	pub input_text: extern fn(*const c_char, *mut c_char, c_int, c_int, extern fn(*mut PDUIInputTextCallbackData), *mut c_void) -> c_int,
	pub input_text_multiline: extern fn(*const c_char, *mut c_char, c_int, PDVec2, c_uint, extern fn(*mut PDUIInputTextCallbackData), *mut c_void) -> c_int,
	pub input_float: extern fn(*const c_char, *mut c_float, c_float, c_float, c_int, c_uint) -> c_int,
	pub input_float2: extern fn(*const c_char, *mut c_float, c_int, c_uint) -> c_int,
	pub input_float3: extern fn(*const c_char, *mut c_float, c_int, c_uint) -> c_int,
	pub input_float4: extern fn(*const c_char, *mut c_float, c_int, c_uint) -> c_int,
	pub input_int: extern fn(*const c_char, *mut c_int, c_int, c_int, c_uint) -> c_int,
	pub input_int2: extern fn(*const c_char, *mut c_int, c_uint) -> c_int,
	pub input_int3: extern fn(*const c_char, *mut c_int, c_uint) -> c_int,
	pub input_int4: extern fn(*const c_char, *mut c_int, c_uint) -> c_int,
	pub tree_node: extern fn(*const c_char) -> c_int,
	pub tree_node_str: extern fn(*const c_char, *const c_char) -> c_int,
	pub tree_node_ptr: extern fn(*const c_void, *const c_char) -> c_int,
	pub tree_node_str_v: extern fn(*const c_char, *const c_char, c_int) -> c_int,
	pub tree_node_ptr_v: extern fn(*const c_void, *const c_char, c_int) -> c_int,
	pub tree_push_str: extern fn(*const c_char),
	pub tree_push_ptr: extern fn(*const c_void),
	pub tree_pop: *mut extern fn () -> c_void,
	pub set_next_tree_node_opened: extern fn(c_int, c_uint),
	pub selectable: extern fn(*const c_char, c_int, c_uint, PDVec2) -> c_int,
	pub selectable_ex: extern fn(*const c_char, *mut c_int, c_uint, PDVec2) -> c_int,
	pub list_box: extern fn(*const c_char, *mut c_int, *mut *const c_char, c_int, c_int) -> c_int,
	pub list_box2: extern fn(*const c_char, *mut c_int, extern fn(*mut c_void, c_int, *mut *const c_char) -> c_int, *mut c_void, c_int, c_int) -> c_int,
	pub list_box_header: extern fn(*const c_char, PDVec2) -> c_int,
	pub list_box_header2: extern fn(*const c_char, c_int, c_int) -> c_int,
	pub list_box_footer: *mut extern fn () -> c_void,
	pub set_tooltip: extern fn(*const c_char),
	pub set_tooltip_v: extern fn(*const c_char, c_int),
	pub begin_tooltip: *mut extern fn () -> c_void,
	pub end_tooltip: *mut extern fn () -> c_void,
	pub begin_main_menu_bar: *mut extern fn () -> c_int,
	pub end_main_menu_bar: *mut extern fn () -> c_void,
	pub begin_menu_bar: *mut extern fn () -> c_int,
	pub end_menu_bar: *mut extern fn () -> c_void,
	pub begin_menu: extern fn(*const c_char, c_int) -> c_int,
	pub end_menu: *mut extern fn () -> c_void,
	pub menu_item: extern fn(*const c_char, *const c_char, c_int, c_int) -> c_int,
	pub menu_item_ptr: extern fn(*const c_char, *const c_char, *mut c_int, c_int) -> c_int,
	pub open_popup: extern fn(*const c_char),
	pub begin_popup: extern fn(*const c_char) -> c_int,
	pub begin_popup_modal: extern fn(*const c_char, *mut c_int, c_uint) -> c_int,
	pub begin_popup_context_item: extern fn(*const c_char, c_int) -> c_int,
	pub begin_popup_context_window: extern fn(c_int, *const c_char, c_int) -> c_int,
	pub begin_popup_context_void: extern fn(*const c_char, c_int) -> c_int,
	pub end_popup: *mut extern fn () -> c_void,
	pub close_current_popup: *mut extern fn () -> c_void,
	pub value_int: extern fn(*const c_char, c_int),
	pub value_u_int: extern fn(*const c_char, c_uint),
	pub value_float: extern fn(*const c_char, c_float, *const c_char),
	pub color: extern fn(*const c_char, c_uint),
	pub log_to_tty: extern fn(c_int),
	pub log_to_file: extern fn(c_int, *const c_char),
	pub log_to_clipboard: extern fn(c_int),
	pub log_finish: *mut extern fn () -> c_void,
	pub log_buttons: *mut extern fn () -> c_void,
	pub is_item_hovered: *mut extern fn () -> c_int,
	pub is_item_hovered_rect: *mut extern fn () -> c_int,
	pub is_item_active: *mut extern fn () -> c_int,
	pub is_item_visible: *mut extern fn () -> c_int,
	pub is_any_item_hovered: *mut extern fn () -> c_int,
	pub is_any_item_active: *mut extern fn () -> c_int,
	pub get_item_rect_min: *mut extern fn () -> PDVec2,
	pub get_item_rect_max: *mut extern fn () -> PDVec2,
	pub get_item_rect_size: *mut extern fn () -> PDVec2,
	pub is_window_hovered: *mut extern fn () -> c_int,
	pub is_window_focused: *mut extern fn () -> c_int,
	pub is_root_window_focused: *mut extern fn () -> c_int,
	pub is_root_window_or_any_child_focused: *mut extern fn () -> c_int,
	pub is_rect_visible: extern fn(PDVec2) -> c_int,
	pub is_pos_hovering_any_window: extern fn(PDVec2) -> c_int,
	pub get_time: *mut extern fn () -> c_float,
	pub get_frame_count: *mut extern fn () -> c_int,
	pub get_style_col_name: extern fn(c_uint) -> *const c_char,
	pub calc_item_rect_closest_point: extern fn(PDVec2, c_int, c_float) -> PDVec2,
	pub calc_text_size: extern fn(*const c_char, *const c_char, c_int, c_float) -> PDVec2,
	pub calc_list_clipping: extern fn(c_int, c_float, *mut c_int, *mut c_int),
	pub begin_child_frame: extern fn(c_uint, PDVec2) -> c_int,
	pub end_child_frame: *mut extern fn () -> c_void,
	pub color_convert_rg_bto_hsv: extern fn(c_float, c_float, c_float, *mut c_float, *mut c_float, *mut c_float),
	pub color_convert_hs_vto_rgb: extern fn(c_float, c_float, c_float, *mut c_float, *mut c_float, *mut c_float),
	pub is_key_down: extern fn(c_int) -> c_int,
	pub is_key_pressed: extern fn(c_int, c_int) -> c_int,
	pub is_key_released: extern fn(c_int) -> c_int,
	pub is_key_down_id: extern fn(c_uint, c_int) -> c_int,
	pub is_mouse_down: extern fn(c_int) -> c_int,
	pub is_mouse_clicked: extern fn(c_int, c_int) -> c_int,
	pub is_mouse_double_clicked: extern fn(c_int) -> c_int,
	pub is_mouse_released: extern fn(c_int) -> c_int,
	pub is_mouse_hovering_window: *mut extern fn () -> c_int,
	pub is_mouse_hovering_any_window: *mut extern fn () -> c_int,
	pub is_mouse_hovering_rect: extern fn(PDVec2, PDVec2) -> c_int,
	pub is_mouse_dragging: extern fn(c_int, c_float) -> c_int,
	pub get_mouse_pos: *mut extern fn () -> PDVec2,
	pub get_mouse_drag_delta: extern fn(c_int, c_float) -> PDVec2,
	pub reset_mouse_drag_delta: extern fn(c_int),
	pub get_mouse_cursor: *mut extern fn () -> c_uint,
	pub set_mouse_cursor: extern fn(c_uint),
	pub fill_rect: extern fn(PDRect, c_uint),
	pub private_data: *mut c_void,
}

#[repr(C)]
pub struct PDRect {
    pub x: c_float,
    pub y: c_float,
    pub width: c_float,
    pub height: c_float,
}

bitflags! {
	flags PDUIWindowFlags_: c_uint {
		const PDUIWINDOWFLAGS_NOTITLEBAR = 1 as c_uint,
		const PDUIWINDOWFLAGS_NORESIZE = 2 as c_uint,
		const PDUIWINDOWFLAGS_NOMOVE =	4 as c_uint,
		const PDUIWINDOWFLAGS_NOSCROLLBAR =	8 as c_uint,
		const PDUIWINDOWFLAGS_NOSCROLLWITHMOUSE = 16 as c_uint,
		const PDUIWINDOWFLAGS_NOCOLLAPSE =	32 as c_uint,
		const PDUIWINDOWFLAGS_ALWAYSAUTORESIZE = 64 as c_uint,
		const PDUIWINDOWFLAGS_SHOWBORDERS =	128 as c_uint,
		const PDUIWINDOWFLAGS_NOSAVEDSETTINGS =	256 as c_uint,
		const PDUIWINDOWFLAGS_NOINPUTS = 512 as c_uint,
		const PDUIWINDOWFLAGS_MENUBAR =	1024 as c_uint,
	}
}

