//! Input field for address in memory

use prodbg_api::{Ui, ImGuiStyleVar, Vec2};
use prodbg_api::{PDUIINPUTTEXTFLAGS_CHARSHEXADECIMAL, PDUIINPUTTEXTFLAGS_ENTERRETURNSTRUE,
                 PDUIINPUTTEXTFLAGS_NOHORIZONTALSCROLL};

/// Input field for address in memory
pub struct AddressInput {
    // TODO: What buffer do we really need for address?
    buf: [u8; 20],
    value: usize,
}

impl AddressInput {
    /// Creates new input
    pub fn new(value: usize) -> AddressInput {
        let mut res = AddressInput {
            buf: [0; 20],
            value: 0,
        };
        res.set(value);
        return res;
    }

    /// Renders input. Returns `true` if value has changed.
    pub fn render(&mut self, ui: &mut Ui) -> bool {
        let mut res = false;
        ui.text("0x");
        ui.push_style_var_vec(ImGuiStyleVar::FramePadding, Vec2 { x: 1.0, y: 0.0 });
        ui.push_item_width(ui.calc_text_size("00000000", 0).x + 2.0);
        ui.same_line(0, 0);
        let flags = PDUIINPUTTEXTFLAGS_CHARSHEXADECIMAL | PDUIINPUTTEXTFLAGS_ENTERRETURNSTRUE |
                    PDUIINPUTTEXTFLAGS_NOHORIZONTALSCROLL;
        if ui.input_text("##address", &mut self.buf, flags, None) {
            let len = self.buf.iter().position(|&b| b == 0).unwrap_or(self.buf.len());
            let str_slice = ::std::str::from_utf8(&self.buf[0..len]).unwrap();
            let old_value = self.value;
            self.value = usize::from_str_radix(str_slice, 16).unwrap_or(old_value);
            res = self.value != old_value;
        }
        ui.pop_item_width();
        ui.pop_style_var(1);
        res
    }

    /// Returns value
    pub fn get(&self) -> usize {
        self.value
    }

    /// Sets new value
    pub fn set(&mut self, value: usize) {
        self.value = value;
        let data = format!("{:08x}", value);
        (&mut self.buf[0..data.len()]).copy_from_slice(data.as_bytes());
        self.buf[data.len() + 1] = 0;
    }
}
