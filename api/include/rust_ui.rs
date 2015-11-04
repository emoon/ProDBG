
extern crate libc;

#[macro_use]
extern crate bitflags;


/*
struct PDVec2
		(float) x
		(float) y
*/
#[repr(C)]
pub struct PDVec2 {
	pub x: libc::c_float,
	pub y: libc::c_float,
}

/*
struct PDVec4
		(float) x
		(float) y
		(float) z
		(float) w
*/
#[repr(C)]
pub struct PDVec4 {
	pub x: libc::c_float,
	pub y: libc::c_float,
	pub z: libc::c_float,
	pub w: libc::c_float,
}

/*
struct PDUISCInterface
		(intptr_t (*)(void *, unsigned int, uintptr_t, intptr_t)) send_command [long long (*)(void *, unsigned int, unsigned long long, long long)]
		(void (*)(void *)) update [void (*)(void *)]
		(void (*)(void *)) draw [void (*)(void *)]
		(void *) private_data
*/
#[repr(C)]
pub struct PDUISCInterface {
	pub send_command: Option<extern fn(*mut libc::c_void, libc::c_uint, libc::c_ulonglong, libc::c_longlong) -> libc::c_longlong>,
	pub update: Option<extern fn(*mut libc::c_void)>,
	pub draw: Option<extern fn(*mut libc::c_void)>,
	pub private_data: *mut libc::c_void,
}

/*
struct PDUIInputTextCallbackData
		(PDUIInputTextFlags) event_flag [unsigned int]
		(PDUIInputTextFlags) flags [unsigned int]
		(void *) user_data
		(uint16_t) event_char [unsigned short]
		(uint16_t) event_key [unsigned short]
		(char *) buf
		(int) buf_size
		(int) buf_dirty
		(int) cursor_pos
		(int) selection_start
		(int) selection_end
		(void (*)(struct PDUIInputTextCallbackData *, int, int)) delete_chars [void (*)(struct PDUIInputTextCallbackData *, int, int)]
		(void (*)(struct PDUIInputTextCallbackData *, int, const char *, const char *)) insert_chars [void (*)(struct PDUIInputTextCallbackData *, int, const char *, const char *)]
*/
#[repr(C)]
pub struct PDUIInputTextCallbackData {
	pub event_flag: libc::c_uint,
	pub flags: libc::c_uint,
	pub user_data: *mut libc::c_void,
	pub event_char: libc::c_ushort,
	pub event_key: libc::c_ushort,
	pub buf: *mut libc::c_char,
	pub buf_size: libc::c_int,
	pub buf_dirty: libc::c_int,
	pub cursor_pos: libc::c_int,
	pub selection_start: libc::c_int,
	pub selection_end: libc::c_int,
	pub delete_chars: Option<extern fn(*mut PDUIInputTextCallbackData, libc::c_int, libc::c_int)>,
	pub insert_chars: Option<extern fn(*mut PDUIInputTextCallbackData, libc::c_int, *const libc::c_char, *const libc::c_char)>,
}

/*
struct PDUIUIPainter
*/
#[repr(C)]
pub struct PDUIUIPainter;

