use std::ptr;
use ui_ffi::*;
use std::mem;
use std::fmt;
use std::fmt::Write;
use scintilla::Scintilla;
use std::os::raw::{c_char, c_int, c_void};

use CFixedString;

/// Taken from minifb. Not sure if we can share this somehow?
/// Key is used by the get key functions to check if some keys on the keyboard has been pressed
#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Key {
    Key0 = 0,
    Key1 = 1,
    Key2 = 2,
    Key3 = 3,
    Key4 = 4,
    Key5 = 5,
    Key6 = 6,
    Key7 = 7,
    Key8 = 8,
    Key9 = 9,

    A = 10,
    B = 11,
    C = 12,
    D = 13,
    E = 14,
    F = 15,
    G = 16,
    H = 17,
    I = 18,
    J = 19,
    K = 20,
    L = 21,
    M = 22,
    N = 23,
    O = 24,
    P = 25,
    Q = 26,
    R = 27,
    S = 28,
    T = 29,
    U = 30,
    V = 31,
    W = 32,
    X = 33,
    Y = 34,
    Z = 35,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,

    Down,
    Left,
    Right,
    Up,
    Apostrophe,
    Backquote,

    Backslash,
    Comma,
    Equal,
    LeftBracket,
    Minus,
    Period,
    RightBracket,
    Semicolon,

    Slash,
    Backspace,
    Delete,
    End,
    Enter,

    Escape,

    Home,
    Insert,
    Menu,

    PageDown,
    PageUp,

    Pause,
    Space,
    Tab,
    NumLock,
    CapsLock,
    ScrollLock,
    LeftShift,
    RightShift,
    LeftCtrl,
    RightCtrl,

    NumPad0,
    NumPad1,
    NumPad2,
    NumPad3,
    NumPad4,
    NumPad5,
    NumPad6,
    NumPad7,
    NumPad8,
    NumPad9,
    NumPadDot,
    NumPadSlash,
    NumPadAsterisk,
    NumPadMinus,
    NumPadPlus,
    NumPadEnter,

    LeftAlt,
    RightAlt,

    LeftSuper,
    RightSuper,

    /// Used when an Unknown key has been pressed
    Unknown,

    Count = 107,
}


// Enumeration for PushStyleColor() / PopStyleColor()
pub enum ImGuiCol {
    Text = 0,
    TextDisabled,
    WindowBg,              // Background of normal windows
    ChildWindowBg,         // Background of child windows
    PopupBg,               // Background of popups, menus, tooltips windows
    Border,
    BorderShadow,
    FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    FrameBgHovered,
    FrameBgActive,
    TitleBg,
    TitleBgCollapsed,
    TitleBgActive,
    MenuBarBg,
    ScrollbarBg,
    ScrollbarGrab,
    ScrollbarGrabHovered,
    ScrollbarGrabActive,
    ComboBg,
    CheckMark,
    SliderGrab,
    SliderGrabActive,
    Button,
    ButtonHovered,
    ButtonActive,
    Header,
    HeaderHovered,
    HeaderActive,
    Column,
    ColumnHovered,
    ColumnActive,
    ResizeGrip,
    ResizeGripHovered,
    ResizeGripActive,
    CloseButton,
    CloseButtonHovered,
    CloseButtonActive,
    PlotLines,
    PlotLinesHovered,
    PlotHistogram,
    PlotHistogramHovered,
    TextSelectedBg,
    ModalWindowDarkening,  // darken entire screen when a modal window is active
}


pub enum ImGuiStyleVar
{
    Alpha = 0,           // float
    WindowPadding,       // ImVec2
    WindowRounding,      // float
    WindowMinSize,       // ImVec2
    ChildWindowRounding, // float
    FramePadding,        // ImVec2
    FrameRounding,       // float
    ItemSpacing,         // ImVec2
    ItemInnerSpacing,    // ImVec2
    IndentSpacing,       // float
    GrabMinSize          // float
}


#[derive(Clone)]
pub struct Ui {
    pub api: *mut CPdUI,
    fmt_buffer: String,
}

