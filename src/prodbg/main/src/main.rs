extern crate core;
extern crate libc;
extern crate minifb;
extern crate prodbg_api;

pub mod windows;
mod docking;
pub mod session;
mod backend_plugin;

use docking::DockingPlugin;

use session::Sessions;
use windows::Windows;
use core::{DynamicReload, Search};
//use minifb::{Scale, WindowOptions, MouseMode, MouseButton, Menu};
//use libc::{c_void, c_int, c_float};
//use prodbg_api::view::CViewCallbacks;
use core::view_plugins::{ViewPlugins};
use std::rc::Rc;
use std::cell::RefCell;

use core::plugins::*;
//use std::ptr;

//const WIDTH: usize = 1280;
//const HEIGHT: usize = 1024;

//const MENU_CREATE_VIEW_0: usize = 2;
//const MENU_CREATE_VIEW_1: usize = 3;
//const MENU_CREATE_VIEW_2: usize = 4;

fn add_view(index: usize, sessions: &mut Sessions, windows: &mut Windows, view_plugins: &mut ViewPlugins) {
    let session = sessions.get_current();
    let window = windows.get_current();

    // TODO: Mask out index for plugin
    view_plugins.create_instance_from_index(index).map(|handle| {
        window.add_view(handle);
        session.add_view(handle);
    });
}

/*
   fn menu_press(id: usize, sessions: &mut Sessions, windows: &mut Windows, view_plugins: &ViewPlugins) {
   match id {
   MENU_CREATE_VIEW_0 => {
//add_view(0, sessions, windows, view_plugins);
}
_ => (),
}
}

fn create_menus() -> Vec<MenuInfo> {

}
*/

fn main() {
    let mut sessions = Sessions::new();
    let mut windows = Windows::new();

    let mut lib_handler = DynamicReload::new(None, Some("t2-output"), Search::Backwards);
    let mut plugins = Plugins::new();

    //let menus = create_menus();

    // Would be nice to nat have it this way
    let view_plugins = Rc::new(RefCell::new(ViewPlugins::new()));
    let docking_plugin = Rc::new(RefCell::new(DockingPlugin::new()));
    //let backend_plugins = Rc::new(RefCell::new(BackendPlugins::new()));

    sessions.create_instance();

    plugins.add_handler(&view_plugins);
    plugins.add_handler(&docking_plugin);
    //plugins.add_handler(&backend_plugins);

    plugins.add_plugin(&mut lib_handler, "bitmap_memory");
    plugins.add_plugin(&mut lib_handler, "i3_docking");
    //plugins.add_plugin(&mut lib_handler, "dummy_backend");


    // TODO: Wrap away this code.

    unsafe { bgfx_create(); }

    windows.create_default();

    add_view(0, &mut sessions, &mut windows, &mut view_plugins.borrow_mut());

    loop {
        unsafe { bgfx_pre_update(); }

        plugins.update(&mut lib_handler);
        sessions.update(&mut view_plugins.borrow_mut());
        windows.update();

        if windows.should_exit() {
            break;
        }

        unsafe { bgfx_post_update(); }
    }

    unsafe { bgfx_destroy(); }
}

///
///
///
///

extern "C" {
    fn bgfx_pre_update();
    fn bgfx_post_update();

    fn bgfx_create();
    fn bgfx_destroy();
}

