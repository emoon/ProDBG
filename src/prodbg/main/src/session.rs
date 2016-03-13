// use core::view_plugins::ViewPlugins;
use core::view_plugins::{ViewInstance, ViewPlugins, ViewHandle};
//use core::backend_plugins::{BackendPlugins, BackendHandle};
//use core::view_plugins::ViewInstance;
use prodbg_api::view::CViewCallbacks;
use core::plugins::PluginHandler;
//use core::plugin::Plugin;
//use std::rc::Rc;
use std::ptr;
use libc::{c_void, c_int};

///! Session is a major part of ProDBG. There can be several sessions active at the same time
///! and each session has exactly one backend. There are only communication internally in a session
///! sessions can't (at least now) not talk to eachother.
///!
///! A backend can have several views at the same time. Data can be sent between the backend and
///| views using the PDReader/PDWriter APIs (prodbg::Writer prodbg::Reader in Rust) and this is the
///| only way for views and backends to talk with each other. There are several reasons for this
///| approach:
///!
///| 1. No "hacks" on trying to share memory. Plugins can be over a socket/webview/etc.
///! 2. Views and backends makes no assumetions on the inner workings of the others.
///! 3. Backends and views can post messages which anyone can decide to (optionally) act on.
///!
pub struct Session {
    views: Vec<ViewHandle>,
    //backend: Option<BackendHandle>,
}

///! Connection options for Remote connections. Currently just one Ip adderss
///!
pub struct ConnectionSettings<'a> {
    pub address: &'a str,
}

impl Session {
    pub fn new() -> Session {
        Session {
            //backend: None,
            views: Vec::new(),
        }
    }

    pub fn start_remote(_plugin_handler: &PluginHandler, _settings: &ConnectionSettings) {}

    pub fn start_local(_: &str, _: usize) {}

    fn update_view_instance(view: &mut ViewInstance) {
        unsafe {
            //bgfx_imgui_set_window_pos(view.x, view.y);
            //bgfx_imgui_set_window_size(view.width, view.height); 
            //bgfx_imgui_set_window_size(500.0, 500.0); 

            // TODO: Fix visibility flag
            bgfx_imgui_begin(1);

            let plugin_funcs = view.plugin_type.plugin_funcs as *mut CViewCallbacks;
            ((*plugin_funcs).update.unwrap())(view.user_data,
                                              bgfx_get_ui_funcs(),
                                              // Send in reader/writer
                                              ptr::null_mut(),
                                              ptr::null_mut());
            bgfx_imgui_end();
        }
    }

    pub fn add_view(&mut self, view: ViewHandle) {
        self.views.push(view);
    }

    /*
    pub fn set_backend(&mut self, backend: Option<BackendHandle>) {
        self.backend = backend 
    }
    */

    pub fn update(&mut self, view_plugins: &mut ViewPlugins) {

        // TODO: Reader/Write setup + backend update

        /*
        unsafe { 
            bgfx_pre_update(); 
        }
        */

        for view in &self.views {
            if let Some(ref mut v) = view_plugins.get_view(*view) {
                Self::update_view_instance(v);
            }
        }

        //unsafe { bgfx_post_update(); }
    }
}


///
/// Sessions handler
///
pub struct Sessions {
    instances: Vec<Session>,
    current: usize,
    
}

impl Sessions {
    pub fn new() -> Sessions {
        Sessions {
            instances: Vec::new(),
            current: 0,
        }
    }

    pub fn create_instance(&mut self) {
        let s = Session { views: Vec::new() };
        self.instances.push(s)
    }

    pub fn update(&mut self, view_plugins: &mut ViewPlugins) {
        for session in self.instances.iter_mut() {
            session.update(view_plugins);
        }
    }

    pub fn get_current(&mut self) -> &mut Session {
        let current = self.current;
        &mut self.instances[current]
    }
}

///
///
///
///

extern "C" {
    //fn bgfx_pre_update();
    //fn bgfx_post_update();

    fn bgfx_get_ui_funcs() -> *mut c_void;

    fn bgfx_imgui_begin(show: c_int);
    fn bgfx_imgui_end();

    //fn bgfx_imgui_set_window_pos(x: c_float, y: c_float);
    //fn bgfx_imgui_set_window_size(x: c_float, y: c_float);

    //fn bgfx_get_screen_width() -> f32;
    //fn bgfx_get_screen_height() -> f32;
}

#[cfg(test)]
mod tests {
    #[test]
    fn test_search_paths_none() {
        assert_eq!(1, 1);
    }
}