/*
struct PDUI
		(void (*)(void *, const char *)) set_title [void (*)(void *, const char *)]
		(PDVec2 (*)()) get_window_size [struct PDVec2 (*)()]
		(PDVec2 (*)()) get_window_pos [struct PDVec2 (*)()]
		(void (*)(const char *, PDVec2, int, int)) begin_child [void (*)(const char *, struct PDVec2, int, int)]
		(void (*)()) end_child [void (*)()]
		(float (*)()) get_scroll_y [float (*)()]
		(float (*)()) get_scroll_max_y [float (*)()]
		(void (*)(float)) set_scroll_y [void (*)(float)]
		(void (*)(float)) set_scroll_here [void (*)(float)]
		(void (*)(float, float)) set_scroll_from_pos_y [void (*)(float, float)]
		(void (*)(int)) set_keyboard_focus_here [void (*)(int)]
		(void (*)(PDUIFont)) push_font [void (*)(void *)]
		(void (*)()) pop_font [void (*)()]
		(void (*)(PDUICol, PDColor)) push_style_color [void (*)(enum PDUICol, unsigned int)]
		(void (*)(int)) pop_style_color [void (*)(int)]
		(void (*)(PDUIStyleVar, float)) push_style_var [void (*)(enum PDUIStyleVar, float)]
		(void (*)(PDUIStyleVar, PDVec2)) push_style_varVec [void (*)(enum PDUIStyleVar, struct PDVec2)]
		(void (*)(int)) pop_style_var [void (*)(int)]
		(void (*)(float)) push_item_width [void (*)(float)]
		(void (*)()) pop_item_width [void (*)()]
		(float (*)()) calc_item_width [float (*)()]
		(void (*)(int)) push_allow_keyboard_focus [void (*)(int)]
		(void (*)()) pop_allow_keyboard_focus [void (*)()]
		(void (*)(float)) push_text_wrap_pos [void (*)(float)]
		(void (*)()) pop_text_wrap_pos [void (*)()]
		(void (*)(int)) push_button_repeat [void (*)(int)]
		(void (*)()) pop_button_repeat [void (*)()]
		(void (*)()) begin_group [void (*)()]
		(void (*)()) end_group [void (*)()]
		(void (*)()) separator [void (*)()]
		(void (*)(int, int)) same_line [void (*)(int, int)]
		(void (*)()) spacing [void (*)()]
		(void (*)(PDVec2)) dummy [void (*)(struct PDVec2)]
		(void (*)()) indent [void (*)()]
		(void (*)()) un_indent [void (*)()]
		(void (*)(int, const char *, int)) columns [void (*)(int, const char *, int)]
		(void (*)()) next_column [void (*)()]
		(int (*)()) get_column_index [int (*)()]
		(float (*)(int)) get_column_offset [float (*)(int)]
		(void (*)(int, float)) set_column_offset [void (*)(int, float)]
		(float (*)(int)) get_column_width [float (*)(int)]
		(int (*)()) get_columns_count [int (*)()]
		(PDVec2 (*)()) get_cursor_pos [struct PDVec2 (*)()]
		(float (*)()) get_cursor_pos_x [float (*)()]
		(float (*)()) get_cursor_pos_y [float (*)()]
		(void (*)(PDVec2)) set_cursor_pos [void (*)(struct PDVec2)]
		(void (*)(float)) set_cursor_pos_x [void (*)(float)]
		(void (*)(float)) set_cursor_pos_y [void (*)(float)]
		(PDVec2 (*)()) get_cursor_screen_pos [struct PDVec2 (*)()]
		(void (*)(PDVec2)) set_cursor_screen_pos [void (*)(struct PDVec2)]
		(void (*)()) align_first_text_height_to_widgets [void (*)()]
		(float (*)()) get_text_line_height [float (*)()]
		(float (*)()) get_text_line_height_with_spacing [float (*)()]
		(float (*)()) get_items_line_height_with_spacing [float (*)()]
		(void (*)(const char *)) push_id_str [void (*)(const char *)]
		(void (*)(const char *, const char *)) push_id_str_range [void (*)(const char *, const char *)]
		(void (*)(const void *)) push_id_ptr [void (*)(const void *)]
		(void (*)(const int)) push_id_int [void (*)(int)]
		(void (*)()) pop_id [void (*)()]
		(PDID (*)(const char *)) get_id_str [unsigned int (*)(const char *)]
		(PDID (*)(const char *, const char *)) get_id_str_range [unsigned int (*)(const char *, const char *)]
		(PDID (*)(const void *)) get_id_ptr [unsigned int (*)(const void *)]
		(void (*)(const char *, ...)) text [void (*)(const char *, ...)]
		(void (*)(const char *, int)) text_v [void (*)(const char *, int)]
		(void (*)(const PDColor, const char *, ...)) text_colored [void (*)(unsigned int, const char *, ...)]
		(void (*)(const PDColor, const char *, int)) text_colored_v [void (*)(unsigned int, const char *, int)]
		(void (*)(const char *, ...)) text_disabled [void (*)(const char *, ...)]
		(void (*)(const char *, int)) text_disabled_v [void (*)(const char *, int)]
		(void (*)(const char *, ...)) text_wrapped [void (*)(const char *, ...)]
		(void (*)(const char *, int)) text_wrapped_v [void (*)(const char *, int)]
		(void (*)(const char *, const char *)) text_unformatted [void (*)(const char *, const char *)]
		(void (*)(const char *, const char *, ...)) label_text [void (*)(const char *, const char *, ...)]
		(void (*)(const char *, const char *, int)) label_text_v [void (*)(const char *, const char *, int)]
		(void (*)()) bullet [void (*)()]
		(void (*)(const char *, ...)) bullet_text [void (*)(const char *, ...)]
		(void (*)(const char *, int)) bullet_text_v [void (*)(const char *, int)]
		(int (*)(const char *, const PDVec2)) button [int (*)(const char *, struct PDVec2)]
		(int (*)(const char *)) small_button [int (*)(const char *)]
		(int (*)(const char *, const PDVec2)) invisible_button [int (*)(const char *, struct PDVec2)]
		(void (*)(PDUITextureID, const PDVec2, const PDVec2, const PDVec2, const PDColor, const PDColor)) image [void (*)(void *, struct PDVec2, struct PDVec2, struct PDVec2, unsigned int, unsigned int)]
		(int (*)(PDUITextureID, const PDVec2, const PDVec2, const PDVec2, int, const PDColor, const PDColor)) image_button [int (*)(void *, struct PDVec2, struct PDVec2, struct PDVec2, int, unsigned int, unsigned int)]
		(int (*)(const char *, const char *, int, int)) collapsing_header [int (*)(const char *, const char *, int, int)]
		(int (*)(const char *, int *)) checkbox [int (*)(const char *, int *)]
		(int (*)(const char *, unsigned int *, unsigned int)) checkbox_flags [int (*)(const char *, unsigned int *, unsigned int)]
		(int (*)(const char *, int)) radio_buttonint [int (*)(const char *, int)]
		(int (*)(const char *, int *, int)) radio_button [int (*)(const char *, int *, int)]
		(int (*)(const char *, int *, const char **, int, int)) combo [int (*)(const char *, int *, const char **, int, int)]
		(int (*)(const char *, int *, const char *, int)) combo2 [int (*)(const char *, int *, const char *, int)]
		(int (*)(const char *, int *, int (*)(void *, int, const char **), void *, int, int)) combo3 [int (*)(const char *, int *, int (*)(void *, int, const char **), void *, int, int)]
		(int (*)(const PDColor, int, int)) color_button [int (*)(unsigned int, int, int)]
		(int (*)(const char *, float *)) color_edit3 [int (*)(const char *, float *)]
		(int (*)(const char *, float *, int)) color_edit4 [int (*)(const char *, float *, int)]
		(void (*)(PDUIColorEditMode)) color_edit_mode [void (*)(enum PDUIColorEditMode)]
		(void (*)(const char *, const float *, int, int, const char *, float, float, PDVec2, int)) plot_lines [void (*)(const char *, const float *, int, int, const char *, float, float, struct PDVec2, int)]
		(void (*)(const char *, float (*)(void *, int), void *, int, int, const char *, float, float, PDVec2)) plot_lines2 [void (*)(const char *, float (*)(void *, int), void *, int, int, const char *, float, float, struct PDVec2)]
		(void (*)(const char *, const float *, int, int, const char *, float, float, PDVec2, int)) plot_histogram [void (*)(const char *, const float *, int, int, const char *, float, float, struct PDVec2, int)]
		(void (*)(const char *, float (*)(void *, int), void *, int, int, const char *, float, float, PDVec2)) plot_histogram2 [void (*)(const char *, float (*)(void *, int), void *, int, int, const char *, float, float, struct PDVec2)]
		(PDUISCInterface *(*)(const char *, float, float, void (*)(void *), void *)) sc_input_text [struct PDUISCInterface *(*)(const char *, float, float, void (*)(void *), void *)]
		(int (*)(const char *, float *, float, float, const char *, float)) slider_float [int (*)(const char *, float *, float, float, const char *, float)]
		(int (*)(const char *, float *, float, float, const char *, float)) slider_float2 [int (*)(const char *, float *, float, float, const char *, float)]
		(int (*)(const char *, float *, float, float, const char *, float)) slider_float3 [int (*)(const char *, float *, float, float, const char *, float)]
		(int (*)(const char *, float *, float, float, const char *, float)) slider_float4 [int (*)(const char *, float *, float, float, const char *, float)]
		(int (*)(const char *, float *, float, float)) slider_angle [int (*)(const char *, float *, float, float)]
		(int (*)(const char *, int *, int, int, const char *)) slider_int [int (*)(const char *, int *, int, int, const char *)]
		(int (*)(const char *, int *, int, int, const char *)) slider_int2 [int (*)(const char *, int *, int, int, const char *)]
		(int (*)(const char *, int *, int, int, const char *)) slider_int3 [int (*)(const char *, int *, int, int, const char *)]
		(int (*)(const char *, int *, int, int, const char *)) slider_int4 [int (*)(const char *, int *, int, int, const char *)]
		(int (*)(const char *, const PDVec2, float *, float, float, const char *, float)) vslider_float [int (*)(const char *, struct PDVec2, float *, float, float, const char *, float)]
		(int (*)(const char *, const PDVec2, int *, int, int, const char *)) vslider_int [int (*)(const char *, struct PDVec2, int *, int, int, const char *)]
		(int (*)(const char *, float *, float, float, float, const char *, float)) drag_float [int (*)(const char *, float *, float, float, float, const char *, float)]
		(int (*)(const char *, float *, float, float, float, const char *, float)) drag_float2 [int (*)(const char *, float *, float, float, float, const char *, float)]
		(int (*)(const char *, float *, float, float, float, const char *, float)) drag_float3 [int (*)(const char *, float *, float, float, float, const char *, float)]
		(int (*)(const char *, float *, float, float, float, const char *, float)) drag_float4 [int (*)(const char *, float *, float, float, float, const char *, float)]
		(int (*)(const char *, int *, float, int, int, const char *)) drag_int [int (*)(const char *, int *, float, int, int, const char *)]
		(int (*)(const char *, int *, float, int, int, const char *)) drag_int2 [int (*)(const char *, int *, float, int, int, const char *)]
		(int (*)(const char *, int *, float, int, int, const char *)) drag_int3 [int (*)(const char *, int *, float, int, int, const char *)]
		(int (*)(const char *, int *, float, int, int, const char *)) drag_int4 [int (*)(const char *, int *, float, int, int, const char *)]
		(int (*)(const char *, char *, int, int, void (*)(PDUIInputTextCallbackData *), void *)) input_text [int (*)(const char *, char *, int, int, void (*)(struct PDUIInputTextCallbackData *), void *)]
		(int (*)(const char *, char *, int, const PDVec2, PDUIInputTextFlags, void (*)(PDUIInputTextCallbackData *), void *)) input_text_multiline [int (*)(const char *, char *, int, struct PDVec2, unsigned int, void (*)(struct PDUIInputTextCallbackData *), void *)]
		(int (*)(const char *, float *, float, float, int, PDUIInputTextFlags)) input_float [int (*)(const char *, float *, float, float, int, unsigned int)]
		(int (*)(const char *, float *, int, PDUIInputTextFlags)) input_float2 [int (*)(const char *, float *, int, unsigned int)]
		(int (*)(const char *, float *, int, PDUIInputTextFlags)) input_float3 [int (*)(const char *, float *, int, unsigned int)]
		(int (*)(const char *, float *, int, PDUIInputTextFlags)) input_float4 [int (*)(const char *, float *, int, unsigned int)]
		(int (*)(const char *, int *, int, int, PDUIInputTextFlags)) input_int [int (*)(const char *, int *, int, int, unsigned int)]
		(int (*)(const char *, int *, PDUIInputTextFlags)) input_int2 [int (*)(const char *, int *, unsigned int)]
		(int (*)(const char *, int *, PDUIInputTextFlags)) input_int3 [int (*)(const char *, int *, unsigned int)]
		(int (*)(const char *, int *, PDUIInputTextFlags)) input_int4 [int (*)(const char *, int *, unsigned int)]
		(int (*)(const char *)) tree_node [int (*)(const char *)]
		(int (*)(const char *, const char *, ...)) tree_node_str [int (*)(const char *, const char *, ...)]
		(int (*)(const void *, const char *, ...)) tree_node_ptr [int (*)(const void *, const char *, ...)]
		(int (*)(const char *, const char *, int)) tree_node_str_v [int (*)(const char *, const char *, int)]
		(int (*)(const void *, const char *, int)) tree_node_ptr_v [int (*)(const void *, const char *, int)]
		(void (*)(const char *)) tree_push_str [void (*)(const char *)]
		(void (*)(const void *)) tree_push_ptr [void (*)(const void *)]
		(void (*)()) tree_pop [void (*)()]
		(void (*)(int, PDUISetCond)) set_next_tree_node_opened [void (*)(int, unsigned int)]
		(int (*)(const char *, int, PDUISelectableFlags, const PDVec2)) selectable [int (*)(const char *, int, unsigned int, struct PDVec2)]
		(int (*)(const char *, int *, PDUISelectableFlags, const PDVec2)) selectable_ex [int (*)(const char *, int *, unsigned int, struct PDVec2)]
		(int (*)(const char *, int *, const char **, int, int)) list_box [int (*)(const char *, int *, const char **, int, int)]
		(int (*)(const char *, int *, int (*)(void *, int, const char **), void *, int, int)) list_box2 [int (*)(const char *, int *, int (*)(void *, int, const char **), void *, int, int)]
		(int (*)(const char *, const PDVec2)) list_box_header [int (*)(const char *, struct PDVec2)]
		(int (*)(const char *, int, int)) list_box_header2 [int (*)(const char *, int, int)]
		(void (*)()) list_box_footer [void (*)()]
		(void (*)(const char *, ...)) set_tooltip [void (*)(const char *, ...)]
		(void (*)(const char *, int)) set_tooltip_v [void (*)(const char *, int)]
		(void (*)()) begin_tooltip [void (*)()]
		(void (*)()) end_tooltip [void (*)()]
		(int (*)()) begin_main_menu_bar [int (*)()]
		(void (*)()) end_main_menu_bar [void (*)()]
		(int (*)()) begin_menuBar [int (*)()]
		(void (*)()) end_menu_bar [void (*)()]
		(int (*)(const char *, int)) begin_menu [int (*)(const char *, int)]
		(void (*)()) end_menu [void (*)()]
		(int (*)(const char *, const char *, int, int)) menu_item [int (*)(const char *, const char *, int, int)]
		(int (*)(const char *, const char *, int *, int)) menu_itemPtr [int (*)(const char *, const char *, int *, int)]
		(void (*)(const char *)) open_popup [void (*)(const char *)]
		(int (*)(const char *)) begin_popup [int (*)(const char *)]
		(int (*)(const char *, int *, PDUIWindowFlags)) begin_popup_modal [int (*)(const char *, int *, unsigned int)]
		(int (*)(const char *, int)) begin_popup_context_item [int (*)(const char *, int)]
		(int (*)(int, const char *, int)) begin_popup_context_window [int (*)(int, const char *, int)]
		(int (*)(const char *, int)) begin_popupContext_void [int (*)(const char *, int)]
		(void (*)()) end_popup [void (*)()]
		(void (*)()) close_current_popup [void (*)()]
		(void (*)(const char *, int)) value_int [void (*)(const char *, int)]
		(void (*)(const char *, unsigned int)) value_u_int [void (*)(const char *, unsigned int)]
		(void (*)(const char *, float, const char *)) value_float [void (*)(const char *, float, const char *)]
		(void (*)(const char *, const PDColor)) color [void (*)(const char *, unsigned int)]
		(void (*)(int)) log_to_tty [void (*)(int)]
		(void (*)(int, const char *)) log_to_file [void (*)(int, const char *)]
		(void (*)(int)) log_to_clipboard [void (*)(int)]
		(void (*)()) log_finish [void (*)()]
		(void (*)()) log_buttons [void (*)()]
		(int (*)()) is_item_hovered [int (*)()]
		(int (*)()) is_item_hovered_rect [int (*)()]
		(int (*)()) is_item_active [int (*)()]
		(int (*)()) is_item_visible [int (*)()]
		(int (*)()) is_any_item_hovered [int (*)()]
		(int (*)()) is_any_item_active [int (*)()]
		(PDVec2 (*)()) get_item_rect_min [struct PDVec2 (*)()]
		(PDVec2 (*)()) get_item_rect_max [struct PDVec2 (*)()]
		(PDVec2 (*)()) get_item_rect_size [struct PDVec2 (*)()]
		(int (*)()) is_window_hovered [int (*)()]
		(int (*)()) is_window_focused [int (*)()]
		(int (*)()) is_root_window_focused [int (*)()]
		(int (*)()) is_root_window_or_any_child_focused [int (*)()]
		(int (*)(const PDVec2)) is_rect_visible [int (*)(struct PDVec2)]
		(int (*)(const PDVec2)) is_pos_hovering_any_window [int (*)(struct PDVec2)]
		(float (*)()) get_time [float (*)()]
		(int (*)()) get_frame_count [int (*)()]
		(const char *(*)(PDUICol)) get_style_col_name [const char *(*)(enum PDUICol)]
		(PDVec2 (*)(const PDVec2, int, float)) calc_item_rect_closest_point [struct PDVec2 (*)(struct PDVec2, int, float)]
		(PDVec2 (*)(const char *, const char *, int, float)) calc_text_size [struct PDVec2 (*)(const char *, const char *, int, float)]
		(void (*)(int, float, int *, int *)) calc_list_clipping [void (*)(int, float, int *, int *)]
		(int (*)(PDID, const struct PDVec2)) begin_childFrame [int (*)(unsigned int, struct PDVec2)]
		(void (*)()) end_child_frame [void (*)()]
		(void (*)(float, float, float, float *, float *, float *)) color_convert_rg_bto_hsv [void (*)(float, float, float, float *, float *, float *)]
		(void (*)(float, float, float, float *, float *, float *)) color_convert_hs_vto_rgb [void (*)(float, float, float, float *, float *, float *)]
		(int (*)(int)) is_key_down [int (*)(int)]
		(int (*)(int, int)) is_key_pressed [int (*)(int, int)]
		(int (*)(int)) is_key_released [int (*)(int)]
		(int (*)(uint32_t, int)) is_key_down_id [int (*)(unsigned int, int)]
		(int (*)(int)) is_mouse_down [int (*)(int)]
		(int (*)(int, int)) is_mouse_clicked [int (*)(int, int)]
		(int (*)(int)) is_mouse_double_clicked [int (*)(int)]
		(int (*)(int)) is_mouse_released [int (*)(int)]
		(int (*)()) is_mouse_hovering_window [int (*)()]
		(int (*)()) is_mouse_hovering_any_window [int (*)()]
		(int (*)(const PDVec2, const PDVec2)) is_mouse_hovering_rect [int (*)(struct PDVec2, struct PDVec2)]
		(int (*)(int, float)) is_mouse_dragging [int (*)(int, float)]
		(PDVec2 (*)()) get_mouse_pos [struct PDVec2 (*)()]
		(PDVec2 (*)(int, float)) get_mouse_drag_delta [struct PDVec2 (*)(int, float)]
		(void (*)(int)) reset_mouse_drag_delta [void (*)(int)]
		(PDUIMouseCursor (*)()) get_mouse_cursor [enum PDUIMouseCursor (*)()]
		(void (*)(PDUIMouseCursor)) set_mouse_cursor [void (*)(enum PDUIMouseCursor)]
		(void (*)(PDRect, unsigned int)) fill_rect [void (*)(struct PDRect, unsigned int)]
		(void *) private_data
*/
#[repr(C)]
pub struct PDUI {
	pub set_title: Option<extern fn(*mut libc::c_void, *const libc::c_char)>,
	pub get_window_size: *mut extern fn () -> PDVec2,
	pub get_window_pos: *mut extern fn () -> PDVec2,
	pub begin_child: Option<extern fn(*const libc::c_char, PDVec2, libc::c_int, libc::c_int)>,
	pub end_child: *mut extern fn () -> libc::c_void,
	pub get_scroll_y: *mut extern fn () -> libc::c_float,
	pub get_scroll_max_y: *mut extern fn () -> libc::c_float,
	pub set_scroll_y: Option<extern fn(libc::c_float)>,
	pub set_scroll_here: Option<extern fn(libc::c_float)>,
	pub set_scroll_from_pos_y: Option<extern fn(libc::c_float, libc::c_float)>,
	pub set_keyboard_focus_here: Option<extern fn(libc::c_int)>,
	pub push_font: Option<extern fn(*mut libc::c_void)>,
	pub pop_font: *mut extern fn () -> libc::c_void,
	pub push_style_color: Option<extern fn(libc::c_uint, libc::c_uint)>,
	pub pop_style_color: Option<extern fn(libc::c_int)>,
	pub push_style_var: Option<extern fn(libc::c_uint, libc::c_float)>,
	pub push_style_varVec: Option<extern fn(libc::c_uint, PDVec2)>,
	pub pop_style_var: Option<extern fn(libc::c_int)>,
	pub push_item_width: Option<extern fn(libc::c_float)>,
	pub pop_item_width: *mut extern fn () -> libc::c_void,
	pub calc_item_width: *mut extern fn () -> libc::c_float,
	pub push_allow_keyboard_focus: Option<extern fn(libc::c_int)>,
	pub pop_allow_keyboard_focus: *mut extern fn () -> libc::c_void,
	pub push_text_wrap_pos: Option<extern fn(libc::c_float)>,
	pub pop_text_wrap_pos: *mut extern fn () -> libc::c_void,
	pub push_button_repeat: Option<extern fn(libc::c_int)>,
	pub pop_button_repeat: *mut extern fn () -> libc::c_void,
	pub begin_group: *mut extern fn () -> libc::c_void,
	pub end_group: *mut extern fn () -> libc::c_void,
	pub separator: *mut extern fn () -> libc::c_void,
	pub same_line: Option<extern fn(libc::c_int, libc::c_int)>,
	pub spacing: *mut extern fn () -> libc::c_void,
	pub dummy: Option<extern fn(PDVec2)>,
	pub indent: *mut extern fn () -> libc::c_void,
	pub un_indent: *mut extern fn () -> libc::c_void,
	pub columns: Option<extern fn(libc::c_int, *const libc::c_char, libc::c_int)>,
	pub next_column: *mut extern fn () -> libc::c_void,
	pub get_column_index: *mut extern fn () -> libc::c_int,
	pub get_column_offset: Option<extern fn(libc::c_int) -> libc::c_float>,
	pub set_column_offset: Option<extern fn(libc::c_int, libc::c_float)>,
	pub get_column_width: Option<extern fn(libc::c_int) -> libc::c_float>,
	pub get_columns_count: *mut extern fn () -> libc::c_int,
	pub get_cursor_pos: *mut extern fn () -> PDVec2,
	pub get_cursor_pos_x: *mut extern fn () -> libc::c_float,
	pub get_cursor_pos_y: *mut extern fn () -> libc::c_float,
	pub set_cursor_pos: Option<extern fn(PDVec2)>,
	pub set_cursor_pos_x: Option<extern fn(libc::c_float)>,
	pub set_cursor_pos_y: Option<extern fn(libc::c_float)>,
	pub get_cursor_screen_pos: *mut extern fn () -> PDVec2,
	pub set_cursor_screen_pos: Option<extern fn(PDVec2)>,
	pub align_first_text_height_to_widgets: *mut extern fn () -> libc::c_void,
	pub get_text_line_height: *mut extern fn () -> libc::c_float,
	pub get_text_line_height_with_spacing: *mut extern fn () -> libc::c_float,
	pub get_items_line_height_with_spacing: *mut extern fn () -> libc::c_float,
	pub push_id_str: Option<extern fn(*const libc::c_char)>,
	pub push_id_str_range: Option<extern fn(*const libc::c_char, *const libc::c_char)>,
	pub push_id_ptr: Option<extern fn(*const libc::c_void)>,
	pub push_id_int: Option<extern fn(libc::c_int)>,
	pub pop_id: *mut extern fn () -> libc::c_void,
	pub get_id_str: Option<extern fn(*const libc::c_char) -> libc::c_uint>,
	pub get_id_str_range: Option<extern fn(*const libc::c_char, *const libc::c_char) -> libc::c_uint>,
	pub get_id_ptr: Option<extern fn(*const libc::c_void) -> libc::c_uint>,
	pub text: Option<extern fn(*const libc::c_char)>,
	pub text_v: Option<extern fn(*const libc::c_char, libc::c_int)>,
	pub text_colored: Option<extern fn(libc::c_uint, *const libc::c_char)>,
	pub text_colored_v: Option<extern fn(libc::c_uint, *const libc::c_char, libc::c_int)>,
	pub text_disabled: Option<extern fn(*const libc::c_char)>,
	pub text_disabled_v: Option<extern fn(*const libc::c_char, libc::c_int)>,
	pub text_wrapped: Option<extern fn(*const libc::c_char)>,
	pub text_wrapped_v: Option<extern fn(*const libc::c_char, libc::c_int)>,
	pub text_unformatted: Option<extern fn(*const libc::c_char, *const libc::c_char)>,
	pub label_text: Option<extern fn(*const libc::c_char, *const libc::c_char)>,
	pub label_text_v: Option<extern fn(*const libc::c_char, *const libc::c_char, libc::c_int)>,
	pub bullet: *mut extern fn () -> libc::c_void,
	pub bullet_text: Option<extern fn(*const libc::c_char)>,
	pub bullet_text_v: Option<extern fn(*const libc::c_char, libc::c_int)>,
	pub button: Option<extern fn(*const libc::c_char, PDVec2) -> libc::c_int>,
	pub small_button: Option<extern fn(*const libc::c_char) -> libc::c_int>,
	pub invisible_button: Option<extern fn(*const libc::c_char, PDVec2) -> libc::c_int>,
	pub image: Option<extern fn(*mut libc::c_void, PDVec2, PDVec2, PDVec2, libc::c_uint, libc::c_uint)>,
	pub image_button: Option<extern fn(*mut libc::c_void, PDVec2, PDVec2, PDVec2, libc::c_int, libc::c_uint, libc::c_uint) -> libc::c_int>,
	pub collapsing_header: Option<extern fn(*const libc::c_char, *const libc::c_char, libc::c_int, libc::c_int) -> libc::c_int>,
	pub checkbox: Option<extern fn(*const libc::c_char, *mut libc::c_int) -> libc::c_int>,
	pub checkbox_flags: Option<extern fn(*const libc::c_char, *mut libc::c_uint, libc::c_uint) -> libc::c_int>,
	pub radio_buttonint: Option<extern fn(*const libc::c_char, libc::c_int) -> libc::c_int>,
	pub radio_button: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_int) -> libc::c_int>,
	pub combo: Option<extern fn(*const libc::c_char, *mut libc::c_int, *mut *const libc::c_char, libc::c_int, libc::c_int) -> libc::c_int>,
	pub combo2: Option<extern fn(*const libc::c_char, *mut libc::c_int, *const libc::c_char, libc::c_int) -> libc::c_int>,
	pub combo3: Option<extern fn(*const libc::c_char, *mut libc::c_int, Option<extern fn(*mut libc::c_void, libc::c_int, *mut *const libc::c_char) -> libc::c_int>, *mut libc::c_void, libc::c_int, libc::c_int) -> libc::c_int>,
	pub color_button: Option<extern fn(libc::c_uint, libc::c_int, libc::c_int) -> libc::c_int>,
	pub color_edit3: Option<extern fn(*const libc::c_char, *mut libc::c_float) -> libc::c_int>,
	pub color_edit4: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_int) -> libc::c_int>,
	pub color_edit_mode: Option<extern fn(libc::c_uint)>,
	pub plot_lines: Option<extern fn(*const libc::c_char, *const libc::c_float, libc::c_int, libc::c_int, *const libc::c_char, libc::c_float, libc::c_float, PDVec2, libc::c_int)>,
	pub plot_lines2: Option<extern fn(*const libc::c_char, Option<extern fn(*mut libc::c_void, libc::c_int) -> libc::c_float>, *mut libc::c_void, libc::c_int, libc::c_int, *const libc::c_char, libc::c_float, libc::c_float, PDVec2)>,
	pub plot_histogram: Option<extern fn(*const libc::c_char, *const libc::c_float, libc::c_int, libc::c_int, *const libc::c_char, libc::c_float, libc::c_float, PDVec2, libc::c_int)>,
	pub plot_histogram2: Option<extern fn(*const libc::c_char, Option<extern fn(*mut libc::c_void, libc::c_int) -> libc::c_float>, *mut libc::c_void, libc::c_int, libc::c_int, *const libc::c_char, libc::c_float, libc::c_float, PDVec2)>,
	pub sc_input_text: Option<extern fn(*const libc::c_char, libc::c_float, libc::c_float, Option<extern fn(*mut libc::c_void)>, *mut libc::c_void) -> *mut PDUISCInterface>,
	pub slider_float: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub slider_float2: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub slider_float3: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub slider_float4: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub slider_angle: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float) -> libc::c_int>,
	pub slider_int: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub slider_int2: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub slider_int3: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub slider_int4: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub vslider_float: Option<extern fn(*const libc::c_char, PDVec2, *mut libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub vslider_int: Option<extern fn(*const libc::c_char, PDVec2, *mut libc::c_int, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub drag_float: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub drag_float2: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub drag_float3: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub drag_float4: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, libc::c_float, *const libc::c_char, libc::c_float) -> libc::c_int>,
	pub drag_int: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_float, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub drag_int2: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_float, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub drag_int3: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_float, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub drag_int4: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_float, libc::c_int, libc::c_int, *const libc::c_char) -> libc::c_int>,
	pub input_text: Option<extern fn(*const libc::c_char, *mut libc::c_char, libc::c_int, libc::c_int, Option<extern fn(*mut PDUIInputTextCallbackData)>, *mut libc::c_void) -> libc::c_int>,
	pub input_text_multiline: Option<extern fn(*const libc::c_char, *mut libc::c_char, libc::c_int, PDVec2, libc::c_uint, Option<extern fn(*mut PDUIInputTextCallbackData)>, *mut libc::c_void) -> libc::c_int>,
	pub input_float: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_float, libc::c_float, libc::c_int, libc::c_uint) -> libc::c_int>,
	pub input_float2: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_int, libc::c_uint) -> libc::c_int>,
	pub input_float3: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_int, libc::c_uint) -> libc::c_int>,
	pub input_float4: Option<extern fn(*const libc::c_char, *mut libc::c_float, libc::c_int, libc::c_uint) -> libc::c_int>,
	pub input_int: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_int, libc::c_int, libc::c_uint) -> libc::c_int>,
	pub input_int2: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_uint) -> libc::c_int>,
	pub input_int3: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_uint) -> libc::c_int>,
	pub input_int4: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_uint) -> libc::c_int>,
	pub tree_node: Option<extern fn(*const libc::c_char) -> libc::c_int>,
	pub tree_node_str: Option<extern fn(*const libc::c_char, *const libc::c_char) -> libc::c_int>,
	pub tree_node_ptr: Option<extern fn(*const libc::c_void, *const libc::c_char) -> libc::c_int>,
	pub tree_node_str_v: Option<extern fn(*const libc::c_char, *const libc::c_char, libc::c_int) -> libc::c_int>,
	pub tree_node_ptr_v: Option<extern fn(*const libc::c_void, *const libc::c_char, libc::c_int) -> libc::c_int>,
	pub tree_push_str: Option<extern fn(*const libc::c_char)>,
	pub tree_push_ptr: Option<extern fn(*const libc::c_void)>,
	pub tree_pop: *mut extern fn () -> libc::c_void,
	pub set_next_tree_node_opened: Option<extern fn(libc::c_int, libc::c_uint)>,
	pub selectable: Option<extern fn(*const libc::c_char, libc::c_int, libc::c_uint, PDVec2) -> libc::c_int>,
	pub selectable_ex: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_uint, PDVec2) -> libc::c_int>,
	pub list_box: Option<extern fn(*const libc::c_char, *mut libc::c_int, *mut *const libc::c_char, libc::c_int, libc::c_int) -> libc::c_int>,
	pub list_box2: Option<extern fn(*const libc::c_char, *mut libc::c_int, Option<extern fn(*mut libc::c_void, libc::c_int, *mut *const libc::c_char) -> libc::c_int>, *mut libc::c_void, libc::c_int, libc::c_int) -> libc::c_int>,
	pub list_box_header: Option<extern fn(*const libc::c_char, PDVec2) -> libc::c_int>,
	pub list_box_header2: Option<extern fn(*const libc::c_char, libc::c_int, libc::c_int) -> libc::c_int>,
	pub list_box_footer: *mut extern fn () -> libc::c_void,
	pub set_tooltip: Option<extern fn(*const libc::c_char)>,
	pub set_tooltip_v: Option<extern fn(*const libc::c_char, libc::c_int)>,
	pub begin_tooltip: *mut extern fn () -> libc::c_void,
	pub end_tooltip: *mut extern fn () -> libc::c_void,
	pub begin_main_menu_bar: *mut extern fn () -> libc::c_int,
	pub end_main_menu_bar: *mut extern fn () -> libc::c_void,
	pub begin_menuBar: *mut extern fn () -> libc::c_int,
	pub end_menu_bar: *mut extern fn () -> libc::c_void,
	pub begin_menu: Option<extern fn(*const libc::c_char, libc::c_int) -> libc::c_int>,
	pub end_menu: *mut extern fn () -> libc::c_void,
	pub menu_item: Option<extern fn(*const libc::c_char, *const libc::c_char, libc::c_int, libc::c_int) -> libc::c_int>,
	pub menu_itemPtr: Option<extern fn(*const libc::c_char, *const libc::c_char, *mut libc::c_int, libc::c_int) -> libc::c_int>,
	pub open_popup: Option<extern fn(*const libc::c_char)>,
	pub begin_popup: Option<extern fn(*const libc::c_char) -> libc::c_int>,
	pub begin_popup_modal: Option<extern fn(*const libc::c_char, *mut libc::c_int, libc::c_uint) -> libc::c_int>,
	pub begin_popup_context_item: Option<extern fn(*const libc::c_char, libc::c_int) -> libc::c_int>,
	pub begin_popup_context_window: Option<extern fn(libc::c_int, *const libc::c_char, libc::c_int) -> libc::c_int>,
	pub begin_popupContext_void: Option<extern fn(*const libc::c_char, libc::c_int) -> libc::c_int>,
	pub end_popup: *mut extern fn () -> libc::c_void,
	pub close_current_popup: *mut extern fn () -> libc::c_void,
	pub value_int: Option<extern fn(*const libc::c_char, libc::c_int)>,
	pub value_u_int: Option<extern fn(*const libc::c_char, libc::c_uint)>,
	pub value_float: Option<extern fn(*const libc::c_char, libc::c_float, *const libc::c_char)>,
	pub color: Option<extern fn(*const libc::c_char, libc::c_uint)>,
	pub log_to_tty: Option<extern fn(libc::c_int)>,
	pub log_to_file: Option<extern fn(libc::c_int, *const libc::c_char)>,
	pub log_to_clipboard: Option<extern fn(libc::c_int)>,
	pub log_finish: *mut extern fn () -> libc::c_void,
	pub log_buttons: *mut extern fn () -> libc::c_void,
	pub is_item_hovered: *mut extern fn () -> libc::c_int,
	pub is_item_hovered_rect: *mut extern fn () -> libc::c_int,
	pub is_item_active: *mut extern fn () -> libc::c_int,
	pub is_item_visible: *mut extern fn () -> libc::c_int,
	pub is_any_item_hovered: *mut extern fn () -> libc::c_int,
	pub is_any_item_active: *mut extern fn () -> libc::c_int,
	pub get_item_rect_min: *mut extern fn () -> PDVec2,
	pub get_item_rect_max: *mut extern fn () -> PDVec2,
	pub get_item_rect_size: *mut extern fn () -> PDVec2,
	pub is_window_hovered: *mut extern fn () -> libc::c_int,
	pub is_window_focused: *mut extern fn () -> libc::c_int,
	pub is_root_window_focused: *mut extern fn () -> libc::c_int,
	pub is_root_window_or_any_child_focused: *mut extern fn () -> libc::c_int,
	pub is_rect_visible: Option<extern fn(PDVec2) -> libc::c_int>,
	pub is_pos_hovering_any_window: Option<extern fn(PDVec2) -> libc::c_int>,
	pub get_time: *mut extern fn () -> libc::c_float,
	pub get_frame_count: *mut extern fn () -> libc::c_int,
	pub get_style_col_name: Option<extern fn(libc::c_uint) -> *const libc::c_char>,
	pub calc_item_rect_closest_point: Option<extern fn(PDVec2, libc::c_int, libc::c_float) -> PDVec2>,
	pub calc_text_size: Option<extern fn(*const libc::c_char, *const libc::c_char, libc::c_int, libc::c_float) -> PDVec2>,
	pub calc_list_clipping: Option<extern fn(libc::c_int, libc::c_float, *mut libc::c_int, *mut libc::c_int)>,
	pub begin_childFrame: Option<extern fn(libc::c_uint, PDVec2) -> libc::c_int>,
	pub end_child_frame: *mut extern fn () -> libc::c_void,
	pub color_convert_rg_bto_hsv: Option<extern fn(libc::c_float, libc::c_float, libc::c_float, *mut libc::c_float, *mut libc::c_float, *mut libc::c_float)>,
	pub color_convert_hs_vto_rgb: Option<extern fn(libc::c_float, libc::c_float, libc::c_float, *mut libc::c_float, *mut libc::c_float, *mut libc::c_float)>,
	pub is_key_down: Option<extern fn(libc::c_int) -> libc::c_int>,
	pub is_key_pressed: Option<extern fn(libc::c_int, libc::c_int) -> libc::c_int>,
	pub is_key_released: Option<extern fn(libc::c_int) -> libc::c_int>,
	pub is_key_down_id: Option<extern fn(libc::c_uint, libc::c_int) -> libc::c_int>,
	pub is_mouse_down: Option<extern fn(libc::c_int) -> libc::c_int>,
	pub is_mouse_clicked: Option<extern fn(libc::c_int, libc::c_int) -> libc::c_int>,
	pub is_mouse_double_clicked: Option<extern fn(libc::c_int) -> libc::c_int>,
	pub is_mouse_released: Option<extern fn(libc::c_int) -> libc::c_int>,
	pub is_mouse_hovering_window: *mut extern fn () -> libc::c_int,
	pub is_mouse_hovering_any_window: *mut extern fn () -> libc::c_int,
	pub is_mouse_hovering_rect: Option<extern fn(PDVec2, PDVec2) -> libc::c_int>,
	pub is_mouse_dragging: Option<extern fn(libc::c_int, libc::c_float) -> libc::c_int>,
	pub get_mouse_pos: *mut extern fn () -> PDVec2,
	pub get_mouse_drag_delta: Option<extern fn(libc::c_int, libc::c_float) -> PDVec2>,
	pub reset_mouse_drag_delta: Option<extern fn(libc::c_int)>,
	pub get_mouse_cursor: *mut extern fn () -> libc::c_uint,
	pub set_mouse_cursor: Option<extern fn(libc::c_uint)>,
	pub fill_rect: Option<extern fn(PDRect, libc::c_uint)>,
	pub private_data: *mut libc::c_void,
}

