use prodbg_api::read_write::{Reader, Writer};
use prodbg_api::backend::{CBackendCallbacks};
use plugins::PluginHandler;
use reader_wrapper::{ReaderWrapper, WriterWrapper};
use backend_plugin::{BackendHandle, BackendPlugins};
use libc::{c_void};

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
    pub reader: Reader,

    current_writer: usize,
    writers: [Writer; 2],

    backend: Option<BackendHandle>,
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

    pub fn update(&mut self, backend_plugins: &mut BackendPlugins) {
        // swap the writers
        let c_writer = self.current_writer;
        let p_writer = (self.current_writer + 1) & 1;
        self.current_writer = p_writer;

        ReaderWrapper::init_from_writer(&mut self.reader, &self.writers[p_writer]);
        ReaderWrapper::reset_writer(&mut self.writers[c_writer]);

        if let Some(backend) = backend_plugins.get_backend(self.backend) {
            unsafe {
                let plugin_funcs = backend.plugin_type.plugin_funcs as *mut CBackendCallbacks;
                ((*plugin_funcs).update.unwrap())(backend.plugin_data,
                                                  0,
                                                  self.reader.api as *mut c_void,
                                                  self.writers[p_writer].api as *mut c_void);
            }
        }

        ReaderWrapper::init_from_writer(&mut self.reader, &self.writers[p_writer]);
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



