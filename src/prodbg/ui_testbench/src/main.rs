extern crate core;
extern crate libc;
extern crate minifb;
extern crate prodbg_api;
extern crate bgfx_rs;
extern crate imgui_sys;

use core::{DynamicReload, Search};
use minifb::{Window, Key, Scale, WindowOptions, MouseMode, MouseButton};
use libc::{c_void};
use prodbg_api::view::CViewCallbacks;
//use prodbg_api::ui::Ui;
use prodbg_api::ui_ffi::{PDVec2};
use core::view_plugins::{ViewPlugins};
use core::session::{Sessions, SessionHandle};
use std::rc::Rc;
use std::cell::RefCell;
use imgui_sys::Imgui;

// use core::plugin_handler::*;
use core::plugins::*;
//use std::ptr;
use bgfx_rs::Bgfx;

const WIDTH: usize = 1024;
const HEIGHT: usize = 800;

fn is_inside(v: (f32, f32), pos: PDVec2, size: (f32, f32)) -> bool {
    let x0 = pos.x;
    let y0 = pos.y;
    let x1 = pos.x + size.0;
    let y1 = pos.y + size.1;

    if (v.0 >= x0 && v.0 < x1) && (v.1 >= y0 && v.1 < y1) {
        true
    } else {
        false
    }
}

fn main() {
    let bgfx = Bgfx::new();

    let mut window = Window::new("Noise Test - Press ESC to exit",
                                 WIDTH,
                                 HEIGHT,
                                 WindowOptions {
                                     resize: true,
                                     scale: Scale::X1,
                                     ..WindowOptions::default()
                                 })
                         .expect("Unable to create window");

    let mut sessions = Sessions::new();
    let mut lib_handler = DynamicReload::new(None, Some("t2-output"), Search::Backwards);
    let mut plugins = Plugins::new();

    // Would be nice to nat have it this way
    let view_plugins = Rc::new(RefCell::new(ViewPlugins::new()));
    sessions.create_instance();

    plugins.add_handler(&view_plugins);
    plugins.add_plugin(&mut lib_handler, "sourcecode_plugin");

    let ui = Imgui::create_ui_instance();

    view_plugins.borrow_mut().create_instance(ui, &"Source Code View".to_owned(), SessionHandle(0));

    Bgfx::create_window(window.get_window_handle() as *mut c_void,
                           WIDTH as i32,
                           HEIGHT as i32);

    while window.is_open() && !window.is_key_down(Key::Escape) {
        plugins.update(&mut lib_handler);

        bgfx.pre_update();

        let mouse = window.get_mouse_pos(MouseMode::Clamp).unwrap_or((0.0, 0.0));
        let mut _has_shown_menu = 0u32;

        Imgui::set_mouse_pos(mouse);
        Imgui::set_mouse_state(0, window.get_mouse_down(MouseButton::Left));

        let show_context_menu = window.get_mouse_down(MouseButton::Right);

        for instance in &view_plugins.borrow_mut().instances {
            let ui = &instance.ui;

            Imgui::begin_window("Test", true);
            Imgui::init_state(ui.api);

            let pos = ui.get_window_pos();
            let size = ui.get_window_size();

            if is_inside(mouse, pos, size) && show_context_menu {
                Imgui::mark_show_popup(ui.api, true);
            } else {
                Imgui::mark_show_popup(ui.api, false);
            }

            unsafe {
                let plugin_funcs = instance.plugin_type.plugin_funcs as *mut CViewCallbacks;
                let session = sessions.get_session(SessionHandle(0)).unwrap();
                ((*plugin_funcs).update.unwrap())(instance.plugin_data,
                                                    ui.api as *mut c_void,
                                                    session.reader.api as *mut c_void,
                                                    session.get_current_writer().api as *mut c_void);
            }

            _has_shown_menu |= Imgui::has_showed_popup(ui.api);

            Imgui::end_window();
        }

        // if now plugin has showed a menu we do it here

        /*
        if has_shown_menu == 0 && show_context_menu {
            Bgfx::test_menu(true);
        } else {
            Bgfx::test_menu(false);
        }
        */

        bgfx.post_update();

        window.update();
    }

    //unsafe {
     //   bgfx_destroy();
    //}
}

// dummy
#[no_mangle]
pub fn init_plugin() {
}
