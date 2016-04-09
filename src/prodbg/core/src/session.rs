// use core::view_plugins::ViewPlugins;
//use view_plugins::{ViewInstance, ViewPlugins, ViewHandle};
//use core::backend_plugins::{BackendPlugins, BackendHandle};
//use core::view_plugins::ViewInstance;
//use prodbg_api::view::CViewCallbacks;
use prodbg_api::read_write::{Reader, Writer};
use plugins::PluginHandler;
use reader_wrapper::{ReaderWrapper, WriterWrapper};
//use imgui_sys::Imgui;
//use libc::c_void;

#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct SessionHandle(pub u64);

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
    pub handle: SessionHandle,
    //pub views: Vec<ViewHandle>,
    //backend: Option<BackendHandle>,

    pub reader: Reader,

    current_writer: usize,
    writers: [Writer; 2],
}

///! Connection options for Remote connections. Currently just one Ip adderss
///!
pub struct ConnectionSettings<'a> {
    pub address: &'a str,
}

impl Session {
    pub fn new(handle: SessionHandle) -> Session {
        Session {
            handle: handle,
            //views: Vec::new(),
            writers: [
                WriterWrapper::create_writer(),
                WriterWrapper::create_writer(),
            ],
            reader: ReaderWrapper::create_reader(),
            current_writer: 0,
            //backend: None,
        }
    }

    pub fn get_current_writer(&mut self) -> &mut Writer {
        &mut self.writers[self.current_writer]
    }

    pub fn start_remote(_plugin_handler: &PluginHandler, _settings: &ConnectionSettings) {}

    pub fn start_local(_: &str, _: usize) {}

    /*
    fn update_view_instance(reader: &Reader, writer: &mut Writer, view: &mut ViewInstance) {
        unsafe {
            //bgfx_imgui_set_window_pos(view.x, view.y);
            //bgfx_imgui_set_window_size(view.width, view.height);
            //bgfx_imgui_set_window_size(500.0, 500.0);

            // TODO: Fix visibility flag
            Imgui::begin_window("Test", true);
            //Imgui::init_state(ui.api);


            let plugin_funcs = view.plugin_type.plugin_funcs as *mut CViewCallbacks;
            ((*plugin_funcs).update.unwrap())(view.plugin_data,
                                              view.ui.api as *mut c_void,
                                              reader.api as *mut c_void,
                                              writer.api as *mut c_void);
            Imgui::end_window();
        }
    }

    pub fn add_view(&mut self, view: ViewHandle) {
        self.views.push(view);
    }
    */

    /*
    pub fn set_backend(&mut self, backend: Option<BackendHandle>) {
        self.backend = backend
    }
    */

    pub fn update(&mut self) {
        // swap the writers
        let p_writer = (self.current_writer + 1) & 1;
        //let c_writer = self.current_writer;
        self.current_writer = p_writer;

        ReaderWrapper::init_from_writer(&mut self.reader, &self.writers[p_writer]);

        //let mut writer = &mut self.writers[c_writer];

        // TODO: Update backend here

        /*
        for view in &self.views {
            if let Some(ref mut v) = view_plugins.get_view(*view) {
                Self::update_view_instance(&self.reader, writer, v);
            }
        }
        */
    }
}


///
/// Sessions handler
///
pub struct Sessions {
    instances: Vec<Session>,
    current: usize,
    session_counter: SessionHandle,
}

impl Sessions {
    pub fn new() -> Sessions {
        Sessions {
            instances: Vec::new(),
            current: 0,
            session_counter: SessionHandle(0),
        }
    }

    pub fn create_instance(&mut self) {
        let s = Session::new(self.session_counter);
        self.instances.push(s);
        self.session_counter.0 += 1;
    }

    pub fn update(&mut self) {
        for session in self.instances.iter_mut() {
            session.update();
        }
    }

    pub fn get_current(&mut self) -> &mut Session {
        let current = self.current;
        &mut self.instances[current]
    }

    pub fn get_session(&mut self, handle: SessionHandle) -> Option<&mut Session> {
        for i in 0..self.instances.len() {
            if self.instances[i].handle == handle {
                return Some(&mut self.instances[i]);
            }
        }

        None
    }
}

#[cfg(test)]
mod tests {
    use core::reader_wrapper::{ReaderWrapper};
    use super::*;

    #[test]
    fn create_session() {
        let _session = Session::new();
    }

    #[test]
    fn write_simple_event() {
        let mut session = Session::new();

        session.writers[0].event_begin(0x44);
        session.writers[0].event_end();

        ReaderWrapper::init_from_writer(&mut session.reader, &session.writers[0]);

        assert_eq!(session.reader.get_event().unwrap(), 0x44);
    }
}



