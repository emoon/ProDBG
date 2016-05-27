use read_write::*;
use service::*;
use libc::*;
//use std::os::raw;
use menu_service::{MenuFuncs, CMenuFuncs1};
use std::mem::transmute;


#[repr(C)]
pub enum EventType {
    No,
    GetLocals,
    SetLocals,
    GetCallstack,
    SetCallstack,
    GetWatch,
    SetWatch,
    GetRegisters,
    SetRegisters,
    GetMemory,
    SetMemory,
    GetTty,
    SetTty,
    GetExceptionLocation,
    SetExceptionLocation,
    GetDisassembly,
    SetDisassembly,
    GetStatus,
    SetStatus,
    SetThreads,
    GetThreads,
    SelectThread,
    SelectFrame,
    GetSourceFiles,
    SetSourceFiles,

    SetSourceCodeFile,

    // setbreakpoint send a breakpoint to the backend with supplied id
    // Back end will reply if this worked correct with supplied ID

    SetBreakpoint,
    ReplyBreakpoint,

    DeleteBreakpoint,
    SetExecutable,
    Action,
    AttachToProcess,
    AttachToRemoteSession,

    ExecuteConsole,
    GetConsole,

	MenuEvent,

    // TODO: Somewhat temporary, need to figure this out

    ToggleBreakpointCurrentLine,

    // End of events

    End,

    /// Custom events. Here you can have your own events. Note that they must start with PDEventType_custom and up
    Custom = 0x1000
}

pub static BACKEND_API_VERSION: &'static [u8] = b"ProDBG Backend 1\0";

pub trait Backend {
    fn new(service: &Service) -> Self;
    fn update(&mut self, action: i32, reader: &mut Reader, writer: &mut Writer);
    fn register_menu(&mut self, menu_funcs: &mut MenuFuncs) -> *mut c_void;
}

pub type ServiceFunc = extern "C" fn(service: *const c_uchar) -> *mut c_void;

#[repr(C)]
pub struct CBackendCallbacks {
    pub name: *const c_uchar,
    pub create_instance: Option<fn(service_func: ServiceFunc) -> *mut c_void>,
    pub destroy_instance: Option<fn(*mut c_void)>,
    pub register_menu: Option<fn(ptr: *mut c_void, menu_funcs: *mut c_void) -> *mut c_void>,
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
                                           action: c_int,
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

pub fn register_backend_menu<T: Backend>(ptr: *mut c_void,
                                         menu_api: *mut c_void) -> *mut c_void {
    let backend: &mut T = unsafe { &mut *(ptr as *mut T) };

    let mut menu_funcs = MenuFuncs {
        api: menu_api as *mut CMenuFuncs1,
    };

    backend.register_menu(&mut menu_funcs)
}


#[macro_export]
macro_rules! define_backend_plugin {
    ($p_name:ident, $name:expr, $x:ty) => {
        static $p_name: CBackendCallbacks = CBackendCallbacks {
            name: $name as *const u8,
            create_instance: Some(prodbg_api::backend::create_backend_instance::<$x>),
            destroy_instance: Some(prodbg_api::backend::destroy_backend_instance::<$x>),
            register_menu: Some(prodbg_api::backend::register_backend_menu::<$x>),
            update: Some(prodbg_api::backend::update_backend_instance::<$x>)
        };
    }
}
