use read_write::*;
use service::*;
use libc::*;
use std::mem::transmute;

pub static BACKEND_API_VERSION: &'static [u8] = b"ProDBG Backend 1\0";

pub trait Backend {
    fn new(service: &Service) -> Self;
    fn update(&mut self, action: i32, reader: &mut Reader, writer: &mut Writer);
}

#[repr(C)]
pub struct CBackendCallbacks {
    pub name: *const c_uchar,
    pub create_instance: Option<fn(service_func: extern "C" fn(service: *const c_uchar)
                                                               -> *mut c_void)
                                   -> *mut c_void>,
    pub destroy_instance: Option<fn(*mut c_void)>,
    pub register_menu: Option<fn() -> *mut c_void>,
    pub update: Option<fn(ptr: *mut c_void,
                          a: c_int,
                          ra: *mut c_void,
                          wa: *mut c_void)>,
}

unsafe impl Sync for CBackendCallbacks {}

pub fn create_backend_instance<T: Backend>(service_func: extern "C" fn(service: *const c_uchar)
                                                                       -> *mut c_void)
                                           -> *mut c_void {
    let service = Service { service_func: service_func };
    let instance = unsafe { transmute(Box::new(T::new(&service))) };
    println!("Lets create instance!");
    instance
}

pub fn destroy_backend_instance<T: Backend>(ptr: *mut c_void) {
    println!("rust: backend: destroy");
    let _: Box<T> = unsafe { transmute(ptr) };
    // implicitly dropped
}

pub fn update_backend_instance<T: Backend>(ptr: *mut c_void,
                                           action: *mut c_int,
                                           reader_api: *mut c_void,
                                           writer_api: *mut c_void) {
    let backend: &mut T = unsafe { &mut *(ptr as *mut T) };
    let c_reader: &mut CPDReaderAPI = unsafe { &mut *(reader_api as *mut CPDReaderAPI) };
    let c_writer: &mut CPDWriterAPI = unsafe { &mut *(writer_api as *mut CPDWriterAPI) };
    let mut reader = Reader {
        api: c_reader,
        it: 0,
    };

    let mut writer = Writer { api: c_writer };

    backend.update(action as i32, &mut reader, &mut writer);
}

#[macro_export]
macro_rules! define_backend_plugin {
    ($p_name:ident, $name:expr, $x:ty) => {
        static $p_name: CBackendCallbacks = CBackendCallbacks {
            name: $name as *const u8,
            create_instance: Some(prodbg::backend::create_backend_instance::<$x>),
            destroy_instance: Some(prodbg::backend::destroy_backend_instance::<$x>),
            register_menu: None,
            update: Some(prodbg::backend::update_backend_instance::<$x>)
        };
    }
}