/*
struct PDMouseWheelEvent
		(float) deltaX
		(float) deltaY
		(short) wheelDelta
		(short) rotation
		(int) wheelAxis
		(int) keyFlags
		(int) linesPerRotation
		(int) columnsPerRotation
*/
#[repr(C)]
pub struct PDMouseWheelEvent {
	pub deltaX: libc::c_float,
	pub deltaY: libc::c_float,
	pub wheelDelta: libc::c_short,
	pub rotation: libc::c_short,
	pub wheelAxis: libc::c_int,
	pub keyFlags: libc::c_int,
	pub linesPerRotation: libc::c_int,
	pub columnsPerRotation: libc::c_int,
}

/*
struct PDRect
		(float) x
		(float) y
		(float) width
		(float) height
*/
#[repr(C)]
pub struct PDRect {
	pub x: libc::c_float,
	pub y: libc::c_float,
	pub width: libc::c_float,
	pub height: libc::c_float,
}

/*
struct PDPluginBase
		(const char *) name
*/
#[repr(C)]
pub struct PDPluginBase {
	pub name: *const libc::c_char,
}

/*
enum PDUIInputTextFlags_ {
	PDUIInputTextFlags_CharsDecimal =	0x00000001 (1)
	PDUIInputTextFlags_CharsHexadecimal =	0x00000002 (2)
	PDUIInputTextFlags_CharsUppercase =	0x00000004 (4)
	PDUIInputTextFlags_CharsNoBlank =	0x00000008 (8)
	PDUIInputTextFlags_AutoSelectAll =	0x00000010 (16)
	PDUIInputTextFlags_EnterReturnsTrue =	0x00000020 (32)
	PDUIInputTextFlags_CallbackCompletion =	0x00000040 (64)
	PDUIInputTextFlags_CallbackHistory =	0x00000080 (128)
	PDUIInputTextFlags_CallbackAlways =	0x00000100 (256)
	PDUIInputTextFlags_CallbackCharFilter =	0x00000200 (512)
	PDUIInputTextFlags_AllowTabInput =	0x00000400 (1024)
	PDUIInputTextFlags_CtrlEnterForNewLine =	0x00000800 (2048)
	PDUIInputTextFlags_NoHorizontalScroll =	0x00001000 (4096)
	PDUIInputTextFlags_AlwaysInsertMode =	0x00002000 (8192)
}
*/
bitflags! {
	flags PDUIInputTextFlags_: libc::c_uint {
		const PDUIInputTextFlags_CharsDecimal =	1 as libc::c_uint,
		const PDUIInputTextFlags_CharsHexadecimal =	2 as libc::c_uint,
		const PDUIInputTextFlags_CharsUppercase =	4 as libc::c_uint,
		const PDUIInputTextFlags_CharsNoBlank =	8 as libc::c_uint,
		const PDUIInputTextFlags_AutoSelectAll =	16 as libc::c_uint,
		const PDUIInputTextFlags_EnterReturnsTrue =	32 as libc::c_uint,
		const PDUIInputTextFlags_CallbackCompletion =	64 as libc::c_uint,
		const PDUIInputTextFlags_CallbackHistory =	128 as libc::c_uint,
		const PDUIInputTextFlags_CallbackAlways =	256 as libc::c_uint,
		const PDUIInputTextFlags_CallbackCharFilter =	512 as libc::c_uint,
		const PDUIInputTextFlags_AllowTabInput =	1024 as libc::c_uint,
		const PDUIInputTextFlags_CtrlEnterForNewLine =	2048 as libc::c_uint,
		const PDUIInputTextFlags_NoHorizontalScroll =	4096 as libc::c_uint,
		const PDUIInputTextFlags_AlwaysInsertMode =	8192 as libc::c_uint,
	}
}


