extern crate core;
extern crate libc;
extern crate minifb;
extern crate prodbg_api;

use core::{DynamicReload, Search};
use minifb::{Window, Key, Scale, WindowOptions, MouseMode, MouseButton};
use libc::{c_void, c_int};
use prodbg_api::view::CViewCallbacks;
use core::view_plugins::{ViewPlugins};
use std::rc::Rc;
use std::cell::RefCell;

// use core::plugin_handler::*;
use core::plugins::*;
use std::ptr;

const WIDTH: usize = 1024;
const HEIGHT: usize = 800;

fn main() {
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
    plugins.add_plugin(&mut lib_handler, "bitmap_memory");

    view_plugins.borrow_mut().create_instance(&"Bitmap View".to_owned());

    unsafe {
        bgfx_create();
        bgfx_create_window(window.get_window_handle() as *mut c_void,
                           WIDTH as i32,
                           HEIGHT as i32);
    }

    while window.is_open() && !window.is_key_down(Key::Escape) {
        plugins.update(&mut lib_handler);

        unsafe {
            bgfx_pre_update();

            for instance in &view_plugins.borrow_mut().instances {
                //bgfx_imgui_set_window_pos(0.0, 0.0);
                //bgfx_imgui_set_window_size(500.0, 500.0); 

                bgfx_imgui_begin(1);

                let plugin_funcs = instance.plugin_type.plugin_funcs as *mut CViewCallbacks;
                ((*plugin_funcs).update.unwrap())(instance.user_data,
                                                  bgfx_get_ui_funcs(),
                                                  ptr::null_mut(),
                                                  ptr::null_mut());

                bgfx_imgui_end();
            }

            bgfx_post_update();

            window.get_mouse_pos(MouseMode::Clamp).map(|mouse| {
                prodbg_set_mouse_pos(mouse.0, mouse.1);
                prodbg_set_mouse_state(0, window.get_mouse_down(MouseButton::Left) as c_int);
            });
        }

        window.update();
    }

    unsafe {
        bgfx_destroy();
    }
}

///
///
///
///

extern "C" {
    fn bgfx_pre_update();
    fn bgfx_post_update();
    fn bgfx_create();
    fn bgfx_create_window(window: *const c_void, width: c_int, height: c_int);
    fn bgfx_destroy();

    fn prodbg_set_mouse_pos(x: f32, y: f32);
    fn prodbg_set_mouse_state(mouse: c_int, state: c_int);

    fn bgfx_get_ui_funcs() -> *mut c_void;

    fn bgfx_imgui_begin(show: c_int);
    fn bgfx_imgui_end();

    //fn bgfx_imgui_set_window_pos(x: c_float, y: c_float);
    //fn bgfx_imgui_set_window_size(x: c_float, y: c_float);

    //fn bgfx_get_screen_width() -> f32;
    //fn bgfx_get_screen_height() -> f32;
}
