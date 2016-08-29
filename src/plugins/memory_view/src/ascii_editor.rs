//! Memory editor that only allows changing digits and not deleting them.
//! This editor can only be used with Hex number representation as it relies on several properties
//! of it.

use prodbg_api::{Ui, PDUIInputTextFlags_};
use char_editor::{CharEditor, NextPosition};
use std;

pub struct AsciiEditor {
    /// Address in memory
    pub address: usize,
    char_editor: CharEditor,
}

impl AsciiEditor {
    pub fn new(address: usize) -> AsciiEditor {
        AsciiEditor {
            address: address,
            char_editor: CharEditor::new(0),
        }
    }

    fn filter_char(c: char) -> char {
        match c as u32 {
            32...127 => c,
            _ => '\u{0}',
        }
    }

    pub fn render(&mut self, ui: &mut Ui, data: &mut u8) -> (Option<usize>, bool) {
        // ids are needed to prevent ImGui from reusing old buffer
        ui.push_id_usize(self.address);
        let mut text = String::with_capacity(1);
        text.push(match *data {
            32...127 => unsafe { std::char::from_u32_unchecked(*data as u32) },
            _ => '.',
        });
        let (next_position, changed_char) = self.char_editor
            .render(ui,
                    &text,
                    PDUIInputTextFlags_::empty(),
                    Some(&AsciiEditor::filter_char));
        ui.pop_id();
        let mut data_has_changed = false;
        if let Some(changed_text) = changed_char {
            let value = changed_text.chars().next().and_then(|c| match c as u32 {
                32...127 => Some(c as u8),
                _ => None,
            });
            if let Some(new_value) = value {
                data_has_changed = new_value != *data;
                *data = new_value;
            }
        }

        let next_position = match next_position {
            NextPosition::Left => self.address.checked_sub(1),
            NextPosition::Right => self.address.checked_add(1),
            _ => None,
        };

        return (next_position, data_has_changed);
    }
}

impl Clone for AsciiEditor {
    fn clone(&self) -> AsciiEditor {
        AsciiEditor::new(self.address)
    }
}
