use prodbg_api::{Ui, PDUIINPUTTEXTFLAGS_CHARSHEXADECIMAL};
use number_view::{NumberView, Endianness};
use char_editor::{CharEditor, NextPosition};

#[derive(Debug)]
pub struct HexEditor {
    pub cursor: usize,
    view: NumberView,
    char_editor: CharEditor,
}

impl HexEditor {
    pub fn new(cursor: usize, view: NumberView) -> HexEditor {
        HexEditor {
            cursor: cursor,
            view: view,
            char_editor: CharEditor::new(),
        }
    }

    pub fn render(&mut self, ui: &mut Ui, data: &mut [u8]) -> (NextPosition<usize>, bool) {
        // TODO: need unique id here
        ui.push_id_usize(self.cursor);
        let text = self.view.format(data);
        let (next_position, changed_digit) = self.char_editor
            .render(ui,
                    &text,
                    self.cursor,
                    PDUIINPUTTEXTFLAGS_CHARSHEXADECIMAL,
                    None);
        ui.pop_id();
        let mut data_has_changed = false;
        if let Some(changed_digit) = changed_digit {
            let value = u8::from_str_radix(&changed_digit, 16).unwrap();
            let offset = match self.view.endianness {
                Endianness::Little => (text.len() - self.cursor - 1) / 2,
                Endianness::Big => self.cursor / 2,
            };
            let new_byte = if self.cursor % 2 == 1 {
                data[offset] & 0b11110000 | value
            } else {
                data[offset] & 0b00001111 | (value << 4)
            };
            data_has_changed = data[offset] != new_byte;
            data[offset] = new_byte;
        }

        (next_position, data_has_changed)
    }
}

impl Clone for HexEditor {
    fn clone(&self) -> HexEditor {
        HexEditor::new(self.cursor, self.view)
    }
}
