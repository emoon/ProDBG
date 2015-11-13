use read_write::*; 
use libc::*;
use std::mem::transmute;

pub trait Backend {
    fn new() -> Self;
    fn update(&mut self, reader: &Reader, writer: &Writer);
}

#[repr(C)]
pub struct CBackendCallbacks {
    pub create_instance: fn() -> *mut c_void, 
    pub destroy_instance: fn(*mut c_void), 
    pub update: fn(*mut c_void), 
}

pub fn create_backend_instance<T: Backend>() -> *mut c_void {
    let instance = unsafe { transmute(Box::new(T::new())) };
    println!("Lets create instance!");
    instance
}

pub fn destroy_backend_instance<T: Backend>(ptr: *mut c_void) {
    let _: Box<T> = unsafe{ transmute(ptr) };
    // implicitly dropped
}

pub fn update_backend_instance<T: Backend>(ptr: *mut c_void, reader_api: *mut c_void, writer_api: *mut c_void) { 
    let backend: &mut T = unsafe { &mut *(ptr as *mut T) };
    let c_reader: &mut CPDReaderAPI = unsafe { &mut *(reader_api as *mut CPDReaderAPI) };
    let c_writer: &mut CPDWriterAPI  = unsafe { &mut *(writer_api  as *mut CPDWriterAPI) };
    let reader = Reader { api: c_reader, it: 0 }; 
    let writer = Writer { api: c_writer }; 

    backend.update(&reader, &writer);
}

macro_rules! define_backend_plugin {
    ($x:ty) => {
        {
            let mut plugin = CBackendCallbacks { 
                create_instance: create_backend_instance::<$x>, 
                destroy_instance: destroy_backend_instance::<$x>, 
                update: update_backend_instance::<$x> 
             };

            plugin
        }
    }
}


