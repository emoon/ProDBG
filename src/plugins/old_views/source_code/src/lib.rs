#[macro_use]
extern crate prodbg_api;

// TODO: Move to scintilla.rs (by using rustbind-gen) 
const SCI_CLEARALL: u32 = 2004;
const SCI_ADDTEXT: u32 = 2001;
const SCN_TOGGLE_BREAKPOINT: u32 = 4431;
const SCN_GETCURRENT_LINE: u32 = 4432;
const SCI_GOTOLINE: u32 = 2024;

use prodbg_api::{View, Ui, Reader, Writer, StateSaver, StateLoader, LoadResult,
                 PluginHandler, Service, CViewCallbacks};

use prodbg_api::scintilla::Scintilla;
use prodbg_api::events::*;
use std::io::Result;
use std::fs::File;
use std::io::Read;

struct SourceCodeView {
    filename: String,
    line: u32,
    update_after_load: bool,
}

impl SourceCodeView {
    fn toggle_breakpoint(&mut self, sc: &Scintilla, writer: &mut Writer) {
        sc.send_command(SCN_TOGGLE_BREAKPOINT, 0, 0);

        let current_line = sc.send_command(SCN_GETCURRENT_LINE, 0, 0) as u32;

        writer.event_begin(EVENT_SET_BREAKPOINT as u16);
        writer.write_string("filename", &self.filename);
        writer.write_u32("line", current_line);
        writer.event_end();
    }

    fn update_file_data(sc: &Scintilla, filename: &str) -> Result<()> {
        let mut file_data = "".to_owned();

        let mut file = try!(File::open(filename));
        let _ = try!(file.read_to_string(&mut file_data));

        sc.send_command(SCI_CLEARALL, 0, 0);
        sc.send_command_str(SCI_ADDTEXT, &file_data);

        Ok(())
    }

    // Calculate if we need to change the scroll based on where the cursor is
    fn set_line(&mut self, sc: &Scintilla, ui: &Ui) {
        let text_height = ui.get_font_size() as isize;
        let window_height = ui.get_window_size().y as isize;
        let scroll_pos = ui.get_scroll_y() as isize;
        let line_pixel_pos = self.line as isize * text_height;

        println!("line_pos {}", line_pixel_pos); 
        println!("scroll_pos {} line {}", scroll_pos, scroll_pos / text_height); 
        println!("scroll_pos + height {}", scroll_pos + window_height); 

        if line_pixel_pos < scroll_pos || 
           line_pixel_pos > (scroll_pos + window_height) { 
            // scroll to center where the line is
            let mut scroll_pos = line_pixel_pos - window_height / 2; 
            if scroll_pos < 0 { scroll_pos = 0; }
            ui.set_scroll_y(scroll_pos as f32);
        }

        sc.send_command(SCI_GOTOLINE, self.line as u64, 0);
    }


    fn set_source_file(&mut self, sc: &Scintilla, filename: &str, line: u32) {
        self.line = line;

        if self.filename != filename {
            if let Err(e) = Self::update_file_data(sc, filename) { 
                println!("Unable to update source view with {} err {:?}", filename, e);
            }
        
            self.filename = filename.to_owned();
        }
    }

    fn set_exception_location(&mut self, sc: &Scintilla, reader: &mut Reader) {
        if let Ok(filename) = reader.find_string("filename") {
            if let Ok(line) = reader.find_u32("line") {
                self.set_source_file(sc, filename, line);
            }
        }
    }
}

impl View for SourceCodeView {
    fn new(_: &Ui, _: &Service) -> Self {
        SourceCodeView {
            filename: "".to_owned(),
            line: 1, 
            update_after_load: false,
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, writer: &mut Writer) {
        let sc_text = ui.sc_input_text("test", 800, 700);

        if self.update_after_load {
            if Self::update_file_data(&sc_text, &self.filename).is_err() {
                println!("Unable to restore file {}", self.filename);
            }

            self.update_after_load = false;
        }

        for event in reader.get_events() {
            match event {
                EVENT_SET_EXCEPTION_LOCATION => {
                    self.set_exception_location(&sc_text, reader);
                    self.set_line(&sc_text, ui);
                }

                EVENT_TOGGLE_BREAKPOINT_CURRENT_LINE => {
                    self.toggle_breakpoint(&sc_text, writer);
                }

                _ => (),
            }
        }

        sc_text.update();
        sc_text.draw();
    }

    fn save_state(&mut self, mut saver: StateSaver) {
        saver.write_str(&self.filename);
    }

    fn load_state(&mut self, mut loader: StateLoader) {
        if let LoadResult::Ok(value) = loader.read_string() {
            self.filename = value.to_owned();
        }
        self.update_after_load = true;
    }
}


#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Source Code View\0", SourceCodeView);
    plugin_handler.register_view(&PLUGIN);
}