/*
enum PDUISelectableFlags_ {
	PDUISelectableFlags_DontClosePopups =	0x00000001 (1)
	PDUISelectableFlags_SpanAllColumns =	0x00000002 (2)
}
*/
bitflags! {
	flags PDUISelectableFlags_: libc::c_uint {
		const PDUISelectableFlags_DontClosePopups =	1 as libc::c_uint,
		const PDUISelectableFlags_SpanAllColumns =	2 as libc::c_uint,
	}
}


/*
enum PDUIWindowFlags_ {
	PDUIWindowFlags_NoTitleBar =	0x00000001 (1)
	PDUIWindowFlags_NoResize =	0x00000002 (2)
	PDUIWindowFlags_NoMove =	0x00000004 (4)
	PDUIWindowFlags_NoScrollbar =	0x00000008 (8)
	PDUIWindowFlags_NoScrollWithMouse =	0x00000010 (16)
	PDUIWindowFlags_NoCollapse =	0x00000020 (32)
	PDUIWindowFlags_AlwaysAutoResize =	0x00000040 (64)
	PDUIWindowFlags_ShowBorders =	0x00000080 (128)
	PDUIWindowFlags_NoSavedSettings =	0x00000100 (256)
	PDUIWindowFlags_NoInputs =	0x00000200 (512)
	PDUIWindowFlags_MenuBar =	0x00000400 (1024)
}
*/
bitflags! {
	flags PDUIWindowFlags_: libc::c_uint {
		const PDUIWindowFlags_NoTitleBar =	1 as libc::c_uint,
		const PDUIWindowFlags_NoResize =	2 as libc::c_uint,
		const PDUIWindowFlags_NoMove =	4 as libc::c_uint,
		const PDUIWindowFlags_NoScrollbar =	8 as libc::c_uint,
		const PDUIWindowFlags_NoScrollWithMouse =	16 as libc::c_uint,
		const PDUIWindowFlags_NoCollapse =	32 as libc::c_uint,
		const PDUIWindowFlags_AlwaysAutoResize =	64 as libc::c_uint,
		const PDUIWindowFlags_ShowBorders =	128 as libc::c_uint,
		const PDUIWindowFlags_NoSavedSettings =	256 as libc::c_uint,
		const PDUIWindowFlags_NoInputs =	512 as libc::c_uint,
		const PDUIWindowFlags_MenuBar =	1024 as libc::c_uint,
	}
}


