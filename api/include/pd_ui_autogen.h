// !!! Autogenerated with codespawn (0.2.0) - do not modify. !!!

enum PDUIWindowFlags_ {
    PDUIWindowFlags_NoTitleBar = 1,
    PDUIWindowFlags_NoResize = 2,
    PDUIWindowFlags_NoMove = 4,
    PDUIWindowFlags_NoScrollbar = 8,
    PDUIWindowFlags_NoScrollWithMouse = 16,
    PDUIWindowFlags_NoCollapse = 32,
    PDUIWindowFlags_AlwaysAutoResize = 64,
    PDUIWindowFlags_ShowBorders = 128,
    PDUIWindowFlags_NoSavedSettings  = 256,
    PDUIWindowFlags_NoInputs  = 512,
    PDUIWindowFlags_MenuBar = 1024,
};


struct PDVec2 {
    float x;
    float y;
};


struct PDVec4 {
    float x;
    float y;
    float z;
    float w;
};


struct PDRect {
    float x;
    float y;
    float width;
    float height;
};


struct PDUIInputTextCallbackData {
    PDUIInputTextFlags event_flag;
    PDUIInputTextFlags flags;
    void* user_data;
    uint16_t event_char;
    uint16_t event_key;
    char* buf;
    size_t buf_size;
    bool buf_dirty;
    int cursor_pos;
    int selection_start;
    int selection_end;
    void (*delete_chars)(struct PDUIInputTextCallbackData* data, int pos, int byteCount);
    void (*insert_chars)(struct PDUIInputTextCallbackData* data, int pos, const char* text, const char* textEnd);
};


