extern crate prodbg_api;

use prodbg_api::ui::Ui;
use prodbg_api::ui_ffi::CPdUI;
use std::os::raw::{c_char, c_void, c_int, c_uchar};
use std::mem::transmute;
use prodbg_api::CFixedString;


pub struct Imgui  {
    show_popup: bool,
    showed_popup: c_int,
}

impl Imgui {
    pub fn create_ui_instance() -> Ui {
        Ui::new(Self::init_ui_funcs())
    }

    pub fn get_ui() -> Ui {
	    Ui::new(unsafe { imgui_get_ui_funcs() })
    }

    pub fn get_ui_funs() -> *mut CPdUI {
	    unsafe { imgui_get_ui_funcs() }
    }

    fn new() -> Imgui {
        Imgui {
            show_popup: false,
            showed_popup: 0,
        }
    }

	extern "C" fn begin_popup_context(priv_data: *mut c_void) -> c_int {
        let data = unsafe { &mut *(priv_data as *mut Imgui) };
	    let ui = Self::get_ui();

        if data.show_popup {
            ui.open_popup("_select");
            data.showed_popup = 1;
        }

        let showed_menu = ui.begin_popup("_select");

        if showed_menu {
            ui.text("test");
            ui.menu_item("Click from Rust", false, true);
        }

        if showed_menu { 1 } else { 0 }
    }

    /// Returns 1 if UI showed a popup or not

    pub fn has_showed_popup(api: *mut CPdUI) -> u32 {
        let data = unsafe { &mut *((*api).private_data as *mut Imgui) };
        data.showed_popup as u32
    }

    /// Marks if this ui instance should show a popup or not

    pub fn mark_show_popup(api: *mut CPdUI, state: bool) {
        let data = unsafe { &mut *((*api).private_data as *mut Imgui) };
        data.show_popup = state
    }

    /// Init the private state of the ui before start

    pub fn init_state(api: *mut CPdUI) {
        let data = unsafe { &mut *((*api).private_data as *mut Imgui) };
        data.showed_popup = 0
    }

    pub fn begin_window(name: &str, show: bool) -> bool {
        unsafe {
            let t = CFixedString::from_str(name).as_ptr();
            if imgui_begin(t, show as c_uchar) == 1 { true } else { false }
        }
    }

    pub fn begin_window_float(name: &str, show: bool) -> bool {
        unsafe {
            let t = CFixedString::from_str(name).as_ptr();
            if imgui_begin_float(t, show as c_uchar) == 1 { true } else { false }
        }
    }

    pub fn begin_window_child(name: &str, height: f32) {
    	unsafe {
            let t = CFixedString::from_str(name).as_ptr();
            imgui_begin_child(t, height);
    	}
    }

    pub fn end_window_child() {
    	unsafe {
    		imgui_end_child();
    	} 
    }

    pub fn map_key(key_target: usize, key_source: usize) {
        unsafe { imgui_map_key(key_target as u32, key_source as u32); }
    }

    pub fn set_mouse_pos(mouse: (f32, f32)) {
        unsafe { imgui_set_mouse_pos(mouse.0, mouse.1) };
    }

    pub fn set_mouse_state(index: usize, state: bool) {
        unsafe { imgui_set_mouse_state(index as i32, state as c_int); }
    }

    pub fn end_window() {
        unsafe { imgui_end(); }
    }

    pub fn set_window_pos(x: f32, y: f32) {
        unsafe { imgui_set_window_pos(x, y); }
    }

    #[inline]
    pub fn add_input_character(c: u16) {
        unsafe { imgui_add_input_character(c) };
    }

    #[inline]
    pub fn set_key_down(key: usize) {
        unsafe { imgui_set_key_down(key as i32); }
    }


    #[inline]
    pub fn clear_keys() {
        unsafe { imgui_clear_keys(); }
    }

    pub fn set_window_size(w: f32, h: f32) {
        unsafe { imgui_set_window_size(w, h); }
    }

    fn init_ui_funcs() -> *mut CPdUI {
        unsafe {
            let priv_data = transmute(Box::new(Imgui::new()));
            let mut ui_ffi = imgui_create_ui_funcs();
            (*ui_ffi).begin_popup_context = Self::begin_popup_context;
            (*ui_ffi).private_data = priv_data;

            ui_ffi
        }
    }

    //+Z
    pub fn tab(label: &str, selected: bool, last: bool) -> bool {
    	unsafe {
            let t = CFixedString::from_str(label).as_ptr();
            if imgui_tab(t, selected, last)==1 { true } else { false }
    	}
    }

    pub fn separator() {
        unsafe {
            imgui_separator()
        }
    }

    pub fn tab_pos() -> f32 {
        unsafe {
            imgui_tab_pos()
        }
    }

    //+Z
    pub fn render_frame(x: f32, y:f32, width:f32, height:f32, fill_col:u32) {
    	unsafe {
    		imgui_RenderFrame(x, y, width, height, fill_col)
    	}
    }
}

extern "C" {
    fn imgui_begin(name: *const c_char, show: c_uchar) -> c_int;
    fn imgui_begin_float(name: *const c_char, show: c_uchar) -> c_int;
    fn imgui_end();
    fn imgui_begin_child(name: *const c_char, h: f32);
    fn imgui_end_child();
    fn imgui_create_ui_funcs() -> *mut CPdUI;
    fn imgui_get_ui_funcs() -> *mut CPdUI;
    fn imgui_set_window_pos(x: f32, y: f32);
    fn imgui_set_window_size(w: f32, h: f32);
    fn imgui_add_input_character(c: u16);
    fn imgui_map_key(key_target: u32, key_source: u32);
    fn imgui_set_mouse_pos(x: f32, y: f32);
    fn imgui_set_mouse_state(index: i32, state: c_int);
    fn imgui_clear_keys();
    fn imgui_set_key_down(key: i32);
    fn imgui_tab(label: *const c_char, selected: bool, last: bool) -> c_int;
    fn imgui_separator();
    fn imgui_tab_pos() -> f32;
    fn imgui_RenderFrame(x: f32, y:f32, width:f32, height:f32, fill_col: u32);
}
