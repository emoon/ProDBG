use libc::*;
use backend::*;
use std::mem::transmute;
use std::ffi::CString;

#[repr(C)]
pub struct PluginHandler {
    pub private_data: *mut c_void,
    pub c_register_plugin: extern "C" fn(pt: *const c_char, plugin: *mut c_void, priv_data: *mut c_void),
}

impl PluginHandler {
    pub fn register(&self, plugin_type: &str, plugin: &mut CBackendCallbacks) {
        unsafe {
            let s = CString::new(plugin_type).unwrap();
            (self.c_register_plugin)(s.as_ptr(), transmute(plugin), (self.private_data));
        }
    }
}

// So this is kinda ugly as this is a Rust function but we handle it as a C function because extern
// "Rust" isn't implemented yet
extern {
    fn init_plugin(_: &mut PluginHandler);
}

#[no_mangle]
#[allow(non_snake_case)]
pub extern "C" fn InitPlugin(cb: extern "C" fn(pt: *const c_char, plugin: *mut c_void, data: *mut c_void),
                             priv_data: *mut c_void) {
    let mut plugin_handler = PluginHandler {
        private_data: priv_data,
        c_register_plugin: cb,
    };

    println!("Rust: InitPlugin");

    unsafe {
        init_plugin(&mut plugin_handler);
    }
}
