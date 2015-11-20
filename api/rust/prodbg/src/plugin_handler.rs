use libc::*;
use backend::*;
use std::mem::transmute;

pub struct PluginHandler {
    pub private_data: *mut c_void,
    pub c_register_plugin: extern "C" fn(plugin: *mut c_void, priv_data: *mut c_void),
}

impl PluginHandler {
    pub fn register(&self, plugin: &mut CBackendCallbacks) {
        unsafe {
            (self.c_register_plugin)(transmute(plugin), (self.private_data));
        }
    }
}

extern "C" fn init_plugin(_: &mut PluginHandler) {}

#[no_mangle]
#[allow(non_snake_case)]
pub extern "C" fn InitPlugin(cb: extern "C" fn(plugin: *mut c_void, data: *mut c_void),
                             priv_data: *mut c_void) {
    let mut plugin_handler = PluginHandler {
        private_data: priv_data,
        c_register_plugin: cb,
    };

    init_plugin(&mut plugin_handler);
}
