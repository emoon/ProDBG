//! Editor that only allows changing characters. It could be just a function `CharEditor::render`
//! but setting focus to editor requires 2 frames so editor has to remember whether it should set
//! focus or not.
//! Since ImGui does not have functionality to change text cursor style, `CharEditor` renders any
//! text before and after the cursor as bare text and only one symbol under the cursor is rendered
//! as actual input.

use prodbg_api::{Ui, PDVec2, ImGuiStyleVar, InputTextCallbackData, Key};
use prodbg_api::{PDUIINPUTTEXTFLAGS_NOHORIZONTALSCROLL, PDUIINPUTTEXTFLAGS_AUTOSELECTALL,
                 PDUIINPUTTEXTFLAGS_ALWAYSINSERTMODE, PDUIINPUTTEXTFLAGS_CALLBACKALWAYS,
                 PDUIINPUTTEXTFLAGS_CALLBACKCHARFILTER, PDUIInputTextFlags_};
use helper::get_text_cursor_index;

#[derive(Debug)]
pub struct CharEditor {
    should_take_focus: bool,
    should_set_pos_to_start: bool,
}

/// Points at next cursor position after current frame.
pub enum NextPosition {
    /// Left arrow key has been pressed when cursor is at the leftmost character of text
    Left,
    /// Right arrow key has been pressed or character has been changed when cursor is at the
    /// rightmost character of text
    Right,
    /// Cursor has not changed
    Unchanged,
    /// Cursor has been changed to corresponding value
    Changed(usize),
}

impl CharEditor {
    /// Creates new `CharEditor`. It will take focus and set cursor to 0 as quickly as possible.
    pub fn new() -> CharEditor {
        CharEditor {
            should_take_focus: true,
            should_set_pos_to_start: true,
        }
    }

    fn render_input(&mut self,
                    ui: &mut Ui,
                    buf: &mut [u8],
                    flags: PDUIInputTextFlags_,
                    char_filter: Option<&Fn(char) -> char>)
                    -> (i32, bool) {
        let mut cursor_pos = 0;
        let text_has_changed;
        {
            let callback = |mut data: InputTextCallbackData| {
                let flag = data.get_event_flag();
                if flag == PDUIINPUTTEXTFLAGS_CALLBACKALWAYS {
                    if self.should_set_pos_to_start {
                        data.set_cursor_pos(0);
                        self.should_set_pos_to_start = false;
                    } else {
                        cursor_pos = data.get_cursor_pos();
                    }
                }
                if flag == PDUIINPUTTEXTFLAGS_CALLBACKCHARFILTER {
                    if let Some(c) = data.get_event_char() {
                        if let Some(filter) = char_filter {
                            data.set_event_char(filter(c));
                        }
                    }
                }
            };
            ui.push_item_width(ui.calc_text_size("f", 0).0);
            ui.push_style_var_vec(ImGuiStyleVar::FramePadding, PDVec2 { x: 0.0, y: 0.0 });
            let mut flags = flags | PDUIINPUTTEXTFLAGS_NOHORIZONTALSCROLL |
                            PDUIINPUTTEXTFLAGS_AUTOSELECTALL |
                            PDUIINPUTTEXTFLAGS_ALWAYSINSERTMODE |
                            PDUIINPUTTEXTFLAGS_CALLBACKALWAYS;
            if char_filter.is_some() {
                flags = flags | PDUIINPUTTEXTFLAGS_CALLBACKCHARFILTER;
            }
            text_has_changed = ui.input_text("##data", buf, flags, Some(&callback));
            ui.pop_style_var(1);
            ui.pop_item_width();
        }
        (cursor_pos, text_has_changed)
    }

    /// Renders char editor.
    /// `flags` are `InputTextFlags` as i32
    /// `char_filter` is function to filter input character. Set character to `\u{0}` to cancel it.
    /// If `char_filter` is `Some`, appropriate flag will be added.
    pub fn render(&mut self,
                  ui: &mut Ui,
                  text: &str,
                  mut cursor: usize,
                  flags: PDUIInputTextFlags_,
                  char_filter: Option<&Fn(char) -> char>)
                  -> (NextPosition, Option<String>) {
        if text.len() == 0 {
            return (NextPosition::Unchanged, None);
        }
        if cursor >= text.len() {
            cursor = text.len() - 1;
        }
        let mut next_position = NextPosition::Unchanged;
        let mut buf = [text.as_bytes()[cursor], 0];
        ui.push_style_var_vec(ImGuiStyleVar::ItemSpacing, PDVec2 { x: 0.0, y: 0.0 });

        // render bare text before cursor
        if cursor > 0 {
            let left = &text[0..cursor];
            ui.text(left);
            ui.same_line(0, -1);
            if ui.is_item_hovered() && ui.is_mouse_clicked(0, false) {
                next_position = NextPosition::Changed(get_text_cursor_index(ui, left.len()));
            }
        }

        if self.should_take_focus {
            ui.set_keyboard_focus_here(0);
            self.should_take_focus = false;
        }
        let (cursor_pos, text_has_changed) = self.render_input(ui, &mut buf, flags, char_filter);
        let changed_text = if text_has_changed {
            ::std::str::from_utf8(&buf[0..1]).ok().map(|s| s.to_owned())
        } else {
            None
        };
        let char_count = text.len();
        if cursor_pos > 0 {
            next_position = if cursor == char_count - 1 {
                NextPosition::Right
            } else {
                NextPosition::Changed(cursor + 1)
            }
        }

        if cursor < char_count {
            ui.same_line(0, -1);
            let right = &text[cursor + 1..char_count];
            ui.text(right);
            if ui.is_item_hovered() && ui.is_mouse_clicked(0, false) {
                next_position = NextPosition::Changed(cursor + 1 +
                                                      get_text_cursor_index(ui, right.len()));
            }
        }

        ui.pop_style_var(1);

        if ui.is_key_pressed(Key::Left, true) {
            next_position = if cursor > 0 {
                NextPosition::Changed(cursor - 1)
            } else {
                NextPosition::Left
            }
        }

        return (next_position, changed_text);
    }
}
