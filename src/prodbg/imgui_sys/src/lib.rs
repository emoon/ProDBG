extern crate prodbg_api;

use prodbg_api::ui::Ui;
use prodbg_api::ui_ffi::CPdUI;
use std::os::raw::{c_void, c_int};
use std::mem::transmute;

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

    pub fn has_showed_popup(api: *mut CPdUI) -> u32 {
        let data = unsafe { &mut *((*api).private_data as *mut Imgui) };
        data.showed_popup as u32
    }

    pub fn init_state(api: *mut CPdUI) {
        let data = unsafe { &mut *((*api).private_data as *mut Imgui) };
        data.showed_popup = 0
    }

    pub fn mark_show_popup(api: *mut CPdUI, state: bool) {
        let data = unsafe { &mut *((*api).private_data as *mut Imgui) };
        data.show_popup = state
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
}

extern "C" {
    fn imgui_create_ui_funcs() -> *mut CPdUI;
    fn imgui_get_ui_funcs() -> *mut CPdUI;
}
