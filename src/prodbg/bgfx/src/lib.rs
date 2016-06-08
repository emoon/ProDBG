//extern crate prodbg_api;
extern crate libc;

use libc::{c_int, c_void};

pub struct Bgfx {
    pub temp: i32,
}

impl Bgfx {
    pub fn new() -> Bgfx {
        unsafe { bgfx_create(); }
        Bgfx {
            temp: 0
        }
    }

    pub fn create_window(window: *const c_void, width: c_int, height: c_int) {
        unsafe { bgfx_create_window(window, width, height); }
    }

    pub fn pre_update(&self) {
        unsafe { bgfx_pre_update(); }
    }

    pub fn update_window_size(width: c_int, height: c_int) {
        unsafe { bgfx_set_window_size(width, height); }
    }

    pub fn post_update(&self) {
        unsafe { bgfx_post_update(); }
    }

    pub fn cursor_init() {
        unsafe { cursor_init() };
    }

    pub fn cursor_set_type(t: i32) {
        unsafe { cursor_set_type(t) };
    }

    /*

    pub fn create_ui_funcs() -> *mut CPdUI {
        unsafe { bgfx_create_ui_funcs() }
    }

    pub fn has_showed_popup(&self, ui: *mut CPdUI) -> u32 {
        unsafe { bgfx_has_showed_popup(ui) }
    }

    pub fn mark_show_popup(&self, ui: *mut CPdUI, state: bool) {
        unsafe { bgfx_mark_show_popup(ui, state as u32); }
    }

    pub fn init_state(&self, ui: *mut CPdUI) {
        unsafe { bgfx_init_state(ui); }
    }

    pub fn imgui_begin(&self, show: bool) {
        unsafe { bgfx_imgui_begin(show as c_int); }
    }

    pub fn imgui_end(&self) {
        unsafe { bgfx_imgui_end(); }
    }

    */

}

impl Drop for Bgfx {
    fn drop(&mut self) {
        unsafe { bgfx_destroy(); }
    }
}

extern "C" {
    fn bgfx_pre_update();
    fn bgfx_post_update();
    fn bgfx_create();
    fn bgfx_create_window(window: *const c_void, width: c_int, height: c_int);
    fn bgfx_destroy();
    fn bgfx_set_window_size(width: c_int, height: c_int);

    fn cursor_init();
    fn cursor_set_type(t: i32);

    //fn bgfx_has_showed_popup(ui: *mut CPdUI) -> u32;
    //fn bgfx_mark_show_popup(ui: *mut CPdUI, state: u32);
    //fn bgfx_init_state(ui: *mut CPdUI);

    //fn bgfx_imgui_begin(show: c_int);
    //fn bgfx_imgui_end();

    //fn bgfx_create_ui_funcs() -> *mut CPdUI;
}

