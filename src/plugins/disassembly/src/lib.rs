#[macro_use]
extern crate prodbg_api;

use prodbg_api::*;

struct Line {
    opcode: String,
    regs_write: String,
    regs_read: String,
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
            let mut regs_read = String::new();
            let mut regs_write =  String::new();

            entry.find_string("registers_read").map(|regs| {
                regs_read = regs.to_owned();
            }).ok();

            entry.find_string("registers_write").map(|regs| {
                regs_write = regs.to_owned();
            }).ok();

            self.lines.push(Line {
                opcode: line.to_owned(),
                regs_read: regs_read,
                regs_write: regs_write,
                address: address,
                _breakpoint: false,
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

    fn color_text_reg_selection(ui: &Ui, regs_use: &Vec<&str>, line: &String, text_height: f32) {
        let (cx, cy) = ui.get_cursor_screen_pos();
        //let mut color_index = 0;

        let font_size = 14.0;
        // TODO: Offset here is hardcoded with given hex size, this should be fixed
        //let start_offset = font_size * 11.0; // 0x00000000 "

        for reg in regs_use {
            line.find(reg).map(|offset| {
                let o = cx + (offset as f32) * font_size;
                println!("found {} at offset {} for {}", reg, offset, line);
                ui.fill_rect(o, cy, font_size * 2.0, text_height, (200 << 24) | 127 << 8);
                //color_index +=
            });
        }
    }

    fn render_ui(&mut self, ui: &mut Ui) {
        if self.lines.len() == 0 {
            return;
        }

        /*
        let colors = [
            0x00b27474,
            0x00b28050,
            0x00a9b250,
            0x0060b250,

            0x004fb292,
            0x004f71b2,
            0x008850b2,
            0x00b25091,
        ];
        */

        let (size_x, size_h) = ui.get_window_size();
        let text_height = ui.get_text_line_height_with_spacing();
        let mut regs = String::new();
        let mut regs_pc_use = Vec::new();
        //let font_size = ui.get_font_size();

        // find registerss for pc

        for line in &self.lines {
            if line.address == self.location {
                if line.regs_read.len() > 1 || line.regs_write.len() > 1 {
                    regs.push_str(&line.regs_read);
                    if line.regs_write.len() > 0 {
                        regs.push(' ');
                        regs.push_str(&line.regs_write);
                    }
                    regs_pc_use = regs.split(' ').collect();
                    break;
                }
            }
        }

        for line in &self.lines {
            if line.address == self.location {
                let (cx, cy) = ui.get_cursor_screen_pos();

                if (cy < 0.0 || cy > (size_h - text_height)) || self.reset_to_center {
                    ui.set_scroll_here(0.5);
                    self.reset_to_center = false;
                }

                ui.fill_rect(cx, cy, size_x, text_height, (200 << 24) | 127);
            }

            if regs_pc_use.len() > 0 {
                Self::color_text_reg_selection(ui, &regs_pc_use, &line.opcode, text_height);
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
