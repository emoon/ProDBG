#[macro_use]
extern crate prodbg_api;

use prodbg_api::*;

struct Line {
    opcode: String,
    regs_write: String,
    regs_read: String,
    address: u64,
}

///
/// Breakpoint
///
struct Breakpoint {
    address: u64,
}

///
/// Holds colors use for the disassembly view. This should be possible to configure later on.
///
// struct Colors {
// breakpoint: Color,
// step_cursor: Color,
// cursor: Color,
// address: Color,
// _bytes: Color,
// }
//


struct DisassemblyView {
    exception_location: u64,
    has_made_step: bool,
    cursor: u64,
    breakpoint_radius: f32,
    breakpoint_spacing: f32,
    address_size: u8,
    reset_to_center: bool,
    lines: Vec<Line>,
    breakpoints: Vec<Breakpoint>,
}

impl DisassemblyView {
    fn set_disassembly(&mut self, reader: &mut Reader) {
        self.lines.clear();

        for entry in reader.find_array("disassembly") {
            let address = entry.find_u64("address").ok().unwrap();
            let line = entry.find_string("line").ok().unwrap();
            let mut regs_read = String::new();
            let mut regs_write = String::new();

            entry.find_string("registers_read")
                .map(|regs| {
                    regs_read = regs.to_owned();
                })
                .ok();

            entry.find_string("registers_write")
                .map(|regs| {
                    regs_write = regs.to_owned();
                })
                .ok();

            self.lines.push(Line {
                opcode: line.to_owned(),
                regs_read: regs_read,
                regs_write: regs_write,
                address: address,
            });
        }
    }

    ///
    /// Calculate how many visible lines we have
    ///
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

    fn color_text_reg_selection(ui: &Ui, regs_use: &Vec<&str>, line: &Line, text_height: f32) {
        let (_cx, cy) = ui.get_cursor_screen_pos();
        let mut color_index = 0;
        // TODO: Allocs memory, fix
        let line_text = format!("   0x{:x} {}", line.address, line.opcode);

        let colors = [0x00b27474, 0x00b28050, 0x00a9b250, 0x0060b250, 0x004fb292, 0x004f71b2,
                      0x008850b2, 0x00b25091];

        // let mut color_index = 0;

        // let font_size = 14.0;
        // TODO: Offset here is hardcoded with given hex size, this should be fixed
        // let start_offset = font_size * 11.0; // 0x00000000 "

        for reg in regs_use {
            let color = colors[color_index & 7];
            line_text.find(reg).map(|offset| {
                let (tx, _) = ui.calc_text_size(&line_text, offset);
                ui.fill_rect(0.0 + tx,
                             cy,
                             22.0,
                             text_height,
                             Color::from_au32(200, color));
            });

            color_index += 1;
        }

        ui.text(&line_text);
    }

    fn toggle_breakpoint(&mut self, writer: &mut Writer) {
        let address = self.cursor;

        for i in (0..self.breakpoints.len()).rev() {
            if self.breakpoints[i].address == address {
                writer.event_begin(EVENT_DELETE_BREAKPOINT as u16);
                writer.write_u64("address", address);
                writer.event_end();
                self.breakpoints.swap_remove(i);
                return;
            }
        }

        writer.event_begin(EVENT_SET_BREAKPOINT as u16);
        writer.write_u64("address", address);
        writer.event_end();

        println!("adding breakpoint");

        // TODO: We shouldn't really add the breakpoint here but wait for reply
        // from the backend that we actually managed to set the breakpoint.
        // +bonus would be a "progress" icon here instead that it's being set.

        self.breakpoints.push(Breakpoint { address: address });
    }

    fn has_breakpoint(&self, address: u64) -> bool {
        self.breakpoints.iter().find(|bp| bp.address == address).is_some()
    }

    // fn set_cursor_at(&mut self, pos: i32) {
    // if we don'n have enough lines we need to fetch more
    // if pos < 0 {
    //
    // return;
    // }
    //
    // need to fetch more data here also
    // if pos > self.lines.len() as i32 {
    //
    // return;
    // }
    //
    // self.cursor = self.lines[pos as usize].address;
    // }
    //

    fn scroll_cursor(&mut self, steps: i32) {
        for (i, line) in self.lines.iter().enumerate() {
            if line.address == self.cursor {
                let pos = (i as i32) + steps;
                // if we don'n have enough lines we need to fetch more
                if pos < 0 {
                    return;
                }

                // need to fetch more data here also
                if pos >= self.lines.len() as i32 {
                    return;
                }

                self.cursor = self.lines[pos as usize].address;
                return;
            }
        }
    }

