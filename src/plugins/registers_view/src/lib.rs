//! View to show registers.
//! All data that backend sends is in network (big) endianness.

#[macro_use]
extern crate prodbg_api;

mod number_view;
mod char_editor;
mod hex_editor;
mod helper;

use prodbg_api::{View, Ui, Service, Reader, Writer, PluginHandler, CViewCallbacks, ReadStatus, EventType, PDUIWindowFlags_, ImGuiStyleVar, Vec2};
use number_view::*;
use hex_editor::HexEditor;
use char_editor::NextPosition;
use helper::get_text_cursor_index;


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
    let width = strings.iter().map(|s| ui.calc_text_size(s, 0).x as i32 + 40).max().unwrap_or(200);
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


#[derive(Debug)]
struct Register {
    name: String,
    read_only: bool,
    value: Vec<u8>,
}

#[derive(Debug, Clone, Copy, PartialEq)]
enum Grouping {
    Size,
    Representation,
}

impl Grouping {
    fn same_param(&self, a: NumberView, b: NumberView) -> bool {
        match *self {
            Grouping::Size => a.size == b.size,
            Grouping::Representation => a.representation == b.representation,
        }
    }

    pub fn group(&self, views: &Vec<NumberView>) -> Vec<Vec<(usize, NumberView)>> {
        let mut res: Vec<Vec<(usize, NumberView)>> = Vec::new();
        'views: for (i, view) in views.iter().enumerate() {
            for group in res.iter_mut() {
                if self.same_param(group[0].1, *view) {
                    group.push((i, *view));
                    continue 'views;
                }
            }
            res.push(vec!((i, *view)));
        }
        res
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
enum Alignment {
    None,
    Visible,
    All,
}

#[derive(Debug, Clone)]
struct EditingCursor {
    register_name: String,
    // TODO: remove `view` as it is already in `editor`
    view: NumberView,
    chunk: usize,
    editor: HexEditor,
}

struct RegistersSettings {
    column_byte_count: Option<usize>,
    grouping: Option<Grouping>,
    alignment: Alignment,
    default_view: NumberView,
}

impl RegistersSettings {
    fn render_view_picker(&mut self, ui: &mut Ui) {
        const VARIANTS: [Option<Grouping>; 3] = [None, Some(Grouping::Size), Some(Grouping::Representation)];
        const NAMES: [&'static str; 3] = ["No grouping", "Group by size", "Group by representation"];
        if let Some(val) = combo(ui, "##grouping", &VARIANTS, &NAMES, &self.grouping) {
            self.grouping = *val;
        }
    }

    fn render_alignment_picker(&mut self, ui: &mut Ui) {
        const VARIANTS: [Alignment; 3] = [Alignment::None, Alignment::Visible, Alignment::All];
        const NAMES: [&'static str; 3] = ["Do not align", "Align visible", "Align all"];
        if let Some(val) = combo(ui, "##alignemnt", &VARIANTS, &NAMES, &self.alignment) {
            self.alignment = *val;
        }
    }

    fn render_bars_picker(&mut self, ui: &mut Ui) {
        const VARIANTS: [Option<usize>; 5] = [None, Some(1), Some(2), Some(4), Some(8)];
        const NAMES: [&'static str; 5] = ["No columns", "1 byte columns", "2 byte columns", "4 byte columns", "8 byte columns"];
        if let Some(val) = combo(ui, "##bars", &VARIANTS, &NAMES, &self.column_byte_count) {
            self.column_byte_count = *val;
        }
    }

    fn render_default_view_picker(&mut self, ui: &mut Ui) {
        let variants: [NumberRepresentation; 4] = [
                        NumberRepresentation::Hex,
                        NumberRepresentation::UnsignedDecimal,
                        NumberRepresentation::SignedDecimal,
                        NumberRepresentation::Float];
        let strings: Vec<&str> = variants.iter().map(|repr| repr.as_str()).collect();
        if let Some(repr) = combo(ui,
                                  "##number_representation",
                                  &variants,
                                  &strings,
                                  &self.default_view.representation) {

            self.default_view.change_representation(*repr);
        }

        let variants = self.default_view.representation.get_avaialable_sizes();
        let strings: Vec<&str> = variants.iter().map(|size| size.as_str()).collect();
        ui.same_line(0, -1);
        if let Some(size) = combo(ui, "##number_size", &variants, &strings, &self.default_view.size) {
            self.default_view.size = *size;
        }
    }

    fn get_view_short_name(view: NumberView) -> String {
        format!("{}{}", view.representation.as_short_str(), view.size.as_bit_len_str())
    }

    fn render_chunk(ui: &mut Ui, name: &str, view: NumberView, chunk_num: usize, bytes: &mut [u8], cursor: &mut Option<EditingCursor>) -> (NextPosition<usize>, bool) {
        match cursor {
            &mut Some(ref mut c) if c.chunk == chunk_num && c.register_name == name && c.view == view && view.representation == NumberRepresentation::Hex => {
                c.editor.render(ui, bytes)
            },
            _ => {
                let text = view.format(bytes);
                ui.text(&text);
                let pos = if ui.is_item_hovered() && ui.is_mouse_clicked(0, false) {
                    NextPosition::Changed(get_text_cursor_index(ui, text.len()))
                } else {
                    NextPosition::Unchanged
                };
                (pos, false)
            }
        }
    }

    fn render_register_data(&self, ui: &mut Ui, register: &mut Register, view: NumberView, single_bar_width: usize, cursor: &mut Option<EditingCursor>, writer: &mut Writer) -> Option<EditingCursor> {
        let mut column_byte_count = self.column_byte_count.unwrap_or(100000);
        let mut res = None;
        let bar_width = if view.size.byte_count() < column_byte_count {
            single_bar_width
        } else {
            let columns = view.size.byte_count() / column_byte_count;
            column_byte_count = view.size.byte_count();
            columns * single_bar_width + (columns - 1) * 3
        };
        let pieces = column_byte_count / view.size.byte_count();
        let leftover = bar_width.saturating_sub(pieces * view.maximum_chars_needed() + pieces - 1);
        let chunks_count = register.value.len() / view.size.byte_count();
        let mut register_is_changed = false;
        for (i, bytes) in register.value
            .chunks_mut(view.size.byte_count())
            .enumerate() {

            let is_start_of_column = pieces == 0 || i % pieces == 0;
            if i > 0 && is_start_of_column {
                ui.same_line(0, 0);
                ui.text(" | ");
            }
            if is_start_of_column {
                for _ in 0..leftover {
                    ui.same_line(0, 0);
                    ui.text(" ");
                }
            } else {
                ui.same_line(0, 0);
                ui.text(" ");
            }
            ui.same_line(0, 0);
            let (next_pos, is_changed) = Self::render_chunk(ui, &register.name, view, i, bytes, cursor);
            if !register.read_only {
                match next_pos {
                    NextPosition::Changed(pos) => {
                        res = Some(EditingCursor {
                            register_name: register.name.clone(),
                            view: view,
                            chunk: i,
                            editor: HexEditor::new(pos, view),
                        })
                    },
                    NextPosition::Left if i > 0 => {
                        res = Some(EditingCursor {
                            register_name: register.name.clone(),
                            view: view,
                            chunk: i - 1,
                            editor: HexEditor::new(view.maximum_chars_needed() - 1, view),
                        })
                    },
                    NextPosition::Right if i < chunks_count - 1 => {
                        res = Some(EditingCursor {
                            register_name: register.name.clone(),
                            view: view,
                            chunk: i + 1,
                            editor: HexEditor::new(0, view),
                        })
                    }
                    _ => {}
                }
            }
            register_is_changed = register_is_changed || is_changed;
        }
        if register_is_changed {
            RegistersView::set_register(register, writer);
        }
        res
    }

    fn all_possible_views(size: usize) -> Vec<NumberView> {
        static ALL_REPRESENTATIONS: [NumberRepresentation; 4] = [NumberRepresentation::Hex, NumberRepresentation::UnsignedDecimal, NumberRepresentation::SignedDecimal, NumberRepresentation::Float];
        static ALL_SIZES: [NumberSize; 4] = [NumberSize::OneByte, NumberSize::TwoBytes, NumberSize::FourBytes, NumberSize::EightBytes];
        let mut views = Vec::new();
        for number_size in ALL_SIZES.iter().filter(|number_size| number_size.byte_count() <= size) {
            for repr in ALL_REPRESENTATIONS.iter().filter(|repr| repr.can_be_of_size(*number_size)) {
                views.push(NumberView {
                    representation: *repr,
                    size: *number_size,
                    endianness: Endianness::Big,
                });
            }
        }
        views
    }

    fn get_view_column_width(view: NumberView, column_byte_count: usize) -> usize {
        let bc = view.size.byte_count();
        if bc >= column_byte_count {
            let bars = bc / column_byte_count;
            (view.maximum_chars_needed().saturating_sub((bars - 1) * 3)) / bars
        } else {
            let items_in_bar = column_byte_count / bc;
            items_in_bar * view.maximum_chars_needed() + (items_in_bar - 1)
        }
    }

    fn get_column_width(&self, views: &Vec<NumberView>, shown_views: &Vec<bool>) -> usize {
        let column_byte_count = match self.column_byte_count {
            Some(bytes) => bytes,
            None => return 0,
        };
        match self.alignment {
            Alignment::None => 0,
            Alignment::Visible => views
                .iter()
                .zip(shown_views.iter())
                .filter(|&(_, &is_shown)| is_shown)
                .map(|(&view, _)| Self::get_view_column_width(view, column_byte_count))
                .max()
                .unwrap_or(0),
            Alignment::All => views
                .iter()
                .map(|&view| Self::get_view_column_width(view, column_byte_count))
                .max()
                .unwrap_or(0),
        }
    }

    fn render_register(&self, ui: &mut Ui, width: usize, register: &mut Register, shown_views: &mut Vec<bool>, cursor: &mut Option<EditingCursor>, writer: &mut Writer) -> Option<EditingCursor> {
        const DEFAULT_VIEW: NumberView = NumberView {
            representation: NumberRepresentation::Hex,
            size: NumberSize::OneByte,
            endianness: Endianness::Big,
        };
        let views = Self::all_possible_views(register.value.len());
        shown_views.resize(views.len(), false);
        // TODO: do not create format names for every register since they are the same.
        let format_names: Vec<String> = views
            .iter()
            .map(|view| Self::get_view_short_name(*view))
            .collect();

        let format_width = format_names.iter().map(|f| f.len()).max().unwrap_or(0);
        let column_width = self.get_column_width(&views, shown_views);
        for is_shown in shown_views.iter_mut() {
            *is_shown = false;
        }

        ui.push_style_var_vec(ImGuiStyleVar::FramePadding, Vec2::new(0.5, 0.0));
        let res = ui.tree_node(&format!("{1:>0$}", width, register.name)).exec(move |ui, is_expanded| {
            let default_view = if register.value.len() >= self.default_view.size.byte_count() {
                self.default_view
            } else {
                DEFAULT_VIEW
            };
            if !is_expanded {
                ui.same_line(0, 0);
                ui.text("  ");
                ui.same_line(0, 0);
                return self.render_register_data(ui, register, default_view, column_width, cursor, writer)
            }
            let mut res = None;
            if let Some(grouping) = self.grouping {
                let groups = grouping.group(&views);
                for group in groups {
                    shown_views[group[0].0] = true;
                    if group.len() > 1 {
                        res = res.or(ui.tree_node(&format!("{1:>0$}  ", format_width, Self::get_view_short_name(group[0].1)))
                            .exec(|ui, is_expanded| {
                                let mut res = self.render_register_data(ui, register, group[0].1, column_width, cursor, writer);
                                if is_expanded {
                                    for &(i, view) in group[1..].iter() {
                                        shown_views[i] = true;
                                        ui.text( & format!("{1:>0$}  ", format_width, Self::get_view_short_name(view)));
                                        res = res.or(self.render_register_data(ui, register, view, column_width, cursor, writer));
                                    }
                                }
                                res
                            }));
                    } else {
                        ui.text(&format!("  {1:>0$}  ", format_width, Self::get_view_short_name(group[0].1)));
                        res = res.or(self.render_register_data(ui, register, group[0].1, column_width, cursor, writer));
                    }
                }
            } else {
                for i in 0..views.len() {
                    shown_views[i] = true;
                    ui.text(&format!("{1:>0$}  ", format_width, format_names[i]));
                    res = res.or(self.render_register_data(ui, register, views[i], column_width, cursor, writer));
                }
            }
            res
        });
        ui.pop_style_var(1);
        res
    }
}

struct RegistersView {
    registers: Vec<Register>,
    shown_register_views: Vec<Vec<bool>>,
    settings: RegistersSettings,
    cursor: Option<EditingCursor>,
    should_update: bool,
}

impl RegistersView {
    fn update_registers(&mut self, reader: &mut Reader) -> Result<(), ReadStatus> {
        let mut new_registers = Vec::new();
        for reg_data in reader.find_array("registers") {
            let name = try!(reg_data.find_string("name")).to_string();
            let read_only = reg_data.find_u8("read_only").unwrap_or(0) != 0;
            let register = try!(reg_data.find_data("register"));
            let mut data = Vec::new();
            data.extend_from_slice(register);
            new_registers.push(Register {
                name: name,
                read_only: read_only,
                value: data,
            });
        }
        self.registers = new_registers;
        Ok(())
    }

    pub fn process_events(&mut self, reader: &mut Reader) {
        for event_type in reader.get_events() {
            match event_type {
                et if et == EventType::SetRegisters as i32 => {
                    if let Err(e) = self.update_registers(reader) {
                        panic!("Could not update registers: {:?}", e);
                    }
                }
                et if et == EventType::SetExceptionLocation as i32 => {
                    self.should_update = true;
                }
                _ => {}
            }
        }
    }

    fn render_header(&mut self, ui: &mut Ui) {
        self.settings.render_view_picker(ui);
        ui.same_line(0, -1);
        self.settings.render_bars_picker(ui);
        ui.same_line(0, -1);
        self.settings.render_alignment_picker(ui);
        ui.same_line(0, -1);
        self.settings.render_default_view_picker(ui);
    }

    pub fn render(&mut self, ui: &mut Ui, writer: &mut Writer) {
        self.render_header(ui);
        let register_name_width = self.registers.iter().map(|r| r.name.len()).max().unwrap_or(0usize);
        ui.begin_child("##body", None, false, PDUIWindowFlags_::empty());
        ui.push_style_var_vec(ImGuiStyleVar::FramePadding, Vec2::new(0.5, 0.0));
        let mut cursor = None;
        self.shown_register_views.resize(self.registers.len(), Vec::new());
        for (register, shown_views) in self.registers.iter_mut()
            .zip(self.shown_register_views.iter_mut()) {

            cursor = cursor.or(self.settings.render_register(ui, register_name_width, register, shown_views, &mut self.cursor, writer));
        }
        if cursor.is_some() {
            self.cursor = cursor;
        }
        ui.pop_style_var(1);
        ui.end_child();
    }

    pub fn set_register(register: &Register, writer: &mut Writer) {
        writer.event_begin(EventType::UpdateRegister as u16);
        writer.write_string("name", &register.name);
        writer.write_data("data", &register.value);
        writer.event_end();
    }

    pub fn request_registers(&mut self, writer: &mut Writer) {
        writer.event_begin(EventType::GetRegisters as u16);
        writer.event_end();
        self.should_update = false;
    }
}

impl View for RegistersView {
    fn new(_: &Ui, _: &Service) -> Self {
        RegistersView {
            registers: Vec::new(),
            shown_register_views: Vec::new(),
            settings: RegistersSettings {
                column_byte_count: None,
                grouping: None,
                alignment: Alignment::None,
                default_view: NumberView {
                    representation: NumberRepresentation::Hex,
                    size: NumberSize::OneByte,
                    endianness: Endianness::Big,
                },
            },
            cursor: None,
            should_update: true
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, writer: &mut Writer) {
        self.process_events(reader);
        self.render(ui, writer);
        if self.should_update {
            self.request_registers(writer);
        }
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Registers View 2\0", RegistersView);
    plugin_handler.register_view(&PLUGIN);
}
