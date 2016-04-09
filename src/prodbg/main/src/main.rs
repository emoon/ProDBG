extern crate core;
extern crate libc;
extern crate minifb;
extern crate prodbg_api;
extern crate bgfx_rs;
extern crate imgui_sys;

pub mod windows;
mod docking;
pub mod session;
mod backend_plugin;

use docking::DockingPlugin;
//use prodbg_api::ui::Ui;
use bgfx_rs::Bgfx;
//use imgui_sys::Imgui;

use session::Sessions;
use windows::Windows;
use core::{DynamicReload, Search};
use core::view_plugins::{ViewPlugins};
use std::cell::RefCell;
use std::rc::Rc;

use core::plugins::*;

/*
fn add_view(index: usize, sessions: &mut Sessions, windows: &mut Windows, view_plugins: &mut ViewPlugins) {
    let session = sessions.get_current();
    let window = windows.get_current();
    let ui = Imgui::create_ui_instance();

    // TODO: Mask out index for plugin
    view_plugins.create_instance_from_index(ui, index).map(|handle| {
        window.add_view(handle);
        session.add_view(handle);
    });
}
*/

fn main() {
    let bgfx = Bgfx::new();
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

    windows.create_default();

    //add_view(0, &mut sessions, &mut windows, &mut view_plugins.borrow_mut());

    loop {
        bgfx.pre_update();

        plugins.update(&mut lib_handler);
        sessions.update(&mut view_plugins.borrow_mut());
        windows.update(&mut view_plugins.borrow_mut());

        if windows.should_exit() {
            break;
        }

        bgfx.post_update();
    }
}
