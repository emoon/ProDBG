#[macro_use]
extern crate prodbg_api;

use prodbg_api::{View, Ui, Reader, Writer, PluginHandler, Service, CViewCallbacks, Vec2};

use prodbg_api::ui_ffi::PDUISELECTABLEFLAGS__SPANALLCOLUMNS;

use prodbg_api::events::*;


struct ThreadsView {
    selected_thread: u32,
    set_selected_thread: bool,
    thread_id: u32,
    request_data: bool
}

impl ThreadsView {
     fn show_in_ui(&mut self, reader: &mut Reader, ui: &mut Ui) {
         ui.text("");

         ui.columns(3, Some("threads"), true);
         ui.text("Id"); ui.next_column();
         ui.text("Name"); ui.next_column();
         ui.text("Function"); ui.next_column();

         let mut i = 0;
         let size = Vec2::new(0f32, 0f32);

         self.set_selected_thread = true;
         let old_selected_thread = self.selected_thread;

         for entry in reader.find_array("threads").unwrap() {
             let id = entry.find_u64("id").unwrap();
             let name = entry.find_string("name").unwrap();
             let function = entry.find_string("function").unwrap();

             if ui.selectable("##selectable", self.selected_thread == i, PDUISELECTABLEFLAGS__SPANALLCOLUMNS, size) {
                 self.selected_thread = i;
                 self.thread_id = id as u32;
             }
             
             ui.next_column();
             ui.text(name); ui.next_column();
             ui.text(function); ui.next_column();

             i+=1;
         }

         if old_selected_thread != self.selected_thread {
             self.set_selected_thread = true;
         }
     }
}

impl View for ThreadsView {
    fn new(_: &Ui, _: &Service) -> Self {
        ThreadsView {
            selected_thread: 0u32,
            set_selected_thread: true,
            thread_id: 0u32,
            request_data: false
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, writer: &mut Writer) {
        self.request_data = false;
        self.set_selected_thread = false;

        for event in reader.get_events() {
            match event {
                EVENT_SET_EXCEPTION_LOCATION => {
                    self.request_data = true
                },

                EVENT_SET_THREADS => {
                    self.show_in_ui(reader, ui)
                }
                
                _ => {},
            }
        }

        // Request threads data

        if self.set_selected_thread {
            writer.event_begin(EVENT_SELECT_THREAD as u16);
            println!("writing thread id {}", self.thread_id);
            writer.write_u32("thread_id", self.thread_id);
            writer.event_end();
        }
        
        if self.request_data {
            writer.event_begin(EVENT_GET_THREADS as u16);
            writer.event_end();
        }
    }
}


#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Threads\0", ThreadsView);
    plugin_handler.register_view(&PLUGIN);
}