#[derive(Clone, Copy, Debug)]
pub struct Color {
    color: u32,
}

impl Color {
    pub fn from_u32(color: u32) -> Color {
        Color { color: color }
    }

    pub fn from_rgb(r: u32, g: u32, b: u32) -> Color {
        Self::from_u32((((r << 16) | (g << 8) | b) << 8) | 0xff)
    }

    pub fn from_rgba(r: u32, g: u32, b: u32, a: u32) -> Color {
        Self::from_u32((((r << 16) | (g << 8) | b) << 8) | a)
    }

    pub fn from_argb(a: u32, r: u32, g: u32, b: u32) -> Color {
        Self::from_u32((((r << 16) | (g << 8) | b) << 8) | a)
    }

    pub fn from_au32(a: u32, rgb: u32) -> Color {
        Self::from_u32((rgb << 8) | a)
    }
}

pub struct Image {
    api: *mut CPdUI,
    handle: *mut c_void,
    size: Vec2,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Vec2 {
    pub x: f32,
    pub y: f32,
}

impl Vec2 {
    pub fn new(x: f32, y: f32) -> Vec2 {
        Vec2 { x: x, y: y }
    }
}

macro_rules! true_is_1 {
    ($e:expr) => (if $e { 1 } else { 0 })
}

pub struct ImageBuilder<'a> {
    pub api: *mut CPdUI,
    image: &'a Image,
    size: Vec2,
    uv0: Vec2,
    uv1: Vec2,
    tint_color: Color,
    border_color: Color,
}

pub struct InputTextCallbackData<'a>(&'a mut PDUIInputTextCallbackData);

impl<'a> InputTextCallbackData<'a> {
    pub fn new(original_data: &mut PDUIInputTextCallbackData) -> InputTextCallbackData {
        InputTextCallbackData(original_data)
    }

    pub fn get_cursor_pos(&mut self) -> i32 {
        self.0.cursor_pos
    }

    pub fn set_cursor_pos(&mut self, pos: i32) {
        self.0.cursor_pos = pos
    }

    /// Returns input character. Returns `\u{0}` for no character. Returns `None` if character is
    /// not valid UTF-16 character or it requires more than 1 UTF-16 character to be encoded.
    pub fn get_event_char(&self) -> Option<char> {
        ::std::char::decode_utf16(Some(self.0.event_char as u16))
            .next()
            .and_then(|res| res.ok())
    }

    /// Change input character. Set to `\u{0}` to cancel input. Will not be changed if character
    /// `c` cannot be encoded into single UTF-16 character.
    pub fn set_event_char(&mut self, c: char) {
        if c.len_utf16() == 1 {
            self.0.event_char = c as u16;
        }
    }

    pub fn get_event_flag(&self) -> PDUIInputTextFlags_ {
        PDUIInputTextFlags_::from_bits_truncate(self.0.event_flag)
    }
}

impl Ui {
    pub fn new(native_api: *mut CPdUI) -> Ui {
        Ui {
            api: native_api,
            fmt_buffer: String::with_capacity(2048),
        }
    }

    pub fn set_title(&self, title: &str) {
        unsafe {
            let t = CFixedString::from_str(title).as_ptr();
            ((*self.api).set_title)((*self.api).private_data, t);
        }
    }

    #[inline]
    pub fn get_window_size(&self) -> (f32, f32) {
        unsafe {
            let t = ((*self.api).get_window_size)();
            (t.x, t.y)
        }
    }

    #[inline]
    pub fn get_window_pos(&self) -> PDVec2 {
        unsafe { ((*self.api).get_window_pos)() }
    }

    #[inline]
    pub fn get_text_line_height_with_spacing(&self) -> f32 {
        unsafe { ((*self.api).get_text_line_height_with_spacing)() as f32 }
    }

    #[inline]
    pub fn get_cursor_pos(&self) -> (f32, f32) {
        unsafe {
            let t = ((*self.api).get_cursor_pos)();
            (t.x, t.y)
        }
    }

