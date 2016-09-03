#[macro_use]
extern crate prodbg_api;

use prodbg_api::{View, Ui, Reader, Writer, PluginHandler, Service, CViewCallbacks};

use prodbg_api::events::*;


struct LocalsView {
    _dummy: u8
}

impl LocalsView {
     fn show_in_ui(&mut self, reader: &mut Reader, ui: &mut Ui) {
         ui.text("");
         
         ui.columns(3, Some("callstack"), true);
         ui.text("Name"); ui.next_column();
         ui.text("Value"); ui.next_column();
         ui.text("Type"); ui.next_column();

         for entry in reader.find_array("threads").unwrap() {
             let _name = entry.find_string("name").unwrap();
             let _value = entry.find_string("value").unwrap();
             let _type_cell = entry.find_string("type").unwrap();
         }
     }
}

impl View for LocalsView {
    fn new(_: &Ui, _: &Service) -> Self {
        LocalsView {
            _dummy: 0u8
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, _writer: &mut Writer) {

        for event in reader.get_events() {
            match event {
                EVENT_SET_LOCALS => {
                    self.show_in_ui(reader, ui)
                }
                
                _ => {},
            }
        }

        // Request callstack data
        // TODO: Dont' request locals all the time

        //writer.event_begin(EVENT_GET_LOCALS as u16);
        //writer.event_end();
    }
}


#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Locals\0", LocalsView);
    plugin_handler.register_view(&PLUGIN);
}

