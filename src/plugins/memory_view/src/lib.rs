#[macro_use]
extern crate prodbg_api;
#[macro_use]
extern crate serde_macros;
extern crate serde;
extern crate serde_json;

mod number_view;
mod hex_editor;
mod char_editor;
mod ascii_editor;
mod address_input;
mod helper;
mod memory_chunk;
mod state;

use prodbg_api::{View, Ui, Service, Reader, Writer, PluginHandler, CViewCallbacks, PDVec2,
                 ImGuiStyleVar, EventType, ImGuiCol, Color, ReadStatus, Key, StateSaver, StateLoader,
                 LoadResult};
use prodbg_api::PDUIWINDOWFLAGS_HORIZONTALSCROLLBAR;
use std::str;
use number_view::{NumberView, NumberRepresentation, Endianness};
use hex_editor::HexEditor;
use ascii_editor::AsciiEditor;
use address_input::AddressInput;
use helper::get_text_cursor_index;
use memory_chunk::MemoryChunk;
use state::MemoryViewState;

const START_ADDRESS: usize = 0xf0000;
const CHARS_PER_ADDRESS: usize = 10;
const TABLE_SPACING: &'static str = "  ";
const COLUMNS_SPACING: &'static str = " ";
// TODO: change to Color when `const fn` is in stable Rust
const CHANGED_DATA_COLOR: u32 = 0xff0000ff;
const LINES_PER_SCROLL: usize = 3;
// Minimal amount of bytes requested.
// TODO: 32 bit linux allows 64bit addresses. Will we work well in such situation?
const MIN_BYTES_PER_REQUEST: usize = 64 * 1024;

#[derive(Clone)]
pub enum Cursor {
    /// Number area is edited right now. `HexEditor` structure contains inner data about focusing
    /// and exact cursor position
    Number(HexEditor),
    /// Text area is edited right now. `AsciiEditor` structure contains inner data about focusing
    /// and exact cursor position
    Text(AsciiEditor),
    /// Memory is not edited right now
    None,
}

impl Cursor {
    pub fn text(&mut self) -> Option<&mut AsciiEditor> {
        match self {
            &mut Cursor::Text(ref mut e) => Some(e),
            _ => None,
        }
    }

    pub fn number(&mut self) -> Option<&mut HexEditor> {
        match self {
            &mut Cursor::Number(ref mut e) => Some(e),
            _ => None,
        }
    }

    pub fn decrease_address(&mut self, delta: usize) {
        match self {
            &mut Cursor::Text(ref mut e) => e.address = e.address.saturating_sub(delta),
            &mut Cursor::Number(ref mut e) => e.address = e.address.saturating_sub(delta),
            _ => {}
        }
    }

    pub fn increase_address(&mut self, delta: usize) {
        match self {
            &mut Cursor::Text(ref mut e) => e.address = e.address.saturating_add(delta),
            &mut Cursor::Number(ref mut e) => e.address = e.address.saturating_add(delta),
            _ => {}
        }
    }

    /// Returns memory address edited right now, if any.
    pub fn get_address(&self) -> Option<usize> {
        match self {
            &Cursor::Text(ref e) => Some(e.address),
            &Cursor::Number(ref e) => Some(e.address),
            _ => None,
        }
    }

    pub fn set_address(&mut self, address: usize) {
        match self {
            &mut Cursor::Text(ref mut e) => e.address = address,
            &mut Cursor::Number(ref mut e) => {
                e.address = address;
                e.cursor = 0;
            }
            _ => {}
        }
    }
}

struct MemoryView {
    /// Address of first byte of memory shown
    start_address: AddressInput,
    /// Amount of bytes needed to fill one screen
    bytes_needed: usize,
    /// Current state of memory
    data: MemoryChunk,
    /// Snapshotted state of memory
    prev_data: MemoryChunk,
    /// Set to force memory update
    should_update_memory: bool,
    /// Number of columns shown (if number view is on) or number of bytes shown
    columns: usize,
    /// Cursor of memory editor
    cursor: Cursor,
    /// Picked number view
    number_view: Option<NumberView>,
    /// Picked text view (currently on/off since only ascii text view is available)
    text_shown: bool,
}