/*
enum PDUIColorEditMode {
	PDUIColorEditMode_UserSelect =	0x-0000002 (-2)
	PDUIColorEditMode_UserSelectShowButton =	0x-0000001 (-1)
	PDUIColorEditMode_RGB =	0x00000000 (0)
	PDUIColorEditMode_HSV =	0x00000001 (1)
	PDUIColorEditMode_HEX =	0x00000002 (2)
}
*/
bitflags! {
	flags PDUIColorEditMode: libc::c_int {
		const PDUIColorEditMode_UserSelect =	-2 as libc::c_int,
		const PDUIColorEditMode_UserSelectShowButton =	-1 as libc::c_int,
		const PDUIColorEditMode_RGB =	0 as libc::c_int,
		const PDUIColorEditMode_HSV =	1 as libc::c_int,
		const PDUIColorEditMode_HEX =	2 as libc::c_int,
	}
}


/*
enum PDUISetCond_ {
	PDUISetCond_Always =	0x00000001 (1)
	PDUISetCond_Once =	0x00000002 (2)
	PDUISetCond_FirstUseEver =	0x00000004 (4)
	PDUISetCond_Appearing =	0x00000008 (8)
}
*/
bitflags! {
	flags PDUISetCond_: libc::c_uint {
		const PDUISetCond_Always =	1 as libc::c_uint,
		const PDUISetCond_Once =	2 as libc::c_uint,
		const PDUISetCond_FirstUseEver =	4 as libc::c_uint,
		const PDUISetCond_Appearing =	8 as libc::c_uint,
	}
}


/*
enum PDUIMouseCursor {
	PDUIMouseCursor_Arrow =	0x00000000 (0)
	PDUIMouseCursor_TextInput =	0x00000001 (1)
	PDUIMouseCursor_Move =	0x00000002 (2)
	PDUIMouseCursor_ResizeNS =	0x00000003 (3)
	PDUIMouseCursor_ResizeEW =	0x00000004 (4)
	PDUIMouseCursor_ResizeNESW =	0x00000005 (5)
	PDUIMouseCursor_ResizeNWSE =	0x00000006 (6)
}
*/
bitflags! {
	flags PDUIMouseCursor: libc::c_uint {
		const PDUIMouseCursor_Arrow =	0 as libc::c_uint,
		const PDUIMouseCursor_TextInput =	1 as libc::c_uint,
		const PDUIMouseCursor_Move =	2 as libc::c_uint,
		const PDUIMouseCursor_ResizeNS =	3 as libc::c_uint,
		const PDUIMouseCursor_ResizeEW =	4 as libc::c_uint,
		const PDUIMouseCursor_ResizeNESW =	5 as libc::c_uint,
		const PDUIMouseCursor_ResizeNWSE =	6 as libc::c_uint,
	}
}