struct PDUI {
    void* private_data;
    void (*set_title)(void* private_data, const char* title);
    PDVec2 (*get_window_size)();
    PDVec2 (*get_window_pos)();
    void (*begin_child)(const char* stringId, PDVec2 size, int border, int extraFlags);
    void (*end_child)();
    float (*get_scroll_y)();
    float (*get_scroll_max_y)();
    void (*set_scroll_y)(float scrollY);
    void (*set_scroll_here)(float centerYratio);
    void (*set_scroll_from_pos_y)(float posY, float centerYratio);
    void (*set_keyboard_focus_here)(int offset);
    void (*push_font)(PDUIFont font);
    void (*pop_font)();
    void (*push_style_color)(PDUICol idx, PDColor col);
    void (*pop_style_color)(int count);
    void (*push_style_var)(PDUIStyleVar idx, float val);
    void (*push_style_var_vec)(PDUIStyleVar idx, PDVec2 val);
    void (*pop_style_var)(int count);
    float (*get_font_size)();
    void (*push_item_width)(float item_width);
    void (*pop_item_width)();
    float (*calc_item_width)();
    void (*push_allow_keyboard_focus)(int v);
    void (*pop_allow_keyboard_focus)();
    void (*push_text_wrap_pos)(float wrapPosX);
    void (*pop_text_wrap_pos)();
    void (*push_button_repeat)(int repeat);
    void (*pop_button_repeat)();
    void (*begin_group)();
    void (*end_group)();
    void (*separator)();
    void (*same_line)(int columnX, int spacingW);
    void (*spacing)();
    void (*dummy)(PDVec2 size);
    void (*indent)();
    void (*un_indent)();
    void (*columns)(int count, const char* id, int border);
    void (*next_column)();
    int (*get_column_index)();
    float (*get_column_offset)(int column_index);
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
    void (*push_id_str)(const char* strId);
    void (*push_id_str_range)(const char* strBegin, const char* strEnd);
    void (*push_id_ptr)(const void* ptrId);
    void (*push_id_int)(const int intId);
    void (*pop_id)();
    PDID (*get_id_str)(const char* strId);
    PDID (*get_id_str_range)(const char* strBegin, const char* strEnd);
    PDID (*get_id_ptr)(const void* ptrId);
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
    int (*button)(const char* label, const PDVec2 size);
    int (*small_button)(const char* label);
    int (*invisible_button)(const char* strId, const PDVec2 size);
    void (*image)(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, const PDColor tintColor, const PDColor borderColor);
    int (*image_button)(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, int framePadding, const PDColor bgColor, const PDColor tintCol);
    int (*collapsing_header)(const char* label, const char* strId, int displayFrame, int defaultOpen);
    int (*checkbox)(const char* label, int* v);
    int (*checkbox_flags)(const char* label, unsigned int* flags, unsigned int flagsValue);
    int (*radio_button_bool)(const char* label, int active);
    int (*radio_button)(const char* label, int* v, int v_button);
    int (*combo)(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems);
    int (*combo2)(const char* label, int* currentItem, const char* itemsSeparatedByZeros, int heightInItems);
    int (*combo3)(const char* label, int* currentItem, bool (*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems);
    int (*color_button)(const PDColor col, int smallHeight, int outlineBorder);
    int (*color_edit3)(const char* label, float col[3]);
    int (*color_edit4)(const char* label, float col[4], int showAlpha);
    void (*color_edit_mode)(PDUIColorEditMode mode);
    void (*plot_lines)(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride);
    void (*plot_lines2)(const char* label, float (*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize);
    void (*plot_histogram)(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride);
    void (*plot_histogram2)(const char* label, float (*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize);
    PDUISCInterface* (*sc_input_text)(const char* label, float xSize, float ySize);
    int (*slider_float)(const char* label, float* v, float vMin, float vMax, const char* displayFormat, float power);
    int (*slider_float2)(const char* label, float v[2], float vMin, float vMax, const char* displayFormat, float power);
    int (*slider_float3)(const char* label, float v[3], float vMin, float vMax, const char* displayFormat, float power);
    int (*slider_float4)(const char* label, float v[4], float vMin, float vMax, const char* displayFormat, float power);
    int (*slider_angle)(const char* label, float* v_rad, float vDegreesMin, float vDegreesMax);
    int (*slider_int)(const char* label, int* v, int vMin, int vMax, const char* displayFormat);
    int (*slider_int2)(const char* label, int v[2], int vMin, int vMax, const char* displayFormat);
    int (*slider_int3)(const char* label, int v[3], int vMin, int vMax, const char* displayFormat);
    int (*slider_int4)(const char* label, int v[4], int vMin, int vMax, const char* displayFormat);
    int (*vslider_float)(const char* label, const PDVec2 size, float* v, float vMin, float vMax, const char* displayFormat, float power);
    int (*vslider_int)(const char* label, const PDVec2 size, int* v, int vMin, int vMax, const char* displayFormat);
    int (*drag_float)(const char* label, float* v, float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
    int (*drag_float2)(const char* label, float v[2], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
    int (*drag_float3)(const char* label, float v[3], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
    int (*drag_float4)(const char* label, float v[4], float vSpeed, float vMin, float vMax, const char* displayFormat, float power);
    int (*drag_int)(const char* label, int* v, float vSpeed, int vMin, int vMax, const char* displayFormat);
    int (*drag_int2)(const char* label, int v[2], float vSpeed, int vMin, int vMax, const char* displayFormat);
    int (*drag_int3)(const char* label, int v[3], float vSpeed, int vMin, int vMax, const char* displayFormat);
    int (*drag_int4)(const char* label, int v[4], float vSpeed, int vMin, int vMax, const char* displayFormat);
    int (*input_text)(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDUIInputTextCallbackData*), void* user_data);
    int (*input_text_multiline)(const char* label, char* buf, size_t buf_size, const PDVec2 size, PDUIInputTextFlags flags, void (*callback)(PDUIInputTextCallbackData*), void* user_data);
    int (*input_float)(const char* label, float* v, float step, float step_fast, int decimal_precision, PDUIInputTextFlags extraFlags);
    int (*input_float2)(const char* label, float v[2], int decimal_precision, PDUIInputTextFlags extraFlags);
    int (*input_float3)(const char* label, float v[3], int decimal_precision, PDUIInputTextFlags extraFlags);
    int (*input_float4)(const char* label, float v[4], int decimal_precision, PDUIInputTextFlags extraFlags);
    int (*input_int)(const char* label, int* v, int step, int step_fast, PDUIInputTextFlags extraFlags);
    int (*input_int2)(const char* label, int v[2], PDUIInputTextFlags extraFlags);
    int (*input_int3)(const char* label, int v[3], PDUIInputTextFlags extraFlags);
    int (*input_int4)(const char* label, int v[4], PDUIInputTextFlags extraFlags);
    int (*tree_node)(const char* str_label_id);
    int (*tree_node_str)(const char* strId, const char* fmt, ...);
    int (*tree_node_ptr)(const void* ptrId, const char* fmt, ...);
    int (*tree_node_str_v)(const char* strId, const char* fmt, va_list args);
    int (*tree_node_ptr_v)(const void* ptrId, const char* fmt, va_list args);
    void (*tree_push_str)(const char* strId);
    void (*tree_push_ptr)(const void* ptrId);
    void (*tree_pop)();
    void (*set_next_tree_node_opened)(int opened, PDUISetCond cond);
    int (*selectable)(const char* label, int selected, PDUISelectableFlags flags, const PDVec2 size);
    int (*selectable_ex)(const char* label, int* p_selected, PDUISelectableFlags flags, const PDVec2 size);
    int (*list_box)(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems);
    int (*list_box2)(const char* label, int* currentItem, bool (*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems);
    int (*list_box_header)(const char* label, const PDVec2 size);
    int (*list_box_header2)(const char* label, int itemsCount, int heightInItems);
    void (*list_box_footer)();
    void (*set_tooltip)(const char* fmt, ...);
    void (*set_tooltip_v)(const char* fmt, va_list args);
    void (*begin_tooltip)();
    void (*end_tooltip)();
    int (*begin_main_menu_bar)();
    void (*end_main_menu_bar)();
    int (*begin_menu_bar)();
    void (*end_menu_bar)();
    int (*begin_menu)(const char* label, int enabled);
    void (*end_menu)();
    int (*menu_item)(const char* label, const char* shortcut, int selected, int enabled);
    int (*menu_item_ptr)(const char* label, const char* shortcut, int* p_selected, int enabled);
    void (*open_popup)(const char* strId);
    int (*begin_popup)(const char* strId);
    int (*begin_popup_modal)(const char* name, int* p_opened, PDUIWindowFlags extraFlags);
    int (*begin_popup_context_item)(const char* strId, int mouse_button);
    int (*begin_popup_context_window)(int also_over_items, const char* strId, int mouse_button);
    int (*begin_popup_context_void)(const char* strId, int mouse_button);
    void (*end_popup)();
    void (*close_current_popup)();
    int (*begin_popup_context)(void* priv_data);
    void (*end_popup_context)(void* priv_data);
    void (*value_bool)(const char* prefix, int b);
    void (*value_int)(const char* prefix, int v);
    void (*value_u_int)(const char* prefix, unsigned int v);
    void (*value_float)(const char* prefix, float v, const char* float_format);
    void (*color)(const char* prefix, const PDColor col);
    void (*log_to_tty)(int maxDepth);
    void (*log_to_file)(int maxDepth, const char* filename);
    void (*log_to_clipboard)(int maxDepth);
    void (*log_finish)();
    void (*log_buttons)();
    int (*is_item_hovered)();
    int (*is_item_hovered_rect)();
    int (*is_item_active)();
    int (*is_item_visible)();
    int (*is_any_item_hovered)();
    int (*is_any_item_active)();
    PDVec2 (*get_item_rect_min)();
    PDVec2 (*get_item_rect_max)();
    PDVec2 (*get_item_rect_size)();
    int (*is_window_hovered)();
    int (*is_window_focused)();
    int (*is_root_window_focused)();
    int (*is_root_window_or_any_child_focused)();
    int (*is_rect_visible)(const PDVec2 itemSize);
    int (*is_pos_hovering_any_window)(const PDVec2 pos);
    float (*get_time)();
    int (*get_frame_count)();
    const char* (*get_style_col_name)(PDUICol idx);
    PDVec2 (*calc_item_rect_closest_point)(const PDVec2 pos, int on_edge, float outward);
    PDVec2 (*calc_text_size)(const char* text, const char* text_end, int hide_text_after_double_hash, float wrap_width);
    void (*calc_list_clipping)(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end);
    int (*begin_child_frame)(PDID id, const struct PDVec2 size);
    void (*end_child_frame)();
    void (*color_convert_rg_bto_hsv)(float r, float g, float b, float* out_h, float* out_s, float* out_v);
    void (*color_convert_hs_vto_rgb)(float h, float s, float v, float* out_r, float* out_g, float* out_b);
    int (*is_key_down)(int key_index);
    int (*is_key_pressed)(int key_index, int repeat);
    int (*is_key_released)(int key_index);
    int (*is_key_down_id)(uint32_t keyId, int repeat);
    int (*is_mouse_down)(int button);
    int (*is_mouse_clicked)(int button, int repeat);
    int (*is_mouse_double_clicked)(int button);
    int (*is_mouse_released)(int button);
    int (*is_mouse_hovering_window)();
    int (*is_mouse_hovering_any_window)();
    int (*is_mouse_hovering_rect)(const PDVec2 rectMin, const PDVec2 rectMax);
    int (*is_mouse_dragging)(int button, float lockThreshold);
    PDVec2 (*get_mouse_pos)();
    PDVec2 (*get_mouse_drag_delta)(int button, float lockThreshold);
    void (*reset_mouse_drag_delta)(int button);
    PDUIMouseCursor (*get_mouse_cursor)();
    void (*set_mouse_cursor)(PDUIMouseCursor type);
    void (*fill_rect)(PDRect rect, unsigned int color);
    void (*fill_convex_poly)(void* verts, int count, PDColor color, int aa);
    void (*fill_circle)(PDVec2 pos, float radius, PDColor color, int num_seg, int aa);
    void* (*image_create_rgba)(int width, int height);
    void (*image_update)(void* dest, const void* src, int size);
};

// !!! End of autogenerated data. !!!