/// A wrapper to render a combo. Matches `variants` and `strings`, returning one of `variants` if
/// new item was selected in combo. Uses maximal string width as a combo width.
pub fn combo<'a, T>(ui: &mut Ui,
                    id: &str,
                    variants: &'a [T],
                    strings: &[&str],
                    cur: &T)
                    -> Option<&'a T>
    where T: PartialEq
{

    if variants.len() != strings.len() {
        panic!("Variants and strings length should be equal in combo");
    }
    if variants.is_empty() {
        panic!("Variants cannot be empty in combo");
    }
    let mut res = None;
    let mut current_item = variants.iter().position(|var| var == cur).unwrap_or(0);
    let width = strings.iter().map(|s| ui.calc_text_size(s, 0).0 as i32 + 40).max().unwrap_or(200);
    ui.push_item_width(width as f32);
    if ui.combo(id,
                &mut current_item,
                &strings,
                strings.len(),
                strings.len()) {
        res = Some(&variants[current_item]);
    }
    ui.pop_item_width();
    res
}

impl MemoryView {
    fn render_address(ui: &mut Ui, address: usize) {
        ui.text(&format!("{:#0width$x}", address, width = CHARS_PER_ADDRESS));
    }

    fn render_const_number(ui: &mut Ui, text: &str) -> Option<usize> {
        ui.text(text);
        if ui.is_item_hovered() && ui.is_mouse_clicked(0, false) {
            return Some(get_text_cursor_index(ui, text.len()));
        } else {
            return None;
        }
    }

    fn render_inaccessible_memory(ui: &mut Ui, char_count: usize) {
        let mut text = String::with_capacity(char_count);
        for _ in 0..char_count {
            text.push('?');
        }
        ui.text(&text);
    }

    fn render_ascii_string(ui: &mut Ui,
                           mut address: usize,
                           data: &mut [u8],
                           prev_data: &[u8],
                           char_count: usize,
                           mut editor: Option<&mut AsciiEditor>)
                           -> (Option<AsciiEditor>, Option<(usize, usize)>) {
        let mut bytes = data.iter_mut();
        let mut prev_bytes = prev_data.iter();
        let mut next_editor = None;
        let mut changed_data = None;
        for _ in 0..char_count {
            let mut cur_char = bytes.next();
            let prev_char = prev_bytes.next();
            let mut is_marked = false;
            if let Some(ref cur) = cur_char {
                if let Some(ref prev) = prev_char {
                    is_marked = cur != prev;
                }
            }
            if is_marked {
                ui.push_style_color(ImGuiCol::Text, Color::from_u32(CHANGED_DATA_COLOR));
            }
            let mut is_editor = false;
            ui.same_line(0, -1);
            if let Some(ref mut c) = cur_char {
                if let Some(ref mut e) = editor {
                    if e.address == address {
                        is_editor = true;
                        let (pos, has_changed) = e.render(ui, c);
                        if has_changed {
                            changed_data = Some((address, 1));
                        }
                        next_editor = next_editor.or(pos.map(|address| AsciiEditor::new(address)));
                    }
                }
            }
            if !is_editor {
                match cur_char {
                    Some(byte) => {
                        match *byte {
                            32...127 => ui.text(unsafe { std::str::from_utf8_unchecked(&[*byte]) }),
                            _ => ui.text("."),
                        }
                        if ui.is_item_hovered() && ui.is_mouse_clicked(0, false) {
                            next_editor = next_editor.or_else(|| Some(AsciiEditor::new(address)));
                        }
                    }
                    None => ui.text("?"),
                };
            }
            if is_marked {
                ui.pop_style_color(1);
            }
            address += 1;
        }
        (next_editor, changed_data)
    }

    fn set_memory(writer: &mut Writer, address: usize, data: &[u8]) {
        writer.event_begin(EventType::UpdateMemory as u16);
        writer.write_u64("address", address as u64);
        writer.write_data("data", data);
        writer.event_end();
    }