    fn render_arrow(ui: &Ui, pos_x: f32, pos_y: f32, scale: f32) {
        let color = Color::from_argb(255, 0, 180, 180);

        ui.fill_rect(pos_x + (0.0 * scale),
                     pos_y + (0.25 * scale),
                     0.5 * scale,
                     0.5 * scale,
                     color);

        // Wasn't able to get this to work with just one convex poly fill
        let arrow: [Vec2; 3] = [Vec2::new((0.50 * scale) + pos_x, (0.00 * scale) + pos_y),
                                Vec2::new((1.00 * scale) + pos_x, (0.50 * scale) + pos_y),
                                Vec2::new((0.50 * scale) + pos_x, (1.00 * scale) + pos_y)];

        ui.fill_convex_poly(&arrow, color, true);
    }

    fn render_ui(&mut self, ui: &mut Ui) {
        if self.lines.len() == 0 {
            return;
        }

        let (size_x, size_h) = ui.get_window_size();
        let text_height = ui.get_text_line_height_with_spacing();
        let mut regs = String::new();
        let mut regs_pc_use = Vec::new();
        // let font_size = ui.get_font_size();

        // find registerss for pc

        for line in &self.lines {
            if line.address == self.exception_location {
                if line.regs_read.len() > 1 || line.regs_write.len() > 1 {
                    if line.regs_read.len() > 0 {
                        regs.push_str(&line.regs_read);
                    }
                    if line.regs_write.len() > 0 {
                        regs.push(' ');
                        regs.push_str(&line.regs_write);
                    }

                    let t = regs.trim_left();
                    regs_pc_use = t.split(' ').collect();
                    break;
                }
            }
        }

        for line in &self.lines {
            let (_cx, cy) = ui.get_cursor_screen_pos();
            let bp_radius = self.breakpoint_radius;

            if line.address == self.cursor {
                if (cy - text_height) < 0.0 {
                    if self.reset_to_center || self.has_made_step {
                        ui.set_scroll_here(0.5);
                        self.reset_to_center = false;
                    } else {
                        ui.set_scroll_from_pos_y(cy - text_height, 0.0);
                    }
                }

                if cy > (size_h - text_height) {
                    if self.reset_to_center || self.has_made_step {
                        ui.set_scroll_here(0.5);
                        self.reset_to_center = false;
                    } else {
                        ui.set_scroll_from_pos_y(cy + text_height, 1.0);
                    }
                }

                ui.fill_rect(0.0,
                             cy,
                             size_x,
                             text_height,
                             Color::from_argb(200, 0, 0, 127));
            }

            if regs_pc_use.len() > 0 {
                Self::color_text_reg_selection(ui, &regs_pc_use, &line, text_height);
            } else {
                ui.text_fmt(format_args!("   0x{:x} {}", line.address, line.opcode));
            }

            if self.has_breakpoint(line.address) {
                ui.fill_circle(&Vec2 {
                                   x: self.breakpoint_spacing + bp_radius,
                                   y: cy + bp_radius + 2.0,
                               },
                               bp_radius,
                               Color::from_argb(255, 0, 0, 140),
                               12,
                               false);
            }

            // println!("draw arrow {} {}", line.address, self.exception_location);

            if line.address == self.exception_location {
                Self::render_arrow(ui, 0.0, cy + 2.0, self.breakpoint_radius * 2.0);
            }
        }

        self.has_made_step = false;
    }
}

impl View for DisassemblyView {
    fn new(_: &Ui, _: &Service) -> Self {
        DisassemblyView {
            exception_location: u64::max_value(),
            cursor: 0xe003, // u64::max_value(),
            breakpoint_radius: 10.0,
            breakpoint_spacing: 6.0,
            address_size: 4,
            has_made_step: false,
            lines: Vec::new(),
            breakpoints: Vec::new(),
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

                    if self.exception_location != location {
                        self.request_disassembly(ui, location, writer);
                        self.has_made_step = true;
                        self.exception_location = location;
                        self.cursor = location;
                    }
                }

                EVENT_SET_DISASSEMBLY => {
                    self.set_disassembly(reader);
                }

                _ => (),
            }
        }

        if ui.is_key_down(Key::F9) {
            self.toggle_breakpoint(writer);
        }

        if ui.is_key_down(Key::Down) {
            self.scroll_cursor(1);
        }

        if ui.is_key_down(Key::Up) {
            self.scroll_cursor(-1);
        }

        if ui.is_key_down(Key::PageDown) {
            self.scroll_cursor(8);
        }

        if ui.is_key_down(Key::PageUp) {
            self.scroll_cursor(-8);
        }

        self.render_ui(ui);
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Disassembly2 View", DisassemblyView);
    plugin_handler.register_view(&PLUGIN);
}