    #[inline]
    pub fn set_cursor_pos(&self, pos: (f32, f32)) {
        unsafe {
            ((*self.api).set_cursor_pos)(PDVec2{x: pos.0, y: pos.1});
        }
    }

    #[inline]
    pub fn get_cursor_screen_pos(&self) -> (f32, f32) {
        unsafe {
            let t = ((*self.api).get_cursor_screen_pos)();
            (t.x, t.y)
        }
    }

    #[inline]
    pub fn set_cursor_screen_pos(&self, pos: (f32, f32)) {
        unsafe {
            ((*self.api).set_cursor_screen_pos)(PDVec2{x: pos.0, y: pos.1});
        }
    }

    #[inline]
    pub fn fill_rect(&self, x: f32, y: f32, width: f32, height: f32, col: Color) {
        unsafe {
            ((*self.api).fill_rect)(PDRect { x: x, y: y, width: width, height: height }, col.color);
        }
    }

    #[inline]
    pub fn set_scroll_here(&self, center: f32) {
        unsafe { ((*self.api).set_scroll_here)(center) }
    }

    pub fn begin_child(&self, id: &str, pos: Option<PDVec2>, border: bool, flags: PDUIWindowFlags_) {
        unsafe {
            let t = CFixedString::from_str(id).as_ptr();
            match pos {
                Some(p) => ((*self.api).begin_child)(t, p, border as i32, flags.bits()),
                None => {
                    ((*self.api).begin_child)(t,
                                              PDVec2 { x: 0.0, y: 0.0 },
                                              border as i32,
                                              flags.bits())
                }
            }
        }
    }

    #[inline]
    pub fn end_child(&self) {
        unsafe { ((*self.api).end_child)() }
    }

    #[inline]
    pub fn get_scroll_y(&self) -> f32 {
        unsafe { ((*self.api).get_scroll_y)() as f32 }
    }

    #[inline]
    pub fn get_scroll_max_y(&self) -> f32 {
        unsafe { ((*self.api).get_scroll_max_y)() as f32 }
    }

    #[inline]
    pub fn set_scroll_y(&self, pos: f32) {
        unsafe { ((*self.api).set_scroll_y)(pos) }
    }

    #[inline]
    pub fn set_scroll_from_pos_y(&self, pos_y: f32, center_ratio: f32) {
        unsafe { ((*self.api).set_scroll_from_pos_y)(pos_y, center_ratio) }
    }

    #[inline]
    pub fn set_keyboard_focus_here(&self, offset: i32) {
        unsafe { ((*self.api).set_keyboard_focus_here)(offset) }
    }

    // TODO: push/pop font

    #[inline]
	pub fn push_style_color(&self, index: ImGuiCol, col: Color) {
        unsafe { ((*self.api).push_style_color)(index as u32, col.color) }
    }

    #[inline]
	pub fn pop_style_color(&self, count: usize) {
        unsafe { ((*self.api).pop_style_color)(count as i32) }
    }

    #[inline]
	pub fn push_style_var(&self, index: ImGuiStyleVar, val: f32) {
        unsafe { ((*self.api).push_style_var)(index as u32, val) }
    }

    #[inline]
	pub fn push_style_var_vec(&self, index: ImGuiStyleVar, val: PDVec2) {
        unsafe { ((*self.api).push_style_var_vec)(index as u32, val) }
    }

    #[inline]
	pub fn pop_style_var(&self, count: usize) {
        unsafe { ((*self.api).pop_style_var)(count as i32) }
    }

    #[inline]
    pub fn get_font_size(&self) -> f32 {
        unsafe { ((*self.api).get_font_size)() }
    }

    #[inline]
    pub fn push_item_width(&self, width: f32) {
        unsafe { ((*self.api).push_item_width)(width) }
    }

    #[inline]
    pub fn pop_item_width(&self) {
        unsafe { ((*self.api).pop_item_width)(); }
    }

