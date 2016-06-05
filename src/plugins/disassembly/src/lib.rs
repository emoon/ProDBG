#[macro_use]
extern crate prodbg_api;

use prodbg_api::*;

struct Line {
    opcode: String,
    address: u64,
    _breakpoint: bool
}

struct DisassemblyView {
    location: u64,
    address_size: u8,
    reset_to_center: bool,
    lines: Vec<Line>,
}

impl DisassemblyView {
    fn set_disassembly(&mut self, reader: &mut Reader) {
        self.lines.clear();

        for entry in reader.find_array("disassembly") {
            let address = entry.find_u64("address").ok().unwrap();
            let line = entry.find_string("line").ok().unwrap();

            self.lines.push(Line {
                address: address,
                _breakpoint: false,
                opcode: line.to_owned(),
            });
        }
    }

    fn get_visible_lines_count(ui: &Ui) -> usize {
        let (_, height) = ui.get_window_size();
        let text_height = ui.get_text_line_height_with_spacing();
        // - 1.0 for title text. Would be better to get the cursor pos here instead
        let visible_lines = (height / text_height) - 1.0;
        // + 0.5 to round up
        (visible_lines + 0.5) as usize
    }

    fn request_disassembly(&mut self, ui: &mut Ui, location: u64, writer: &mut Writer) {
        let visible_lines = Self::get_visible_lines_count(ui) as u64;

        self.location = location;

        // check if we have the the location within all lines, then we don't need to request more
        for line in &self.lines {
            if line.address == location {
                return;
            }
        }

        self.reset_to_center = true;
        writer.event_begin(EVENT_GET_DISASSEMBLY as u16);
        writer.write_u64("address_start", location - (visible_lines * 4));
        writer.write_u32("instruction_count", (visible_lines * 4) as u32);
        writer.event_end();
        println!("requsted {}", visible_lines * 10);
    }

    fn render_ui(&mut self, ui: &mut Ui) {
        if self.lines.len() == 0 {
            return;
        }

        let (size_x, size_h) = ui.get_window_size();
        let text_height = ui.get_text_line_height_with_spacing();

        for line in &self.lines {
            if line.address == self.location {
                let (cx, cy) = ui.get_cursor_screen_pos();

                if (cy < 0.0 || cy > (size_h - text_height)) || self.reset_to_center {
                    ui.set_scroll_here(0.5);
                    self.reset_to_center = false;
                }

                ui.fill_rect(cx, cy, size_x, text_height, (200 << 24) | 127);
            }

            ui.text_fmt(format_args!("0x{:x} {}", line.address, line.opcode));
        }
    }
}

impl View for DisassemblyView {
    fn new(_: &Ui, _: &Service) -> Self {
        DisassemblyView {
            location: u64::max_value(),
            address_size: 4,
            lines: Vec::new(),
            reset_to_center: false,
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, writer: &mut Writer) {
        for event in reader.get_events() {
            match event {
                EVENT_SET_EXCEPTION_LOCATION => {
                    let location = reader.find_u64("address").ok().unwrap();

                    reader.find_u8("address_size").ok().map(|adress_size| {
                        self.address_size = adress_size;
                    });

                    if self.location != location {
                        self.request_disassembly(ui, location, writer);
                    }
                }

                EVENT_SET_DISASSEMBLY => {
                    self.set_disassembly(reader);
                }

                _ => (),
            }
        }

        self.render_ui(ui);
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Disassembly2 View", DisassemblyView);
    plugin_handler.register_view(&PLUGIN);
}
