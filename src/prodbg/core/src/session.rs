use prodbg_api::read_write::{Reader, Writer};
use prodbg_api::backend::{CBackendCallbacks};
use plugins::PluginHandler;
use reader_wrapper::{ReaderWrapper, WriterWrapper};
use backend_plugin::{BackendHandle, BackendPlugins};
use std::os::raw::{c_void};
use prodbg_api::events::*;

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
    pub backend: Option<BackendHandle>,
    pub handle: SessionHandle,
    pub reader: Reader,

    current_writer: usize,
    writers: [Writer; 2],
    action: i32,
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
            writers: [
                WriterWrapper::create_writer(),
                WriterWrapper::create_writer(),
            ],
            reader: ReaderWrapper::create_reader(),
            action: 0,
            current_writer: 0,
            backend: None,
        }
    }

    pub fn get_current_writer(&mut self) -> &mut Writer {
        &mut self.writers[self.current_writer]
    }

    pub fn start_remote(_plugin_handler: &PluginHandler, _settings: &ConnectionSettings) {}

    pub fn start_local(_: &str, _: usize) {}

    pub fn set_backend(&mut self, backend: Option<BackendHandle>) {
        self.backend = backend
    }

    pub fn action_step(&mut self) {
        println!("do step");
        self.action = ACTION_STEP;
    }

    pub fn action_run(&mut self) {
        println!("do run");
        self.action = ACTION_RUN;
    }

    pub fn action_step_over(&mut self) {
        println!("do step over");
        self.action = ACTION_STEP_OVER;
    }

    pub fn action_break(&mut self) {
        println!("do break");
        self.action = ACTION_BREAK;
    }

    pub fn send_menu_id(&mut self, menu_id: u32, backend_plugins: &mut BackendPlugins) {
        if let Some(backend) = backend_plugins.get_backend(self.backend) {
            if menu_id >= backend.menu_id_offset && (menu_id < backend.menu_id_offset + 1000) {
                let writer = self.get_current_writer();
                //println!("Write event, writer {}", );
                writer.event_begin(35); // Menu event, TODO: Fix hard-coded value
                writer.write_u32("menu_id", menu_id - backend.menu_id_offset);
                writer.event_end();
            }
        }
    }

    // The way this code works is to allow the view plugins to have "two rounds" of updates.
    // That is to allow the view plugins to send things that other view plugins can listen
    // to and not only get data from the backend.
    pub fn update(&mut self, backend_plugins: &mut BackendPlugins) {
        // swap the writers
        let c_writer = self.current_writer;
        let n_writer = (self.current_writer + 1) & 1;

        ReaderWrapper::init_from_writer(&mut self.reader, &self.writers[c_writer]);
        ReaderWrapper::reset_writer(&mut self.writers[n_writer]);

        if let Some(backend) = backend_plugins.get_backend(self.backend) {
            unsafe {
                let plugin_funcs = backend.plugin_type.plugin_funcs as *mut CBackendCallbacks;
                ((*plugin_funcs).update.unwrap())(backend.plugin_data,
                                                  self.action,
                                                  self.reader.api as *mut c_void,
                                                  self.writers[n_writer].api as *mut c_void);
            }
        }

        self.action = 0;
        self.current_writer = n_writer;

        //ReaderWrapper::init_from_writer(&mut self.reader, &self.writers[1]);
        //ReaderWrapper::reset_writer(&mut self.writers[0]);
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

    pub fn create_instance(&mut self) -> SessionHandle {
        let s = Session::new(self.session_counter);
        let handle = s.handle;
        self.instances.push(s);
        self.session_counter.0 += 1;
        handle
    }

    pub fn update(&mut self, backend_plugins: &mut BackendPlugins) {
        for session in self.instances.iter_mut() {
            session.update(backend_plugins);
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
    //use core::reader_wrapper::{ReaderWrapper};
    //use super::*;

    #[test]
    fn create_session() {
        //let _session = Session::new();
    }

    #[test]
    fn write_simple_event() {
        /*
        let mut session = Session::new();

        session.writers[0].event_begin(0x44);
        session.writers[0].event_end();

        ReaderWrapper::init_from_writer(&mut session.reader, &session.writers[0]);

        assert_eq!(session.reader.get_event().unwrap(), 0x44);
        */
    }
}