    #[inline]
    pub fn same_line(&self, column_x: i32, spacing_w: i32) {
        unsafe { ((*self.api).same_line)(column_x, spacing_w) }
    }

    #[inline]
    pub fn is_item_hovered(&self) -> bool {
        unsafe { ((*self.api).is_item_hovered)() != 0}
    }

    #[inline]
    pub fn get_item_rect_min(&self) -> PDVec2 {
        unsafe { ((*self.api).get_item_rect_min)() }
    }

    #[inline]
    pub fn get_item_rect_max(&self) -> PDVec2 {
        unsafe { ((*self.api).get_item_rect_max)() }
    }

    #[inline]
    pub fn get_item_rect_size(&self) -> PDVec2 {
        unsafe { ((*self.api).get_item_rect_size)() }
    }

    #[inline]
    pub fn is_mouse_clicked(&self, button: i32, repeat: bool) -> bool {
        unsafe { ((*self.api).is_mouse_clicked)(button, true_is_1!(repeat)) != 0}
    }

    #[inline]
    pub fn checkbox(&self, label: &str, state: &mut bool) -> bool{
        unsafe {
            let c_label = CFixedString::from_str(label).as_ptr();
            let mut c_state: i32 = if *state { 1 } else { 0 };
            let res = ((*self.api).checkbox)(c_label, &mut c_state) != 0;
            *state = c_state != 0;
            res
        }
    }

    #[inline]
    // callback is not called the same frame as input was created
    pub fn input_text(&self, label: &str, buf: &mut [u8], flags: PDUIInputTextFlags_, callback: Option<&FnMut(InputTextCallbackData)>) -> bool {
        unsafe {
            let c_label = CFixedString::from_str(label).as_ptr();
            let buf_len = buf.len() as i32;
            let buf_pointer = buf.as_mut_ptr() as *mut i8;
            if let Some(callback) = callback {
                extern fn cb(data: *mut PDUIInputTextCallbackData) {
                    unsafe {
                        let callback: *const *mut FnMut(InputTextCallbackData) = mem::transmute((*data).user_data);
                        (**callback)(InputTextCallbackData(&mut *data));
                    }
                };
                ((*self.api).input_text)(c_label, buf_pointer, buf_len, flags.bits(), cb, mem::transmute(&callback)) != 0
            } else {
                ((*self.api).input_text)(c_label, buf_pointer, buf_len, flags.bits(), mem::transmute(ptr::null::<()>()), ptr::null_mut()) != 0
            }
        }
    }

    /// Combobox
    /// `height` is number of lines when combobox is open
    /// Returns `true` if `current_item` was changed
    pub fn combo(&self, label: &str, current_item: &mut usize, items: &[&str], count: usize, height: usize) -> bool {
        extern fn c_get_item(closure: *mut c_void, item_index: c_int, res: *mut *const c_char) -> c_int {
            unsafe {
                let get_data: *const *mut FnMut(c_int, &mut *const c_char) -> c_int = mem::transmute(closure);
                (**get_data)(item_index, &mut *res)
            }
        }
        unsafe {
            let c_label = CFixedString::from_str(label).as_ptr();
            let mut item = *current_item as i32;
            let mut buffer = CFixedString::new();
            let res;
            {
                let get_item = |item_index: c_int, res: &mut *const c_char| -> c_int {
                    buffer = CFixedString::from_str(items[item_index as usize]);
                    *res = buffer.as_ptr();
                    1
                };
                let tmp = &get_item as &FnMut(c_int, &mut *const c_char) -> c_int;
                res = ((*self.api).combo3)(c_label, &mut item, c_get_item, mem::transmute(&tmp), count as i32, height as i32) != 0;
            }
            drop(buffer);
            *current_item = item as usize;
            res
        }
    }

    #[inline]
    pub fn calc_text_size(&self, text: &str, offset: usize) -> (f32, f32) {
        unsafe {
            let start = CFixedString::from_str(text);
            if offset == 0 {
                let t = ((*self.api).calc_text_size)(start.as_ptr(), ptr::null(), 0, -1.0);
                (t.x, t.y)
            } else {
                let slice = &start.as_str()[offset..];
                let t = ((*self.api).calc_text_size)(start.as_ptr(), slice.as_ptr(), 0, -1.0);
                (t.x, t.y)
            }
        }
    }

