//! Editor that only allows changing characters. It could be just a function `CharEditor::render`
//! but setting focus to editor requires 2 frames so editor has to remember whether it should set
//! focus or not.
//! Since ImGui does not have functionality to change text cursor style, `CharEditor` renders any
//! text before and after the cursor as bare text and only one symbol under the cursor is rendered
//! as actual input.
//! Does not support multi-byte characters.
//! TODO: use internal ImGui methods to register this component as active and process tab/mouse
//! clicks using ImGui.


extern crate prodbg_api;

use prodbg_api::{Ui, Vec2, ImGuiStyleVar, InputTextCallbackData, Key};
pub use prodbg_api::{PDUIINPUTTEXTFLAGS_NOHORIZONTALSCROLL, PDUIINPUTTEXTFLAGS_AUTOSELECTALL,
                 PDUIINPUTTEXTFLAGS_ALWAYSINSERTMODE, PDUIINPUTTEXTFLAGS_CALLBACKALWAYS,
                 PDUIINPUTTEXTFLAGS_CALLBACKCHARFILTER, PDUIInputTextFlags_};


/// Returns index if character under mouse cursor. Assumes one-line text and monospace font.
pub fn get_text_cursor_index(ui: &Ui, len: usize) -> usize {
    let width = ui.get_item_rect_size().x;
    let left_border = ui.get_item_rect_min().x;
    let mouse = ui.get_mouse_pos().x;
    let space_per_symbol = width / len as f32;
    if mouse < left_border {
        return 0;
    }
    let res = ((mouse - left_border) / space_per_symbol) as usize;
    if res > len - 1 {
        return len - 1;
    }
    return res;
}


#[derive(Debug)]
pub struct CharEditor {
    pub cursor: Option<usize>,
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
    /// Cursor is within
    Within,
}

impl CharEditor {
    /// Creates new `CharEditor`. It will take focus and set cursor to 0 as quickly as possible.
    pub fn new(cursor: usize) -> CharEditor {
        CharEditor {
            cursor: Some(cursor),
            should_take_focus: true,
            should_set_pos_to_start: true,
        }
    }

    fn change_cursor(&mut self, cursor: usize) {
        if self.cursor == Some(cursor) {
            return;
        }
        self.cursor = Some(cursor);
        self.should_take_focus = true;
        self.should_set_pos_to_start = true;
    }

    fn drop_cursor(&mut self) {
        self.cursor = None;
        self.should_take_focus = false;
        self.should_set_pos_to_start = false;
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
            ui.push_item_width(ui.calc_text_size("f", 0).x);
            ui.push_style_var_vec(ImGuiStyleVar::FramePadding, Vec2 { x: 0.0, y: 0.0 });
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

    /// Renders char editor. Returns next cursor position and new string if it was changed.
    /// `flags` are `InputTextFlags` as i32
    /// `char_filter` is function to filter input character. Set character to `\u{0}` to cancel it.
    /// If `char_filter` is `Some`, appropriate flag will be added.
    pub fn render(&mut self,
                  ui: &mut Ui,
                  text: &str,
                  flags: PDUIInputTextFlags_,
                  char_filter: Option<&Fn(char) -> char>)
                  -> (NextPosition, Option<String>) {
        if text.len() == 0 {
            return (NextPosition::Within, None);
        }
        let cursor = match self.cursor {
            None => {
                ui.text(text);
                return (NextPosition::Within, None)
            },
            Some(c) if c < text.len() => c,
            // TODO: should it return NextPosition::Right instead?
            _ => text.len() - 1,
        };
        let mut next_position = NextPosition::Within;
        let mut buf = [text.as_bytes()[cursor], 0];
        ui.push_style_var_vec(ImGuiStyleVar::ItemSpacing, Vec2 { x: 0.0, y: 0.0 });

        // render bare text before cursor
        if cursor > 0 {
            let left = &text[0..cursor];
            ui.text(left);
            ui.same_line(0, 0);
            if ui.is_item_hovered() && ui.is_mouse_clicked(0, false) {
                self.change_cursor(get_text_cursor_index(ui, left.len()));
            }
        }

        if self.should_take_focus {
            ui.set_keyboard_focus_here(0);
            self.should_take_focus = false;
        }
        ui.push_id_usize(cursor);
        let (cursor_pos, text_has_changed) = self.render_input(ui, &mut buf, flags, char_filter);
        ui.pop_id();
        let char_count = text.len();
        if cursor_pos > 0 {
            if cursor == char_count - 1 {
                next_position = NextPosition::Right;
                self.drop_cursor();
            } else {
                self.change_cursor(cursor + 1);
            }
        }

        if cursor < char_count {
            ui.same_line(0, 0);
            let right = &text[cursor + 1..char_count];
            ui.text(right);
            if ui.is_item_hovered() && ui.is_mouse_clicked(0, false) {
                self.change_cursor(cursor + 1 + get_text_cursor_index(ui, right.len()));
            }
        }

        ui.pop_style_var(1);

        // Key processing. ImGui input can only have cursor at the start so right arrow key presses
        // are processed by ImGui, left arrow key presses are processed here.

        if ui.is_key_pressed(Key::Left, true) {
            if cursor > 0 {
                self.change_cursor(cursor - 1);
            } else {
                self.drop_cursor();
                next_position = NextPosition::Left;
            }
        }

        let changed_text = if text_has_changed {
            let changed_symbol = ::std::str::from_utf8(&buf[0..1]).ok().map(|s| s.to_owned());
            changed_symbol.map(|changed_symbol| {
                let mut res = String::with_capacity(text.len());
                res.push_str(&text[0..cursor]);
                res.push_str(&changed_symbol);
                res.push_str(&text[cursor + 1..]);
                res
            })
        } else {
            None
        };
        return (next_position, changed_text);
    }
}

impl Clone for CharEditor {
    fn clone(&self) -> CharEditor {
        CharEditor {
            cursor: self.cursor,
            should_take_focus: true,
            should_set_pos_to_start: true,
        }
    }
}