    fn render_numbers(ui: &mut Ui,
                      mut editor: Option<&mut HexEditor>,
                      address: usize,
                      data: &mut [u8],
                      prev_data: &[u8],
                      view: NumberView,
                      columns: usize)
                      -> (Option<HexEditor>, Option<(usize, usize)>) {
        let bytes_per_unit = view.size.byte_count();
        let mut next_editor = None;
        let mut changed_data = None;
        let mut cur_address = address;
        {
            let mut data_chunks = data.chunks_mut(bytes_per_unit);
            let mut prev_data_chunks = prev_data.chunks(bytes_per_unit);
            for column in 0..columns {
                ui.same_line(0, -1);
                match data_chunks.next() {
                    Some(ref mut unit) if unit.len() == bytes_per_unit => {
                        let has_changed = match prev_data_chunks.next() {
                            Some(ref prev_unit) if prev_unit.len() == bytes_per_unit => {
                                unit != prev_unit
                            }
                            _ => false,
                        };
                        if has_changed {
                            ui.push_style_color(ImGuiCol::Text,
                                                Color::from_u32(CHANGED_DATA_COLOR));
                        }
                        let mut is_editor = false;
                        if let Some(ref mut e) = editor {
                            if e.address == cur_address {
                                let (np, data_edited) = e.render(ui, *unit);
                                next_editor = next_editor.or(np.map(|(address, cursor)| {
                                    HexEditor::new(address, cursor, view)
                                }));
                                if data_edited {
                                    changed_data = Some((cur_address, bytes_per_unit));
                                }
                                is_editor = true;
                            }
                        }
                        if !is_editor {
                            if let Some(index) =
                                   MemoryView::render_const_number(ui, &view.format(*unit)) {
                                next_editor =
                                    next_editor.or(Some(HexEditor::new(cur_address, index, view)));
                            }
                        }
                        if has_changed {
                            ui.pop_style_color(1);
                        }
                    }
                    _ => MemoryView::render_inaccessible_memory(ui, view.maximum_chars_needed()),
                }
                if column < columns - 1 {
                    ui.same_line(0, -1);
                    ui.text(COLUMNS_SPACING);
                }
                cur_address += bytes_per_unit as usize;
            }
        }
        (next_editor, changed_data)
    }

    fn render_line(cursor: &mut Cursor,
                   ui: &mut Ui,
                   address: usize,
                   data: &mut [u8],
                   prev_data: &[u8],
                   view: Option<NumberView>,
                   writer: &mut Writer,
                   columns: usize,
                   text_shown: bool)
                   -> Option<Cursor> {
        // TODO: Hide cursor when user clicks somewhere else
        MemoryView::render_address(ui, address);

        let mut new_data = None;
        let mut res = None;
        if let Some(view) = view {
            ui.same_line(0, -1);
            ui.text(TABLE_SPACING);
            let (hex_editor, hex_data) = MemoryView::render_numbers(ui,
                                                                    cursor.number(),
                                                                    address,
                                                                    data,
                                                                    prev_data,
                                                                    view,
                                                                    columns);
            res = res.or(hex_editor.map(|editor| Cursor::Number(editor)));
            new_data = new_data.or(hex_data);
        }
        if text_shown {
            ui.same_line(0, -1);
            ui.text(TABLE_SPACING);
            let line_len = columns *
                           match view {
                Some(ref v) => v.size.byte_count(),
                _ => 1,
            };
            let (ascii_editor, ascii_data) = MemoryView::render_ascii_string(ui,
                                                                             address,
                                                                             data,
                                                                             prev_data,
                                                                             line_len,
                                                                             cursor.text());
            res = res.or_else(|| ascii_editor.map(|editor| Cursor::Text(editor)));
            new_data = new_data.or(ascii_data);
        }
        if let Some((abs_address, size)) = new_data {
            let offset = abs_address - address;
            MemoryView::set_memory(writer, abs_address, &data[offset..offset + size]);
        }
        return match view {
            Some(v) if v.representation == NumberRepresentation::Hex => res,
            _ => None,
        };
    }

