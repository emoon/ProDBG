use service::*;
use read_write::*;
use ui::*;
use ui_ffi::*;
use libc::{c_void, c_uchar};
use std::mem::transmute;
use io::{CPDSaveState, CPDLoadState};

pub static VIEW_API_VERSION: &'static [u8] = b"ProDBG View 1\0";

pub trait View {
    fn new(ui: &Ui, service: &Service) -> Self;
    fn update(&mut self, ui: &Ui, reader: &mut Reader, writer: &mut Writer);
}

#[repr(C)]
pub struct CViewCallbacks {
    pub name: *const c_uchar,
    pub create_instance: Option<fn(ui_api: *const c_void,
                                   service_func: extern "C" fn(service: *const c_uchar)
                                                               -> *mut c_void)
                                   -> *mut c_void>,
    pub destroy_instance: Option<fn(*mut c_void)>,
    pub update: Option<fn(ptr: *mut c_void,
                          ui: *mut c_void,
                          reader: *mut c_void,
                          writer: *mut c_void)>,

    pub save_state: Option<fn(*mut c_void, api: *mut CPDSaveState)>,
    pub load_state: Option<fn(*mut c_void, api: *mut CPDLoadState)>,
}

unsafe impl Sync for CViewCallbacks {}

pub fn create_view_instance<T: View>(ui_api: *const c_void,
                                     service_func: extern "C" fn(service: *const c_uchar)
                                                                 -> *mut c_void)
                                     -> *mut c_void {
    let c_ui: &mut CPdUI = unsafe { &mut *(ui_api as *mut CPdUI) };
    let ui = Ui { api: c_ui };
    let service = Service { service_func: service_func };
    let instance = unsafe { transmute(Box::new(T::new(&ui, &service))) };
    instance
}

pub fn destroy_view_instance<T: View>(ptr: *mut c_void) {
    println!("rust: backend: destroy");
    let _: Box<T> = unsafe { transmute(ptr) };
    // implicitly dropped
}

pub fn update_view_instance<T: View>(ptr: *mut c_void,
                                        ui_api: *mut c_void,
                                        reader_api: *mut c_void,
                                        writer_api: *mut c_void) {
    let view: &mut T = unsafe { &mut *(ptr as *mut T) };
    let c_ui: &mut CPdUI = unsafe { &mut *(ui_api as *mut CPdUI) };
    let c_reader: &mut CPDReaderAPI = unsafe { &mut *(reader_api as *mut CPDReaderAPI) };
    let c_writer: &mut CPDWriterAPI = unsafe { &mut *(writer_api as *mut CPDWriterAPI) };
    let mut reader = Reader {
        api: c_reader,
        it: 0,
    };

    let mut writer = Writer { api: c_writer };
    let ui = Ui { api: c_ui };

    view.update(&ui, &mut reader, &mut writer);
}

#[macro_export]
macro_rules! define_view_plugin {
    ($p_name:ident, $name:expr, $x:ty) => {
        static $p_name: CViewCallbacks = CViewCallbacks {
                name: $name as *const u8,
                create_instance: Some(prodbg_api::view::create_view_instance::<$x>),
                destroy_instance: Some(prodbg_api::view::destroy_view_instance::<$x>),
                update: Some(prodbg_api::view::update_view_instance::<$x>),
                save_state: None,
                load_state: None
        };
    }
}
