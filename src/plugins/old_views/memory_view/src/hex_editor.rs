//! Memory editor that only allows changing digits and not deleting them.
//! This editor can only be used with Hex number representation as it relies on several properties
//! of it.


use prodbg_api::{Ui, PDUIINPUTTEXTFLAGS_CHARSHEXADECIMAL};
use number_view::NumberView;
use char_editor::{CharEditor, NextPosition};

#[derive(Debug, Clone)]
pub struct HexEditor {
    pub address: usize,
    view: NumberView,
    char_editor: CharEditor,
}

impl HexEditor {
    pub fn new(address: usize, cursor: usize, view: NumberView) -> HexEditor {
        HexEditor {
            address: address,
            view: view,
            char_editor: CharEditor::new(cursor),
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

    pub fn set_cursor(&mut self, cursor: usize) {
        self.char_editor.cursor = Some(cursor);
    }

    pub fn render(&mut self, ui: &mut Ui, data: &mut [u8]) -> (Option<(usize, usize)>, bool) {
        ui.push_id_usize(self.address);
        let text = self.view.format(data);
        let (next_position, changed_text) = self.char_editor
            .render(ui,
                    &text,
                    PDUIINPUTTEXTFLAGS_CHARSHEXADECIMAL,
                    None);
        ui.pop_id();
        let mut data_has_changed = false;
        if let Some(text) = changed_text {
            match self.view.parse(&text) {
                Ok(bytes) => {
                    data_has_changed = data != bytes.as_slice();
                    data.copy_from_slice(&bytes);
                }
                Err(e) => println!("Could not parse: {}", e),
            }
        }

        let next_position = match next_position {
            NextPosition::Within => None,
            NextPosition::Left => self.previous_position(),
            NextPosition::Right => self.next_position(),
        };

        return (next_position, data_has_changed);
    }
}