    fn change_number_view(&mut self, view: Option<NumberView>) {
        if view != self.number_view {
            self.number_view = view;
            self.cursor = Cursor::None;
        }
    }

    fn render_number_view_picker(&mut self, ui: &mut Ui) {
        let mut res_view = self.number_view;
        let variants = [None,
                        Some(NumberRepresentation::Hex),
                        Some(NumberRepresentation::UnsignedDecimal),
                        Some(NumberRepresentation::SignedDecimal),
                        Some(NumberRepresentation::Float)];
        let strings = ["Off",
                       NumberRepresentation::Hex.as_str(),
                       NumberRepresentation::UnsignedDecimal.as_str(),
                       NumberRepresentation::SignedDecimal.as_str(),
                       NumberRepresentation::Float.as_str()];
        let cur_repr = res_view.map(|v| v.representation);
        if let Some(repr) = combo(ui,
                                  "##number_representation",
                                  &variants,
                                  &strings,
                                  &cur_repr) {
            match *repr {
                None => res_view = None,
                Some(r) => {
                    res_view = self.number_view.or(Some(NumberView::default()));
                    if let Some(ref mut v) = res_view {
                        v.change_representation(r);
                    }
                }
            }
        }

        if let Some(ref mut view) = res_view {
            let variants = view.representation.get_avaialable_sizes();
            let strings: Vec<&str> = variants.iter().map(|size| size.as_str()).collect();
            ui.same_line(0, -1);
            if let Some(size) = combo(ui, "##number_size", &variants, &strings, &view.size) {
                view.size = *size;
            }

            let variants = [Endianness::Little, Endianness::Big];
            let strings = [Endianness::Little.as_str(), Endianness::Big.as_str()];
            ui.same_line(0, -1);
            if let Some(e) = combo(ui, "##endianness", &variants, &strings, &view.endianness) {
                view.endianness = *e;
            }
        }
        self.change_number_view(res_view);
    }

    fn render_columns_picker(&mut self, ui: &mut Ui) {
        let variants = [0, 1, 2, 4, 8, 16, 32, 64, 128];
        let strings = ["Fit width",
                       "1 column",
                       "2 columns",
                       "4 columns",
                       "8 columns",
                       "16 columns",
                       "32 columns",
                       "64 columns",
                       "128 columns"];
        if let Some(columns) = combo(ui, "##byte_per_line", &variants, &strings, &self.columns) {
            self.columns = *columns;
        }
    }

    fn render_header(&mut self, ui: &mut Ui) {
        if self.start_address.render(ui) {
            let new_address = self.start_address.get();
            self.cursor.set_address(new_address);
        }
        ui.same_line(0, -1);
        self.render_number_view_picker(ui);
        ui.same_line(0, -1);
        self.render_columns_picker(ui);
        ui.same_line(0, -1);
        ui.checkbox("Show text", &mut self.text_shown);
    }

    fn process_step(&mut self) {
        std::mem::swap(&mut self.data, &mut self.prev_data);
        self.should_update_memory = true;
    }

    fn process_events(&mut self, reader: &mut Reader) {
        for event_type in reader.get_events() {
            match event_type {
                et if et == EventType::SetMemory as i32 => {
                    if let Err(e) = self.update_memory(reader) {
                        panic!("Could not update memory: {:?}", e);
                    }
                }
                et if et == EventType::SetExceptionLocation as i32 => {
                    self.process_step();
                }
                _ => {}
            }
        }
    }

    fn update_memory(&mut self, reader: &mut Reader) -> Result<(), ReadStatus> {
        let address = try!(reader.find_u64("address")) as usize;
        let data = try!(reader.find_data("data"));
        self.data.set_accessible(address, data);
        self.prev_data.extend_accessible(address, data);
        Ok(())
    }

