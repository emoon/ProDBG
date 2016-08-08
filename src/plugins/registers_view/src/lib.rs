#[macro_use]
extern crate prodbg_api;

use prodbg_api::{View, Ui, Service, Reader, Writer, PluginHandler, CViewCallbacks};


struct RegistersView;

impl View for RegistersView {
    fn new(_: &Ui, _: &Service) -> Self {
        RegistersView {}
    }

    fn update(&mut self, _: &mut Ui, _: &mut Reader, _: &mut Writer) {
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Registers View 2\0", RegistersView);
    plugin_handler.register_view(&PLUGIN);
}
