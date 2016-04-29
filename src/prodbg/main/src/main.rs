extern crate core;
extern crate libc;
extern crate minifb;
extern crate prodbg_api;
extern crate bgfx_rs;
extern crate imgui_sys;

pub mod windows;
pub mod menu;

use bgfx_rs::Bgfx;
use core::session::Sessions;
use windows::Windows;
use core::{DynamicReload, Search};
use core::view_plugins::{ViewPlugins};
use core::backend_plugin::{BackendPlugins};
use std::cell::RefCell;
use std::rc::Rc;

use core::plugins::*;

fn main() {
    let bgfx = Bgfx::new();
    let mut sessions = Sessions::new();
    let mut windows = Windows::new();

    let mut lib_handler = DynamicReload::new(None, Some("t2-output"), Search::Backwards);
    let mut plugins = Plugins::new();

    let view_plugins = Rc::new(RefCell::new(ViewPlugins::new()));
    let backend_plugins = Rc::new(RefCell::new(BackendPlugins::new()));

    let session = sessions.create_instance();

    plugins.add_handler(&view_plugins);
    plugins.add_handler(&backend_plugins);

    plugins.add_plugin(&mut lib_handler, "dummy_backend_plugin");
    plugins.add_plugin(&mut lib_handler, "bitmap_memory");
    plugins.add_plugin(&mut lib_handler, "registers_plugin");
    plugins.add_plugin(&mut lib_handler, "hex_memory_plugin");

    windows.create_default();

    // test code, we set dummy backend as active

    if let Some(backend) = backend_plugins.borrow_mut().create_instance(&"Dummy Backend".to_owned()) {
        if let Some(session) = sessions.get_session(session) {
            session.set_backend(Some(backend));
        }
    }

    loop {
        bgfx.pre_update();

        plugins.update(&mut lib_handler);
        sessions.update(&mut backend_plugins.borrow_mut());
        windows.update(&mut sessions, &mut view_plugins.borrow_mut());

        if windows.should_exit() {
            break;
        }

        bgfx.post_update();
    }
}

// dummy
#[no_mangle]
pub fn init_plugin() {
}
