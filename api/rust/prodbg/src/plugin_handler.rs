use libc::*;
use backend::*;
use view::*;
use std::mem::transmute;

#[repr(C)]
pub struct PluginHandler {
    pub private_data: *mut c_void,
    pub c_register_plugin: extern "C" fn(pt: *const c_char,
                                         plugin: *mut c_void,
                                         priv_data: *mut c_void)
}

impl PluginHandler {
    pub fn register_backend(&self, plugin: &CBackendCallbacks) {
        unsafe {
            (self.c_register_plugin)(BACKEND_API_VERSION.as_ptr() as *const i8,
                                     transmute(plugin),
                                     (self.private_data));
        }
    }

    pub fn register_view(&self, plugin: &CViewCallbacks) {
        unsafe {
            (self.c_register_plugin)(VIEW_API_VERSION.as_ptr() as *const i8,
                                     transmute(plugin),
                                     (self.private_data));
        }
    }
}

// So this is kinda ugly as this is a Rust function but we handle it as a C function because extern
// "Rust" isn't implemented yet
extern "C" {
    fn init_plugin(_: &mut PluginHandler);
}

#[no_mangle]
#[allow(non_snake_case)]
pub extern "C" fn InitPlugin(cb: extern "C" fn(pt: *const c_char,
                                               plugin: *mut c_void,
                                               data: *mut c_void),
                             priv_data: *mut c_void) {
    let mut plugin_handler = PluginHandler {
        private_data: priv_data,
        c_register_plugin: cb,
    };

    unsafe {
        init_plugin(&mut plugin_handler);
    }
}