    #[inline]
    pub fn calc_list_clipping(&self, items_height: f32) -> (usize, usize) {
        unsafe {
            let mut start_index = 0i32;
            let mut end_index = 0i32;
            ((*self.api).calc_list_clipping)(i32::max_value(), items_height, &mut start_index, &mut end_index);
            (start_index as usize, end_index as usize)
        }
    }

    ///
    /// Ids
    ///

    // This version is added since `usize` cannot be converted into pointer in safe Rust.
    #[inline]
    pub fn push_id_usize(&self, id: usize) {
        unsafe { ((*self.api).push_id_ptr)(mem::transmute(id)) }
    }

    #[inline]
    pub fn push_id_ptr<T>(&self, id: &T) {
        unsafe { ((*self.api).push_id_ptr)(mem::transmute(id)) }
    }

    #[inline]
    pub fn push_id_int(&self, id: i32) {
        unsafe { ((*self.api).push_id_int)(id) }
    }

    #[inline]
    pub fn pop_id(&self) {
        unsafe { ((*self.api).pop_id)(); }
    }

    // Text

    pub fn text(&self, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).text)(t);
        }
    }

    pub fn text_fmt(&mut self, args: fmt::Arguments) {
        unsafe {
            self.fmt_buffer.clear();
            write!(&mut self.fmt_buffer, "{}", args).unwrap();
            let t = CFixedString::from_str(&self.fmt_buffer).as_ptr();
            ((*self.api).text)(t);
        }
    }

    pub fn text_colored(&self, col: Color, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).text_colored)(col.color, t);
        }
    }

    pub fn text_disabled(&self, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).text_disabled)(t);
        }
    }

    pub fn text_wrapped(&self, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).text_wrapped)(t);
        }
    }

	pub fn columns(&self, count: isize, id: Option<&str>, border: bool) {
	    unsafe {
            match id {
                Some(p) => {
                    let t = CFixedString::from_str(p).as_ptr();
                    ((*self.api).columns)(count as i32, t, border as i32)
                }
                None => ((*self.api).columns)(count as i32, ptr::null(), border as i32),
            }
        }
    }

    #[inline]
    pub fn next_column(&self) {
        unsafe { ((*self.api).next_column)() }
    }

    pub fn button(&self, title: &str, pos: Option<PDVec2>) -> bool {
        unsafe {
            let t = CFixedString::from_str(title).as_ptr();
            match pos {
                Some(p) => ((*self.api).button)(t, p) != 0,
                None => ((*self.api).button)(t, PDVec2 { x: 0.0, y: 0.0 }) != 0,
            }
        }
    }
    pub fn begin_popup(&self, text: &str) -> bool {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).begin_popup)(t) == 1
        }
    }

    pub fn begin_menu(&self, text: &str, enabled: bool) -> bool {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            let s = if enabled { 1 } else { 0 };
            ((*self.api).begin_menu)(t, s) == 1
        }
    }

    pub fn open_popup(&self, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).open_popup)(t);
        }
    }

	pub fn menu_item(&self, text: &str, selected: bool, enabled: bool) -> bool {
        unsafe {
            let name = CFixedString::from_str(text).as_ptr();
            let s = if selected { 1 } else { 0 };
            let e = if enabled { 1 } else { 0 };
            ((*self.api).menu_item)(name, ptr::null(), s, e) == 1
        }
    }

    pub fn end_menu(&self) {
        unsafe { ((*self.api).end_menu)() }
    }

    pub fn end_popup(&self) {
        unsafe { ((*self.api).end_popup)() }
    }

    ///
    ///
    ///

	pub fn tree_node<'a>(&'a self, label: &'a str) -> TreeBuilder<'a> {
		TreeBuilder {
			ui: self,
			label: label,
		}
	}

	pub fn sc_input_text(&self, title: &str, width: usize, height: usize) -> Scintilla {
	    unsafe {
            let name = CFixedString::from_str(title);
	        Scintilla::new(((*self.api).sc_input_text)(name.as_ptr(),
	                        width as f32, height as f32))
        }
    }

    pub fn image<'a>(&'a self, image: &'a Image) -> ImageBuilder {
        ImageBuilder {
            api: self.api,
            image: image,
            size: image.size,
            uv0: Vec2::new(0.0, 0.0),
            uv1: Vec2::new(1.0, 1.0),
            tint_color: Color::from_rgba(255, 255, 255, 255),
            border_color: Color::from_rgba(0, 0, 0, 0),
        }
    }

    ///
    /// Keyboard support
    ///

	pub fn is_key_down(&self, key: Key) -> bool {
        unsafe { ((*self.api).is_key_down)(key as i32) == 1 }
    }

	pub fn is_key_pressed(&self, key: Key, repeat: bool) -> bool {
        unsafe { ((*self.api).is_key_pressed)(key as i32, true_is_1!(repeat)) == 1 }
    }

	pub fn is_key_released(&self, key: Key) -> bool {
        unsafe { ((*self.api).is_key_released)(key as i32) == 1 }
    }

    ///
    /// Mouse support
    ///

	pub fn get_mouse_pos(&self) -> PDVec2 {
        unsafe { ((*self.api).get_mouse_pos)() }
    }

	pub fn get_mouse_wheel(&self) -> f32 {
        unsafe { ((*self.api).get_mouse_wheel)() }
    }

    ///
    /// Rendering primitives
    ///

    pub fn fill_convex_poly(&self, vertices: &[Vec2], col: Color, anti_aliased: bool) {
	    unsafe {
            ((*self.api).fill_convex_poly)(
                vertices.as_ptr() as *const c_void,
                vertices.len() as u32,
                col.color,
                true_is_1!(anti_aliased))
        }
    }

    pub fn fill_circle(&self, pos: &Vec2, radius: f32, col: Color, segment_count: usize, anti_aliased: bool) {
	    unsafe {
            ((*self.api).fill_circle)(
                PDVec2 { x: pos.x, y: pos.y },
                radius,
                col.color,
                segment_count as u32,
                true_is_1!(anti_aliased))
        }
    }

    ///
    /// Image
    ///

    pub fn image_create_rgba(&self, width: u32, height: u32) -> Option<Image> {
        unsafe {
            let handle = ((*self.api).image_create_rgba)(width, height);
            if handle != ptr::null_mut() {
                Some(Image {
                    api: self.api,
                    handle: handle,
                    size: Vec2 { x: width as f32, y: height as f32 },
                })
            } else {
                None
            }
        }
    }
}

pub struct TreeBuilder<'a> {
	ui: &'a Ui,
	label: &'a str,
}

impl<'a> TreeBuilder<'a> {
	pub fn show<F: FnOnce(&Ui)>(&self, f: F) {
		unsafe {
            let label = CFixedString::from_str(self.label);
            let t = ((*self.ui.api).tree_node)(label.as_ptr());

            if t == 1 {
                f(self.ui);
                ((*self.ui.api).tree_pop)();
            }
		}
	}
}

impl Image {
    pub fn update<T>(&self, data: &[T]) {
        unsafe {
            let size = data.len() * mem::size_of::<T>();
            ((*self.api).image_update)(self.handle, data.as_ptr() as *const c_void, size as u32);
        }
    }
}

impl<'a> ImageBuilder<'a> {
    pub fn show(&self) {
        unsafe {
            ((*self.api).image)(self.image.handle,
                                PDVec2 { x:self.size.x, y:self.size.y },
                                PDVec2 { x:self.uv0.x, y:self.uv0.y },
                                PDVec2 { x:self.uv1.x, y:self.uv1.y },
                                self.tint_color.color,
                                self.border_color.color);
        }
    }
}