/*
enum PDUICol {
	PDUICol_Text =	0x00000000 (0)
	PDUICol_TextDisabled =	0x00000001 (1)
	PDUICol_WindowBg =	0x00000002 (2)
	PDUICol_ChildWindowBg =	0x00000003 (3)
	PDUICol_Border =	0x00000004 (4)
	PDUICol_BorderShadow =	0x00000005 (5)
	PDUICol_FrameBg =	0x00000006 (6)
	PDUICol_FrameBgHovered =	0x00000007 (7)
	PDUICol_FrameBgActive =	0x00000008 (8)
	PDUICol_TitleBg =	0x00000009 (9)
	PDUICol_TitleBgCollapsed =	0x0000000A (10)
	PDUICol_TitleBgActive =	0x0000000B (11)
	PDUICol_MenuBarBg =	0x0000000C (12)
	PDUICol_ScrollbarBg =	0x0000000D (13)
	PDUICol_ScrollbarGrab =	0x0000000E (14)
	PDUICol_ScrollbarGrabHovered =	0x0000000F (15)
	PDUICol_ScrollbarGrabActive =	0x00000010 (16)
	PDUICol_ComboBg =	0x00000011 (17)
	PDUICol_CheckMark =	0x00000012 (18)
	PDUICol_SliderGrab =	0x00000013 (19)
	PDUICol_SliderGrabActive =	0x00000014 (20)
	PDUICol_Button =	0x00000015 (21)
	PDUICol_ButtonHovered =	0x00000016 (22)
	PDUICol_ButtonActive =	0x00000017 (23)
	PDUICol_Header =	0x00000018 (24)
	PDUICol_HeaderHovered =	0x00000019 (25)
	PDUICol_HeaderActive =	0x0000001A (26)
	PDUICol_Column =	0x0000001B (27)
	PDUICol_ColumnHovered =	0x0000001C (28)
	PDUICol_ColumnActive =	0x0000001D (29)
	PDUICol_ResizeGrip =	0x0000001E (30)
	PDUICol_ResizeGripHovered =	0x0000001F (31)
	PDUICol_ResizeGripActive =	0x00000020 (32)
	PDUICol_CloseButton =	0x00000021 (33)
	PDUICol_CloseButtonHovered =	0x00000022 (34)
	PDUICol_CloseButtonActive =	0x00000023 (35)
	PDUICol_PlotLines =	0x00000024 (36)
	PDUICol_PlotLinesHovered =	0x00000025 (37)
	PDUICol_PlotHistogram =	0x00000026 (38)
	PDUICol_PlotHistogramHovered =	0x00000027 (39)
	PDUICol_TextSelectedBg =	0x00000028 (40)
	PDUICol_TooltipBg =	0x00000029 (41)
	PDUICol_ModalWindowDarkening =	0x0000002A (42)
	PDUICol_COUNT =	0x0000002B (43)
}
*/
bitflags! {
	flags PDUICol: libc::c_uint {
		const PDUICol_Text =	0 as libc::c_uint,
		const PDUICol_TextDisabled =	1 as libc::c_uint,
		const PDUICol_WindowBg =	2 as libc::c_uint,
		const PDUICol_ChildWindowBg =	3 as libc::c_uint,
		const PDUICol_Border =	4 as libc::c_uint,
		const PDUICol_BorderShadow =	5 as libc::c_uint,
		const PDUICol_FrameBg =	6 as libc::c_uint,
		const PDUICol_FrameBgHovered =	7 as libc::c_uint,
		const PDUICol_FrameBgActive =	8 as libc::c_uint,
		const PDUICol_TitleBg =	9 as libc::c_uint,
		const PDUICol_TitleBgCollapsed =	10 as libc::c_uint,
		const PDUICol_TitleBgActive =	11 as libc::c_uint,
		const PDUICol_MenuBarBg =	12 as libc::c_uint,
		const PDUICol_ScrollbarBg =	13 as libc::c_uint,
		const PDUICol_ScrollbarGrab =	14 as libc::c_uint,
		const PDUICol_ScrollbarGrabHovered =	15 as libc::c_uint,
		const PDUICol_ScrollbarGrabActive =	16 as libc::c_uint,
		const PDUICol_ComboBg =	17 as libc::c_uint,
		const PDUICol_CheckMark =	18 as libc::c_uint,
		const PDUICol_SliderGrab =	19 as libc::c_uint,
		const PDUICol_SliderGrabActive =	20 as libc::c_uint,
		const PDUICol_Button =	21 as libc::c_uint,
		const PDUICol_ButtonHovered =	22 as libc::c_uint,
		const PDUICol_ButtonActive =	23 as libc::c_uint,
		const PDUICol_Header =	24 as libc::c_uint,
		const PDUICol_HeaderHovered =	25 as libc::c_uint,
		const PDUICol_HeaderActive =	26 as libc::c_uint,
		const PDUICol_Column =	27 as libc::c_uint,
		const PDUICol_ColumnHovered =	28 as libc::c_uint,
		const PDUICol_ColumnActive =	29 as libc::c_uint,
		const PDUICol_ResizeGrip =	30 as libc::c_uint,
		const PDUICol_ResizeGripHovered =	31 as libc::c_uint,
		const PDUICol_ResizeGripActive =	32 as libc::c_uint,
		const PDUICol_CloseButton =	33 as libc::c_uint,
		const PDUICol_CloseButtonHovered =	34 as libc::c_uint,
		const PDUICol_CloseButtonActive =	35 as libc::c_uint,
		const PDUICol_PlotLines =	36 as libc::c_uint,
		const PDUICol_PlotLinesHovered =	37 as libc::c_uint,
		const PDUICol_PlotHistogram =	38 as libc::c_uint,
		const PDUICol_PlotHistogramHovered =	39 as libc::c_uint,
		const PDUICol_TextSelectedBg =	40 as libc::c_uint,
		const PDUICol_TooltipBg =	41 as libc::c_uint,
		const PDUICol_ModalWindowDarkening =	42 as libc::c_uint,
		const PDUICol_COUNT =	43 as libc::c_uint,
	}
}


/*
enum PDUIStyleVar {
	PDUIStyleVar_Invalid =	0x00000000 (0)
	PDUIStyleVar_Alpha =	0x00000001 (1)
	PDUIStyleVar_WindowPadding =	0x00000002 (2)
	PDUIStyleVar_WindowRounding =	0x00000003 (3)
	PDUIStyleVar_FramePadding =	0x00000004 (4)
	PDUIStyleVar_FrameRounding =	0x00000005 (5)
	PDUIStyleVar_ItemSpacing =	0x00000006 (6)
	PDUIStyleVar_ItemInnerSpacing =	0x00000007 (7)
	PDUIStyleVar_TreeNodeSpacing =	0x00000008 (8)
	PDUIStyleVar_Count =	0x00000009 (9)
}
*/
bitflags! {
	flags PDUIStyleVar: libc::c_uint {
		const PDUIStyleVar_Invalid =	0 as libc::c_uint,
		const PDUIStyleVar_Alpha =	1 as libc::c_uint,
		const PDUIStyleVar_WindowPadding =	2 as libc::c_uint,
		const PDUIStyleVar_WindowRounding =	3 as libc::c_uint,
		const PDUIStyleVar_FramePadding =	4 as libc::c_uint,
		const PDUIStyleVar_FrameRounding =	5 as libc::c_uint,
		const PDUIStyleVar_ItemSpacing =	6 as libc::c_uint,
		const PDUIStyleVar_ItemInnerSpacing =	7 as libc::c_uint,
		const PDUIStyleVar_TreeNodeSpacing =	8 as libc::c_uint,
		const PDUIStyleVar_Count =	9 as libc::c_uint,
	}
}


/* _PDUIUI_H_ # */

/* _PDCOMMON_H_ # */

/* PD_EXPORT # */

/* PD_KEYS_H_ # */

/* PDKEY_UNKNOWN 0 # */
pub const PDKEY_UNKNOWN: i32 = 0;

/* PDKEY_SPACE 32 # */
pub const PDKEY_SPACE: i32 = 32;

/* PDKEY_APOSTROPHE 39 # */
pub const PDKEY_APOSTROPHE: i32 = 39;

/* PDKEY_COMMA 44 # */
pub const PDKEY_COMMA: i32 = 44;

/* PDKEY_MINUS 45 # */
pub const PDKEY_MINUS: i32 = 45;

/* PDKEY_PERIOD 46 # */
pub const PDKEY_PERIOD: i32 = 46;

/* PDKEY_SLASH 47 # */
pub const PDKEY_SLASH: i32 = 47;

/* PDKEY_0 48 # */
pub const PDKEY_0: i32 = 48;

/* PDKEY_1 49 # */
pub const PDKEY_1: i32 = 49;

/* PDKEY_2 50 # */
pub const PDKEY_2: i32 = 50;

/* PDKEY_3 51 # */
pub const PDKEY_3: i32 = 51;

/* PDKEY_4 52 # */
pub const PDKEY_4: i32 = 52;

/* PDKEY_5 53 # */
pub const PDKEY_5: i32 = 53;

/* PDKEY_6 54 # */
pub const PDKEY_6: i32 = 54;

/* PDKEY_7 55 # */
pub const PDKEY_7: i32 = 55;

/* PDKEY_8 56 # */
pub const PDKEY_8: i32 = 56;

/* PDKEY_9 57 # */
pub const PDKEY_9: i32 = 57;

/* PDKEY_SEMICOLON 59 # */
pub const PDKEY_SEMICOLON: i32 = 59;

/* PDKEY_EQUAL 61 # */
pub const PDKEY_EQUAL: i32 = 61;

/* PDKEY_A 65 # */
pub const PDKEY_A: i32 = 65;

/* PDKEY_B 66 # */
pub const PDKEY_B: i32 = 66;

/* PDKEY_C 67 # */
pub const PDKEY_C: i32 = 67;

/* PDKEY_D 68 # */
pub const PDKEY_D: i32 = 68;

/* PDKEY_E 69 # */
pub const PDKEY_E: i32 = 69;

/* PDKEY_F 70 # */
pub const PDKEY_F: i32 = 70;

/* PDKEY_G 71 # */
pub const PDKEY_G: i32 = 71;

/* PDKEY_H 72 # */
pub const PDKEY_H: i32 = 72;

/* PDKEY_I 73 # */
pub const PDKEY_I: i32 = 73;

/* PDKEY_J 74 # */
pub const PDKEY_J: i32 = 74;

/* PDKEY_K 75 # */
pub const PDKEY_K: i32 = 75;

/* PDKEY_L 76 # */
pub const PDKEY_L: i32 = 76;

/* PDKEY_M 77 # */
pub const PDKEY_M: i32 = 77;

