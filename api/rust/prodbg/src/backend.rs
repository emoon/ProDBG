use read_write::*;
use service::*;
use std::os::raw::{c_uchar, c_int, c_void};
use std::mem::transmute;
use io::{CPDSaveState, StateSaver, CPDLoadState, StateLoader};

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

    UpdateMemory,
    UpdateRegister,
    UpdatePc,

    // End of events
    End,

    /// Custom events. Here you can have your own events. Note that they must start with PDEventType_custom and up
    Custom = 0x1000,
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#[repr(C)]
#[derive(PartialEq, Clone, Copy)]
pub enum DebugState {
    NoTarget,
    Running,
    StopBreakpoint,
    StopException,
    Trace,
}

pub static BACKEND_API_VERSION: &'static [u8] = b"ProDBG Backend 1\0";

pub trait Backend {
    fn new(service: &Service) -> Self;
    fn destroy_instance(&mut self) { }
    fn update(&mut self, action: i32, reader: &mut Reader, writer: &mut Writer) -> DebugState;
    fn save_state(&mut self, _: StateSaver) {}
    fn load_state(&mut self, _: StateLoader) {}
}

pub type ServiceFunc = extern "C" fn(service: *const c_uchar) -> *mut c_void;

#[repr(C)]
pub struct CBackendCallbacks {
    pub name: *const c_uchar,
    pub create_instance: Option<fn(service_func: ServiceFunc) -> *mut c_void>,
    pub destroy_instance: Option<fn(*mut c_void)>,
    pub update: Option<fn(ptr: *mut c_void, a: c_int, ra: *mut c_void, wa: *mut c_void) -> DebugState>,
    pub save_state: Option<fn(*mut c_void, api: *mut CPDSaveState)>,
    pub load_state: Option<fn(*mut c_void, api: *mut CPDLoadState)>,
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
    let backend: &mut T = unsafe { &mut *(ptr as *mut T) };
    backend.destroy_instance();
    let _: Box<T> = unsafe { transmute(ptr) };
    // implicitly dropped
}

pub fn update_backend_instance<T: Backend>(ptr: *mut c_void,
                                           action: c_int,
                                           reader_api: *mut c_void,
                                           writer_api: *mut c_void) -> DebugState {
    let backend: &mut T = unsafe { &mut *(ptr as *mut T) };
    let c_reader: &mut CPDReaderAPI = unsafe { &mut *(reader_api as *mut CPDReaderAPI) };
    let c_writer: &mut CPDWriterAPI = unsafe { &mut *(writer_api as *mut CPDWriterAPI) };
    let mut reader = Reader {
        api: c_reader,
        it: 0,
    };

    let mut writer = Writer { api: c_writer };

    backend.update(action as i32, &mut reader, &mut writer)
}

pub fn save_backend_state<T: Backend>(ptr: *mut c_void, saver_api: *mut CPDSaveState) {
    let view: &mut T = unsafe { &mut *(ptr as *mut T) };
    let saver = StateSaver::new(saver_api);
    view.save_state(saver);
}

pub fn load_backend_state<T: Backend>(ptr: *mut c_void, loader_api: *mut CPDLoadState) {
    let view: &mut T = unsafe { &mut *(ptr as *mut T) };
    let loader = StateLoader::new(loader_api);
    view.load_state(loader);
}

#[macro_export]
macro_rules! define_backend_plugin {
    ($p_name:ident, $name:expr, $x:ty) => {
        static $p_name: CBackendCallbacks = CBackendCallbacks {
            name: $name as *const u8,
            create_instance: Some(prodbg_api::backend::create_backend_instance::<$x>),
            destroy_instance: Some(prodbg_api::backend::destroy_backend_instance::<$x>),
            update: Some(prodbg_api::backend::update_backend_instance::<$x>),
			save_state: Some(prodbg_api::backend::save_backend_state::<$x>),
			load_state: Some(prodbg_api::backend::load_backend_state::<$x>)
        };
    }
}
