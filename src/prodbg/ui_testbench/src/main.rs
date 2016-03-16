extern crate core;
extern crate libc;
extern crate minifb;
extern crate prodbg_api;
extern crate bgfx;

use core::{DynamicReload, Search};
use minifb::{Window, Key, Scale, WindowOptions, MouseMode, MouseButton};
use libc::{c_void};
use prodbg_api::view::CViewCallbacks;
use prodbg_api::ui::Ui;
use prodbg_api::ui_ffi::{PDVec2};
use core::view_plugins::{ViewPlugins};
use std::rc::Rc;
use std::cell::RefCell;

// use core::plugin_handler::*;
use core::plugins::*;
use std::ptr;
use bgfx::Bgfx;

const WIDTH: usize = 1024;
const HEIGHT: usize = 800;

fn is_inside(v: (f32, f32), pos: PDVec2, size: PDVec2) -> bool {
    let x0 = pos.x;
    let y0 = pos.y;
    let x1 = pos.x + size.x;
    let y1 = pos.y + size.y;

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

    let mut lib_handler = DynamicReload::new(None, Some("t2-output"), Search::Backwards);
    let mut plugins = Plugins::new();

    // Would be nice to nat have it this way
    let view_plugins = Rc::new(RefCell::new(ViewPlugins::new()));

    plugins.add_handler(&view_plugins);
    plugins.add_plugin(&mut lib_handler, "registers_plugin");

    let ui = Ui::new(bgfx.create_ui_funcs());

    view_plugins.borrow_mut().create_instance(ui, &"Registers View".to_owned());

    bgfx.create_window(window.get_window_handle() as *mut c_void,
                           WIDTH as i32,
                           HEIGHT as i32);

    while window.is_open() && !window.is_key_down(Key::Escape) {
        plugins.update(&mut lib_handler);

        bgfx.pre_update();

        let mouse = window.get_mouse_pos(MouseMode::Clamp).unwrap_or((0.0, 0.0));
        let mut has_shown_menu = 0u32;

        bgfx.set_mouse_pos(mouse);
        bgfx.set_mouse_state(0, window.get_mouse_down(MouseButton::Left));

        let show_context_menu = window.get_mouse_down(MouseButton::Right);

        for instance in &view_plugins.borrow_mut().instances {
            //bgfx_imgui_set_window_pos(0.0, 0.0);
            //bgfx_imgui_set_window_size(500.0, 500.0); 

            bgfx.imgui_begin(true);

            let ui = instance.ui;
            let pos = ui.get_window_pos();
            let size = ui.get_window_size();

            bgfx.init_state(ui.api);

            if is_inside(mouse, pos, size) && show_context_menu {
                bgfx.mark_show_popup(ui.api, true);
            } else {
                bgfx.mark_show_popup(ui.api, false);
            }

            unsafe {
                let plugin_funcs = instance.plugin_type.plugin_funcs as *mut CViewCallbacks;
                ((*plugin_funcs).update.unwrap())(instance.plugin_data,
                                                    ui.api as *mut c_void,
                                                    ptr::null_mut(),
                                                    ptr::null_mut());
            }

            has_shown_menu |= bgfx.has_showed_popup(ui.api);

            bgfx.imgui_end();
        }

        // if now plugin has showed a menu we do it here

        if has_shown_menu == 0 && show_context_menu {
            bgfx.test_menu(true);
        } else {
            bgfx.test_menu(false);
        }

        bgfx.post_update();

        window.update();
    }

    //unsafe {
     //   bgfx_destroy();
    //}
}