/* PDKEY_N 78 # */
pub const PDKEY_N: i32 = 78;

/* PDKEY_O 79 # */
pub const PDKEY_O: i32 = 79;

/* PDKEY_P 80 # */
pub const PDKEY_P: i32 = 80;

/* PDKEY_Q 81 # */
pub const PDKEY_Q: i32 = 81;

/* PDKEY_R 82 # */
pub const PDKEY_R: i32 = 82;

/* PDKEY_S 83 # */
pub const PDKEY_S: i32 = 83;

/* PDKEY_T 84 # */
pub const PDKEY_T: i32 = 84;

/* PDKEY_U 85 # */
pub const PDKEY_U: i32 = 85;

/* PDKEY_V 86 # */
pub const PDKEY_V: i32 = 86;

/* PDKEY_W 87 # */
pub const PDKEY_W: i32 = 87;

/* PDKEY_X 88 # */
pub const PDKEY_X: i32 = 88;

/* PDKEY_Y 89 # */
pub const PDKEY_Y: i32 = 89;

/* PDKEY_Z 90 # */
pub const PDKEY_Z: i32 = 90;

/* PDKEY_LEFT_BRACKET 91 # */
pub const PDKEY_LEFT_BRACKET: i32 = 91;

/* PDKEY_BACKSLASH 92 # */
pub const PDKEY_BACKSLASH: i32 = 92;

/* PDKEY_RIGHT_BRACKET 93 # */
pub const PDKEY_RIGHT_BRACKET: i32 = 93;

/* PDKEY_GRAVE_ACCENT 96 // Function keys */
pub const PDKEY_GRAVE_ACCENT: i32 = 96;

/* PDKEY_ESCAPE 256 # */
pub const PDKEY_ESCAPE: i32 = 256;

/* PDKEY_ENTER 257 # */
pub const PDKEY_ENTER: i32 = 257;

/* PDKEY_TAB 258 # */
pub const PDKEY_TAB: i32 = 258;

/* PDKEY_BACKSPACE 259 # */
pub const PDKEY_BACKSPACE: i32 = 259;

/* PDKEY_INSERT 260 # */
pub const PDKEY_INSERT: i32 = 260;

/* PDKEY_DELETE 261 # */
pub const PDKEY_DELETE: i32 = 261;

/* PDKEY_RIGHT 262 # */
pub const PDKEY_RIGHT: i32 = 262;

/* PDKEY_LEFT 263 # */
pub const PDKEY_LEFT: i32 = 263;

/* PDKEY_DOWN 264 # */
pub const PDKEY_DOWN: i32 = 264;

/* PDKEY_UP 265 # */
pub const PDKEY_UP: i32 = 265;

/* PDKEY_PAGE_UP 266 # */
pub const PDKEY_PAGE_UP: i32 = 266;

/* PDKEY_PAGE_DOWN 267 # */
pub const PDKEY_PAGE_DOWN: i32 = 267;

/* PDKEY_HOME 268 # */
pub const PDKEY_HOME: i32 = 268;

/* PDKEY_END 269 # */
pub const PDKEY_END: i32 = 269;

/* PDKEY_CAPS_LOCK 280 # */
pub const PDKEY_CAPS_LOCK: i32 = 280;

/* PDKEY_SCROLL_LOCK 281 # */
pub const PDKEY_SCROLL_LOCK: i32 = 281;

/* PDKEY_NUM_LOCK 282 # */
pub const PDKEY_NUM_LOCK: i32 = 282;

/* PDKEY_PRINT_SCREEN 283 # */
pub const PDKEY_PRINT_SCREEN: i32 = 283;

/* PDKEY_PAUSE 284 # */
pub const PDKEY_PAUSE: i32 = 284;

/* PDKEY_F1 290 # */
pub const PDKEY_F1: i32 = 290;

/* PDKEY_F2 291 # */
pub const PDKEY_F2: i32 = 291;

/* PDKEY_F3 292 # */
pub const PDKEY_F3: i32 = 292;

/* PDKEY_F4 293 # */
pub const PDKEY_F4: i32 = 293;

/* PDKEY_F5 294 # */
pub const PDKEY_F5: i32 = 294;

/* PDKEY_F6 295 # */
pub const PDKEY_F6: i32 = 295;

/* PDKEY_F7 296 # */
pub const PDKEY_F7: i32 = 296;

/* PDKEY_F8 297 # */
pub const PDKEY_F8: i32 = 297;

/* PDKEY_F9 298 # */
pub const PDKEY_F9: i32 = 298;

/* PDKEY_F10 299 # */
pub const PDKEY_F10: i32 = 299;

/* PDKEY_F11 300 # */
pub const PDKEY_F11: i32 = 300;

/* PDKEY_F12 301 # */
pub const PDKEY_F12: i32 = 301;

/* PDKEY_F13 302 # */
pub const PDKEY_F13: i32 = 302;

/* PDKEY_F14 303 # */
pub const PDKEY_F14: i32 = 303;

/* PDKEY_F15 304 # */
pub const PDKEY_F15: i32 = 304;

/* PDKEY_F16 305 # */
pub const PDKEY_F16: i32 = 305;

/* PDKEY_F17 306 # */
pub const PDKEY_F17: i32 = 306;

/* PDKEY_F18 307 # */
pub const PDKEY_F18: i32 = 307;

/* PDKEY_F19 308 # */
pub const PDKEY_F19: i32 = 308;

/* PDKEY_F20 309 # */
pub const PDKEY_F20: i32 = 309;

/* PDKEY_F21 310 # */
pub const PDKEY_F21: i32 = 310;

/* PDKEY_F22 311 # */
pub const PDKEY_F22: i32 = 311;

/* PDKEY_F23 312 # */
pub const PDKEY_F23: i32 = 312;

/* PDKEY_F24 313 # */
pub const PDKEY_F24: i32 = 313;

/* PDKEY_F25 314 # */
pub const PDKEY_F25: i32 = 314;

/* PDKEY_KP_0 320 # */
pub const PDKEY_KP_0: i32 = 320;

/* PDKEY_KP_1 321 # */
pub const PDKEY_KP_1: i32 = 321;

/* PDKEY_KP_2 322 # */
pub const PDKEY_KP_2: i32 = 322;

/* PDKEY_KP_3 323 # */
pub const PDKEY_KP_3: i32 = 323;

/* PDKEY_KP_4 324 # */
pub const PDKEY_KP_4: i32 = 324;

/* PDKEY_KP_5 325 # */
pub const PDKEY_KP_5: i32 = 325;

/* PDKEY_KP_6 326 # */
pub const PDKEY_KP_6: i32 = 326;

/* PDKEY_KP_7 327 # */
pub const PDKEY_KP_7: i32 = 327;

/* PDKEY_KP_8 328 # */
pub const PDKEY_KP_8: i32 = 328;

/* PDKEY_KP_9 329 # */
pub const PDKEY_KP_9: i32 = 329;

/* PDKEY_KP_DECIMAL 330 # */
pub const PDKEY_KP_DECIMAL: i32 = 330;

/* PDKEY_KP_DIVIDE 331 # */
pub const PDKEY_KP_DIVIDE: i32 = 331;

/* PDKEY_KP_MULTIPLY 332 # */
pub const PDKEY_KP_MULTIPLY: i32 = 332;

/* PDKEY_KP_SUBTRACT 333 # */
pub const PDKEY_KP_SUBTRACT: i32 = 333;

/* PDKEY_KP_ADD 334 # */
pub const PDKEY_KP_ADD: i32 = 334;

/* PDKEY_KP_ENTER 335 # */
pub const PDKEY_KP_ENTER: i32 = 335;

/* PDKEY_KP_EQUAL 336 # */
pub const PDKEY_KP_EQUAL: i32 = 336;

/* PDKEY_LEFT_SHIFT 340 # */
pub const PDKEY_LEFT_SHIFT: i32 = 340;

/* PDKEY_LEFT_CONTROL 341 # */
pub const PDKEY_LEFT_CONTROL: i32 = 341;

/* PDKEY_LEFT_ALT 342 # */
pub const PDKEY_LEFT_ALT: i32 = 342;

/* PDKEY_LEFT_SUPER 343 # */
pub const PDKEY_LEFT_SUPER: i32 = 343;

/* PDKEY_RIGHT_SHIFT 344 # */
pub const PDKEY_RIGHT_SHIFT: i32 = 344;

/* PDKEY_RIGHT_CONTROL 345 # */
pub const PDKEY_RIGHT_CONTROL: i32 = 345;

/* PDKEY_RIGHT_ALT 346 # */
pub const PDKEY_RIGHT_ALT: i32 = 346;

/* PDKEY_RIGHT_SUPER 347 # */
pub const PDKEY_RIGHT_SUPER: i32 = 347;

/* PDKEY_MENU 348 # */
pub const PDKEY_MENU: i32 = 348;

/* PDKEY_MAX 349 # */
pub const PDKEY_MAX: i32 = 349;

/* PDKEY_SHIFT 1 # */
pub const PDKEY_SHIFT: i32 = 1;

/* PDKEY_ALT 2 # */
pub const PDKEY_ALT: i32 = 2;

/* PDKEY_CTRL 4 # */
pub const PDKEY_CTRL: i32 = 4;

/* PDKEY_SUPER 8 # */
pub const PDKEY_SUPER: i32 = 8;

/* PDWHEEL_AXIS_VERTICAL 0 # */
pub const PDWHEEL_AXIS_VERTICAL: i32 = 0;

/* PDWHEEL_AXIS_HORIZONTAL 1 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
pub const PDWHEEL_AXIS_HORIZONTAL: i32 = 1;

/* __CLANG_STDINT_H /* If we're hosted, fall back to the system's stdint.h, which might have
 * additional definitions.
 */ */

/* __int_least64_t int64_t # */

/* __uint_least64_t uint64_t # */

/* __int_least32_t int64_t # */

/* __uint_least32_t uint64_t # */

/* __int_least16_t int64_t # */

/* __uint_least16_t uint64_t # */

/* __int_least8_t int64_t # */

/* __uint_least8_t uint64_t # */

/* __uint32_t_defined typedef */

/* __int_least32_t int32_t # */

/* __uint_least32_t uint32_t # */

/* __int_least16_t int32_t # */

/* __uint_least16_t uint32_t # */

/* __int_least8_t int32_t # */

/* __uint_least8_t uint32_t # */

/* __int_least16_t int16_t # */

/* __uint_least16_t uint16_t # */

/* __int_least8_t int16_t # */

/* __uint_least8_t uint16_t # */

/* __int_least8_t int8_t # */

/* __uint_least8_t uint8_t # */