    /// Returns maximum amount of bytes that could be rendered within window width
    /// Minimum number of columns reported is 1.
    fn get_columns_from_width(&self, ui: &Ui) -> usize {
        // TODO: ImGui reports inaccurate glyph size. Find a better way to find chars_in_screen.
        let glyph_size = ui.calc_text_size("ff", 0).0 / 2.0;
        let mut chars_left = (ui.get_window_size().0 / glyph_size) as usize;
        // Number of large columns (for numbers, text)
        let mut large_columns: usize = 0;
        // Number of chars per one rendered column
        let mut chars_per_column = 0;
        if let Some(ref view) = self.number_view {
            large_columns += 1;
            // Every number is fixed number of chars + spacing between them
            chars_per_column += view.maximum_chars_needed() + COLUMNS_SPACING.len();
        }
        if self.text_shown {
            large_columns += 1;
            // One char per byte.
            chars_per_column += match self.number_view {
                Some(ref view) => view.size.byte_count(),
                None => 1,
            }
        }
        chars_left =
            chars_left.saturating_sub(large_columns * TABLE_SPACING.len() + CHARS_PER_ADDRESS);
        if chars_per_column > 0 {
            std::cmp::max(chars_left / chars_per_column, 1)
        } else {
            // Neither number nor text view is shown
            1
        }
    }

    fn get_screen_lines_count(ui: &Ui) -> usize {
        let line_height = ui.get_text_line_height_with_spacing();
        let (start, end) = ui.calc_list_clipping(line_height);
        // Strip last line to make sure vertical scrollbar will not appear
        end.saturating_sub(start + 1)
    }

    fn handle_cursor_move_keys(&mut self, ui: &Ui, bytes_per_line: usize) -> Option<Cursor> {
        if ui.is_key_pressed(Key::Up, true) {
            let mut cursor = self.cursor.clone();
            cursor.decrease_address(bytes_per_line);
            return Some(cursor);
        }
        if ui.is_key_pressed(Key::Down, true) {
            let mut cursor = self.cursor.clone();
            cursor.increase_address(bytes_per_line);
            return Some(cursor);
        }
        None
    }

    fn handle_scroll_keys(&mut self, ui: &Ui, bytes_per_line: usize, lines_on_screen: usize) {
        let address = self.start_address.get();
        let mut new_address = None;
        if ui.is_key_pressed(Key::PageUp, true) {
            new_address = Some(address.saturating_sub(bytes_per_line * lines_on_screen));
        }
        if ui.is_key_pressed(Key::PageDown, true) {
            new_address = Some(address.saturating_add(bytes_per_line * lines_on_screen));
        }
        let wheel = ui.get_mouse_wheel();
        if wheel > 0.0 {
            new_address = Some(address.saturating_sub(bytes_per_line * LINES_PER_SCROLL));
        }
        if wheel < 0.0 {
            new_address = Some(address.saturating_add(bytes_per_line * LINES_PER_SCROLL));
        }
        if let Some(new_address) = new_address {
            self.start_address.set(new_address);
        }
    }

    fn follow_cursor(&mut self, bytes_per_line: usize, lines_on_screen: usize) {
        if let Some(address) = self.cursor.get_address() {
            let start_address = self.start_address.get();
            if address < start_address {
                let lines_needed = (start_address - address + bytes_per_line - 1) / bytes_per_line;
                self.start_address.set(start_address.saturating_sub(lines_needed * bytes_per_line));
            }
            let last_address =
                self.start_address.get().saturating_add(bytes_per_line * lines_on_screen);
            if address >= last_address {
                let lines_needed = (address - last_address) / bytes_per_line + 1;
                self.start_address.set(start_address.saturating_add(lines_needed * bytes_per_line));
            }
        }
    }

