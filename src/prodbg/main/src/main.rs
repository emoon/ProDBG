extern crate core;
extern crate minifb;
extern crate prodbg_api;
extern crate bgfx;
extern crate imgui_sys;
extern crate settings;
extern crate renderer;

pub mod windows;
pub mod menu;
pub mod statusbar;

//use renderer::Renderer;
use core::session::Sessions;
use windows::Windows;
use settings::Settings;
use core::{DynamicReload, Search};
use core::view_plugins::{ViewPlugins};
use core::backend_plugin::{BackendPlugins};
use std::cell::RefCell;
use std::rc::Rc;
use std::time::Duration;

use core::plugins::*;

fn main() {
    let mut sessions = Sessions::new();
    let mut windows = Windows::new();
    let mut settings = Settings::new();

    let mut lib_handler = DynamicReload::new(None, Some("t2-output"), Search::Backwards);
    let mut plugins = Plugins::new();

    let view_plugins = Rc::new(RefCell::new(ViewPlugins::new()));
    let backend_plugins = Rc::new(RefCell::new(BackendPlugins::new()));

    let session = sessions.create_instance();

    match settings.load_default_settings("data/settings.json") {
        Err(e) => println!("Unable to load data/settings: {}", e),
        _ => (),
    }

    plugins.add_handler(&view_plugins);
    plugins.add_handler(&backend_plugins);
    plugins.search_load_plugins(&mut lib_handler);

    if let Some(backend) = backend_plugins.borrow_mut().create_instance(&"Dummy Backend".to_owned()) {
        if let Some(session) = sessions.get_session(session) {
            session.set_backend(Some(backend));
            println!("set backend");
        }
    }

    windows.create_default(&settings);
    windows.load("data/user_layout.json", &mut view_plugins.borrow_mut());

    loop {
        plugins.update(&mut lib_handler);
        sessions.update(&mut backend_plugins.borrow_mut());
        windows.update(&mut sessions,
                       &mut view_plugins.borrow_mut(),
                       &mut backend_plugins.borrow_mut());

        if windows.should_exit() {
            break;
        }

        // TODO: Proper config and sleep timings
        std::thread::sleep(Duration::from_millis(5));
    }

    //windows.save("data/user_layout.json", &mut view_plugins.borrow_mut());
}

// dummy
#[no_mangle]
pub fn init_plugin() {
}