/* __int8_t_defined # */

/* __stdint_join3 ( a , b , c ) a ## b ## c # */

/* __intn_t ( n ) __stdint_join3 ( int , n , _t ) # */

/* __uintn_t ( n ) __stdint_join3 ( uint , n , _t ) # */

/* __intptr_t_defined # */

/* _INTPTR_T # */

/* _UINTPTR_T # */

/* __int_c_join ( a , b ) a ## b # */

/* __int_c ( v , suffix ) __int_c_join ( v , suffix ) # */

/* __uint_c ( v , suffix ) __int_c_join ( v ## U , suffix ) # */

/* __int64_c_suffix __INT64_C_SUFFIX__ # */

/* __int32_c_suffix __INT64_C_SUFFIX__ # */

/* __int16_c_suffix __INT64_C_SUFFIX__ # */

/* __int8_c_suffix __INT64_C_SUFFIX__ # */

/* INT64_C ( v ) __int_c ( v , __int64_c_suffix ) # */

/* UINT64_C ( v ) __uint_c ( v , __int64_c_suffix ) # */

/* __int32_c_suffix __INT32_C_SUFFIX__ # */

/* __int16_c_suffix __INT32_C_SUFFIX__ # */

/* __int8_c_suffix __INT32_C_SUFFIX__ # */

/* INT32_C ( v ) __int_c ( v , __int32_c_suffix ) # */

/* UINT32_C ( v ) __uint_c ( v , __int32_c_suffix ) # */

/* __int16_c_suffix __INT16_C_SUFFIX__ # */

/* __int8_c_suffix __INT16_C_SUFFIX__ # */

/* INT16_C ( v ) __int_c ( v , __int16_c_suffix ) # */

/* UINT16_C ( v ) __uint_c ( v , __int16_c_suffix ) # */

/* __int8_c_suffix __INT8_C_SUFFIX__ # */

/* INT8_C ( v ) __int_c ( v , __int8_c_suffix ) # */

/* UINT8_C ( v ) __uint_c ( v , __int8_c_suffix ) # */

/* INT64_MAX INT64_C ( 9223372036854775807 ) # */

/* INT64_MIN ( - INT64_C ( 9223372036854775807 ) - 1 ) # */

/* UINT64_MAX UINT64_C ( 18446744073709551615 ) # */

/* __INT_LEAST64_MIN INT64_MIN # */

/* __INT_LEAST64_MAX INT64_MAX # */

/* __UINT_LEAST64_MAX UINT64_MAX # */

/* __INT_LEAST32_MIN INT64_MIN # */

/* __INT_LEAST32_MAX INT64_MAX # */

/* __UINT_LEAST32_MAX UINT64_MAX # */

/* __INT_LEAST16_MIN INT64_MIN # */

/* __INT_LEAST16_MAX INT64_MAX # */

/* __UINT_LEAST16_MAX UINT64_MAX # */

/* __INT_LEAST8_MIN INT64_MIN # */

/* __INT_LEAST8_MAX INT64_MAX # */

/* __UINT_LEAST8_MAX UINT64_MAX # */

/* INT_LEAST64_MIN __INT_LEAST64_MIN # */

/* INT_LEAST64_MAX __INT_LEAST64_MAX # */

/* UINT_LEAST64_MAX __UINT_LEAST64_MAX # */

/* INT_FAST64_MIN __INT_LEAST64_MIN # */

/* INT_FAST64_MAX __INT_LEAST64_MAX # */

/* UINT_FAST64_MAX __UINT_LEAST64_MAX # */

/* INT32_MAX INT32_C ( 2147483647 ) # */

/* INT32_MIN ( - INT32_C ( 2147483647 ) - 1 ) # */

/* UINT32_MAX UINT32_C ( 4294967295 ) # */

/* __INT_LEAST32_MIN INT32_MIN # */

/* __INT_LEAST32_MAX INT32_MAX # */

/* __UINT_LEAST32_MAX UINT32_MAX # */

/* __INT_LEAST16_MIN INT32_MIN # */

/* __INT_LEAST16_MAX INT32_MAX # */

/* __UINT_LEAST16_MAX UINT32_MAX # */

/* __INT_LEAST8_MIN INT32_MIN # */

/* __INT_LEAST8_MAX INT32_MAX # */

/* __UINT_LEAST8_MAX UINT32_MAX # */

/* INT_LEAST32_MIN __INT_LEAST32_MIN # */

/* INT_LEAST32_MAX __INT_LEAST32_MAX # */

/* UINT_LEAST32_MAX __UINT_LEAST32_MAX # */

/* INT_FAST32_MIN __INT_LEAST32_MIN # */

/* INT_FAST32_MAX __INT_LEAST32_MAX # */

/* UINT_FAST32_MAX __UINT_LEAST32_MAX # */

/* INT16_MAX INT16_C ( 32767 ) # */

/* INT16_MIN ( - INT16_C ( 32767 ) - 1 ) # */

/* UINT16_MAX UINT16_C ( 65535 ) # */

/* __INT_LEAST16_MIN INT16_MIN # */

/* __INT_LEAST16_MAX INT16_MAX # */

/* __UINT_LEAST16_MAX UINT16_MAX # */

/* __INT_LEAST8_MIN INT16_MIN # */

/* __INT_LEAST8_MAX INT16_MAX # */

/* __UINT_LEAST8_MAX UINT16_MAX # */

/* INT_LEAST16_MIN __INT_LEAST16_MIN # */

/* INT_LEAST16_MAX __INT_LEAST16_MAX # */

/* UINT_LEAST16_MAX __UINT_LEAST16_MAX # */

/* INT_FAST16_MIN __INT_LEAST16_MIN # */

/* INT_FAST16_MAX __INT_LEAST16_MAX # */

/* UINT_FAST16_MAX __UINT_LEAST16_MAX # */

/* INT8_MAX INT8_C ( 127 ) # */

/* INT8_MIN ( - INT8_C ( 127 ) - 1 ) # */

/* UINT8_MAX UINT8_C ( 255 ) # */

/* __INT_LEAST8_MIN INT8_MIN # */

/* __INT_LEAST8_MAX INT8_MAX # */

/* __UINT_LEAST8_MAX UINT8_MAX # */

/* INT_LEAST8_MIN __INT_LEAST8_MIN # */

/* INT_LEAST8_MAX __INT_LEAST8_MAX # */

/* UINT_LEAST8_MAX __UINT_LEAST8_MAX # */

/* INT_FAST8_MIN __INT_LEAST8_MIN # */

/* INT_FAST8_MAX __INT_LEAST8_MAX # */

/* UINT_FAST8_MAX __UINT_LEAST8_MAX # */

/* __INTN_MIN ( n ) __stdint_join3 ( INT , n , _MIN ) # */

/* __INTN_MAX ( n ) __stdint_join3 ( INT , n , _MAX ) # */

/* __UINTN_MAX ( n ) __stdint_join3 ( UINT , n , _MAX ) # */

/* __INTN_C ( n , v ) __stdint_join3 ( INT , n , _C ( v ) ) # */

/* __UINTN_C ( n , v ) __stdint_join3 ( UINT , n , _C ( v ) ) /* C99 7.18.2.4 Limits of integer types capable of holding object pointers. */ */

/* INTPTR_MIN __INTN_MIN ( __INTPTR_WIDTH__ ) # */

/* INTPTR_MAX __INTN_MAX ( __INTPTR_WIDTH__ ) # */

/* UINTPTR_MAX __UINTN_MAX ( __INTPTR_WIDTH__ ) # */

/* PTRDIFF_MIN __INTN_MIN ( __PTRDIFF_WIDTH__ ) # */

/* PTRDIFF_MAX __INTN_MAX ( __PTRDIFF_WIDTH__ ) # */

/* SIZE_MAX __UINTN_MAX ( __SIZE_WIDTH__ ) /* ISO9899:2011 7.20 (C11 Annex K): Define RSIZE_MAX if __STDC_WANT_LIB_EXT1__
 * is enabled. */ */

/* INTMAX_MIN __INTN_MIN ( __INTMAX_WIDTH__ ) # */

/* INTMAX_MAX __INTN_MAX ( __INTMAX_WIDTH__ ) # */

/* UINTMAX_MAX __UINTN_MAX ( __INTMAX_WIDTH__ ) /* C99 7.18.3 Limits of other integer types. */ */

/* SIG_ATOMIC_MIN __INTN_MIN ( __SIG_ATOMIC_WIDTH__ ) # */

/* SIG_ATOMIC_MAX __INTN_MAX ( __SIG_ATOMIC_WIDTH__ ) # */

/* WINT_MIN __INTN_MIN ( __WINT_WIDTH__ ) # */

/* WINT_MAX __INTN_MAX ( __WINT_WIDTH__ ) # */

/* WCHAR_MAX __WCHAR_MAX__ # */

/* WCHAR_MIN __INTN_MIN ( __WCHAR_WIDTH__ ) # */

/* INTMAX_C ( v ) __INTN_C ( __INTMAX_WIDTH__ , v ) # */

/* UINTMAX_C ( v ) __UINTN_C ( __INTMAX_WIDTH__ , v ) # */

/* PDUI_COLOR ( r , g , b , a ) ( ( ( uint32_t ) a << 24 ) | ( ( uint32_t ) g << 16 ) | ( ( uint32_t ) b << 8 ) | ( r ) ) /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/* PDUI_collumns ( uiFuncs , count , id , border ) uiFuncs -> columns ( count , id , border ) # */

/* PDUI_next_column ( uiFuncs ) uiFuncs -> next_column ( ) # */

/* PDUI_text ( uiFuncs , format , ... ) uiFuncs -> text ( format , __VA_ARGS__ ) # */

/* PDUI_text_wrapped ( uiFuncs , format , ... ) uiFuncs -> text_wrapped ( format , __VA_ARGS__ ) # */

/* PDUI_button ( uiFuncs , label ) uiFuncs -> button ( label ) //#define PDUI_buttonSmall(uiFuncs, label) uiFuncs->buttonSmall(label) */

/* PDUI_buttonSize ( uiFuncs , label , w , h ) uiFuncs -> button ( label , w , h ) # */

/* PDUI_SCSendCommand ( funcs , msg , p0 , p1 ) funcs -> send_command ( funcs -> private_data , msg , p0 , p1 ) # */

/* PDUI_SCDraw ( funcs ) funcs -> update ( funcs -> private_data ) # */

/* PDUI_SCUpdate ( funcs ) funcs -> draw ( funcs -> private_data ) # */

/* PDUI_set_title ( funcs , title ) funcs -> set_title ( funcs -> private_data , title ) /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