    fn render(&mut self, ui: &mut Ui, writer: &mut Writer) {
        self.render_header(ui);
        let columns = match self.columns {
            0 => self.get_columns_from_width(ui),
            x => x,
        };
        let bytes_per_line = columns *
                             match self.number_view {
            Some(ref view) => view.size.byte_count(),
            None => 1,
        };

        ui.push_style_var_vec(ImGuiStyleVar::ItemSpacing, PDVec2 { x: 0.0, y: 0.0 });
        ui.begin_child("##lines", None, false, PDUIWINDOWFLAGS_HORIZONTALSCROLLBAR);

        let lines_needed = MemoryView::get_screen_lines_count(ui);
        self.bytes_needed = bytes_per_line * lines_needed;

        let mut address = self.start_address.get();
        let mut next_cursor = None;
        {
            let mut lines_slice = self.data.slice_mut(self.start_address.get(), self.bytes_needed);
            let mut lines = lines_slice.chunks_mut(bytes_per_line);
            let mut prev_lines_slice = self.prev_data
                .slice_mut(self.start_address.get(), self.bytes_needed);
            let mut prev_lines = prev_lines_slice.chunks_mut(bytes_per_line);
            for _ in 0..lines_needed {
                let line = lines.next().unwrap_or(&mut []);
                let prev_line = prev_lines.next().unwrap_or(&mut []);
                next_cursor = next_cursor.or(MemoryView::render_line(&mut self.cursor,
                                                                     ui,
                                                                     address,
                                                                     line,
                                                                     prev_line,
                                                                     self.number_view,
                                                                     writer,
                                                                     columns,
                                                                     self.text_shown));
                address += bytes_per_line;
            }
        }

        ui.end_child();
        ui.pop_style_var(1);

        next_cursor = next_cursor.or_else(|| self.handle_cursor_move_keys(ui, bytes_per_line));

        if let Some(cursor) = next_cursor {
            self.cursor = cursor;
            self.follow_cursor(bytes_per_line, lines_needed);
        }
        self.handle_scroll_keys(ui, bytes_per_line, lines_needed);
    }

    fn process_memory_request(&mut self, writer: &mut Writer) {
        let start = self.start_address.get();
        // Make sure we have 1 screen of data available in both directions.
        let start_border = start.saturating_sub(self.bytes_needed);
        let end_border = start.saturating_add(2 * self.bytes_needed);

        let data_end = self.data.start() + self.data.len();
        if start_border < self.data.start() || end_border > data_end {
            self.should_update_memory = true;
        }
        if self.should_update_memory {
            let left_data_amount = MIN_BYTES_PER_REQUEST.saturating_sub(end_border - start_border);
            let request_start = start_border.saturating_sub(left_data_amount / 2);
            let len = std::cmp::max(MIN_BYTES_PER_REQUEST, end_border - start_border);
            writer.event_begin(EventType::GetMemory as u16);
            writer.write_u64("address_start", request_start as u64);
            writer.write_u64("size", len as u64);
            writer.event_end();
            self.data.transform(request_start, len);
            self.prev_data.transform(request_start, len);
            self.should_update_memory = false;
        }
    }

    fn to_state(&self) -> MemoryViewState {
        MemoryViewState {
            start_address: self.start_address.get(),
            columns: self.columns,
            number_view: self.number_view.clone(),
            text_shown: self.text_shown,
        }
    }

    fn restore_from_state(&mut self, state: MemoryViewState) {
        self.start_address.set(state.start_address);
        self.columns = state.columns;
        self.change_number_view(state.number_view);
        self.text_shown = state.text_shown;
    }
}

impl View for MemoryView {
    fn new(_: &Ui, _: &Service) -> Self {
        MemoryView {
            start_address: AddressInput::new(START_ADDRESS),
            data: MemoryChunk::new(),
            prev_data: MemoryChunk::new(),
            should_update_memory: false,
            bytes_needed: 0,
            columns: 0,
            cursor: Cursor::None,
            number_view: Some(NumberView::default()),
            text_shown: true,
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, writer: &mut Writer) {
        self.process_events(reader);
        self.render(ui, writer);
        self.process_memory_request(writer);
    }

    fn save_state(&mut self, mut saver: StateSaver) {
        let state = self.to_state().to_string();
        saver.write_str(&state);
    }

    fn load_state(&mut self, mut loader: StateLoader) {
        if let LoadResult::Ok(source) = loader.read_string() {
            match MemoryViewState::from_str(&source) {
                Ok(state) => self.restore_from_state(state),
                Err(err) => println!("Could not restore memory view state: {}", err),
            }
        }
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Memory View\0", MemoryView);
    plugin_handler.register_view(&PLUGIN);
}
