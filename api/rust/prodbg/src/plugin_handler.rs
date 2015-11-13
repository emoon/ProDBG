pub struct PluginHandler {
    pub private_data: *mut c_void,
    pub c_register_plugin: extern fn(plugin: *mut c_void, priv_data: *mut c_void),
}

impl PluginHandler {
    fn register_plugin(&self, plugin: &mut CBackendCallbacks) {
        unsafe {
            (self.c_register_plugin)(transmute(plugin), (self.private_data));
        }
    }
}

extern fn init_plugin(plugin_handler: &mut prodbg::PluginHandler) {}

#[no_mangle]
pub extern fn InitPlugin(cb: extern fn(plugin: *mut c_void, data: *mut c_void), priv_data: *mut c_void) {
    let mut plugin_handler = prodbg::PluginHandler { 
        private_data : priv_data, 
        c_register_plugin : cb
    };

    init_plugin(&mut plugin_handler);
}

