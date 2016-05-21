#[macro_use]
extern crate prodbg_api;

use prodbg_api::*;

struct DisassemblyView {
    dummy: i32,
}

impl View for DisassemblyView {
    fn new(_: &Ui, _: &Service) -> Self {
        DisassemblyView {
            dummy: 0
        }
    }

    fn update(&mut self, ui: &Ui, _: &mut Reader, _: &mut Writer) {
        if ui.button("sthteeoh", None) {
            println!("yah");
        }

        self.dummy += 1;
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Disassembly2 View", DisassemblyView);
    plugin_handler.register_view(&PLUGIN);
}
