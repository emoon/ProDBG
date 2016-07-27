//! Memory editor that only allows changing digits and not deleting them.
//! This editor can only be used with Hex number representation as it relies on several properties
//! of it.

use prodbg_api::{Ui, PDUIINPUTTEXTFLAGS_CHARSHEXADECIMAL};
use number_view::{NumberView, Endianness};
use char_editor::{CharEditor, NextPosition};

#[derive(Debug)]
pub struct HexEditor {
    pub address: usize,
    pub cursor: usize,
    view: NumberView,
    char_editor: CharEditor,
}

impl HexEditor {
    pub fn new(address: usize, cursor: usize, view: NumberView) -> HexEditor {
        HexEditor {
            address: address,
            cursor: cursor,
            view: view,
            char_editor: CharEditor::new(),
        }
    }

    /// Returns position preceding current. Returns `None` if we're at (0, 0) or `self.position` is
    /// `None`.
    fn previous_position(&self) -> Option<(usize, usize)> {
        self.address
            .checked_sub(self.view.size.byte_count())
            .map(|address| (address, self.view.maximum_chars_needed() - 1))
    }

    /// Returns position succeeding current. Returns `None` if address overflows.
    fn next_position(&self) -> Option<(usize, usize)> {
        self.address
            .checked_add(self.view.size.byte_count())
            .map(|address| (address, 0))
    }

    pub fn render(&mut self, ui: &mut Ui, data: &mut [u8]) -> (Option<(usize, usize)>, bool) {
        // ids are needed to prevent ImGui from reusing old buffer
        ui.push_id_usize(self.address);
        ui.push_id_usize(self.cursor);
        let text = self.view.format(data);
        let (next_position, changed_digit) = self.char_editor
            .render(ui,
                    &text,
                    self.cursor,
                    PDUIINPUTTEXTFLAGS_CHARSHEXADECIMAL,
                    None);
        ui.pop_id();
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

        let next_position = match next_position {
            NextPosition::Unchanged => None,
            NextPosition::Changed(next_cursor) => Some((self.address, next_cursor)),
            NextPosition::Left => self.previous_position(),
            NextPosition::Right => self.next_position(),
        };

        return (next_position, data_has_changed);
    }
}

impl Clone for HexEditor {
    fn clone(&self) -> HexEditor {
        HexEditor::new(self.address, self.cursor, self.view)
    }
}
