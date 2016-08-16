//! View to show registers.
//! All data that backend sends is in network (big) endianness.

#[macro_use]
extern crate prodbg_api;

mod number_view;

use prodbg_api::{View, Ui, Service, Reader, Writer, PluginHandler, CViewCallbacks, ReadStatus, EventType, PDUIWindowFlags_, ImGuiStyleVar, Vec2};
use number_view::*;


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

    pub fn group(&self, views: &Vec<NumberView>) -> Vec<Vec<NumberView>> {
        let mut res: Vec<Vec<NumberView>> = Vec::new();
        'views: for view in views {
            for group in res.iter_mut() {
                if self.same_param(group[0], *view) {
                    group.push(*view);
                    continue 'views;
                }
            }
            res.push(vec!(*view));
        }
        res
    }
}

struct RegistersView {
    registers: Vec<Register>,
    bars_byte_count: Option<usize>,
    grouping: Option<Grouping>,
    align_columns: bool,
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
                _ => {}
            }
        }
    }

    fn get_view_short_name(view: NumberView) -> String {
        format!("{}{}", view.representation.as_short_str(), view.size.as_bit_len_str())
    }

    fn render_register_data_no_alignment(&self, ui: &Ui, register: &Register, view: NumberView) {
        let text = register
            .value
            .chunks(view.size.byte_count())
            .map(|bytes| view.format(bytes))
            .collect::<Vec<String>>()
            .join(" ");
        ui.same_line(0, -1);
        ui.text(&text);
    }

    fn render_register_data(&self, ui: &Ui, register: &Register, view: NumberView, single_bar_width: usize) {
        let mut bars_byte_count = self.bars_byte_count.unwrap_or(100000);
        let bar_width = if view.size.byte_count() < bars_byte_count {
            single_bar_width
        } else {
            let columns = view.size.byte_count() / bars_byte_count;
            bars_byte_count = view.size.byte_count();
            columns * single_bar_width + (columns - 1) * 3
        };
        let chunks_per_bar = std::cmp::max(1, bars_byte_count / view.size.byte_count());
        let bar_text = register.value
            .chunks(bars_byte_count)
            .map(|bar_bytes| {
                let text = bar_bytes
                    .chunks(view.size.byte_count())
                    .map(|chunk| view.format(chunk))
                    .collect::<Vec<String>>()
                    .join(" ");
                format!("{1:>0$}", bar_width, text)
            })
            .collect::<Vec<String>>()
            .join(" | ");
        ui.same_line(0, -1);
        ui.text(&bar_text);
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

    fn render_register(&self, ui: &mut Ui, width: usize, register: &Register) {
        let default_view = NumberView {
            representation: NumberRepresentation::Hex,
            size: NumberSize::OneByte,
            endianness: Endianness::Big,
        };
        let views = Self::all_possible_views(register.value.len());
        // TODO: do not create format names for every register since they are the same.
        let format_names: Vec<String> = views
            .iter()
            .map(|view| Self::get_view_short_name(*view))
            .collect();

        let format_width = format_names.iter().map(|f| f.len()).max().unwrap_or(0);
        let render = |ui: &Ui, register: &Register, view: NumberView| {
            if let Some(bar_bytes) = self.bars_byte_count {
                let bar_width = if self.align_columns {
                    views.iter()
                        .map(|view| {
                            let bc = view.size.byte_count();
                            let res = if bc >= bar_bytes {
                                let bars = view.size.byte_count() / bar_bytes;
                                (view.maximum_chars_needed().saturating_sub((bars - 1) * 3)) / bars
                            } else {
                                let items_in_bar = bar_bytes / bc;
                                items_in_bar * view.maximum_chars_needed() + (items_in_bar - 1)
                            };
                            res
                        })
                        .max().unwrap_or(0)
                } else {
                    0
                };
                self.render_register_data(ui, register, view, bar_width)
            } else {
                self.render_register_data_no_alignment(ui, register, view)
            };
        };

        ui.push_style_var_vec(ImGuiStyleVar::FramePadding, Vec2::new(0.5, 0.0));
        ui.tree_node(&format!("{1:>0$}", width, register.name)).exec(|ui, is_expanded| {
            if !is_expanded {
                ui.same_line(0, 0);
                ui.text("  ");
                ui.same_line(0, 0);
                render(ui, register, default_view);
                return;
            }
            if let Some(grouping) = self.grouping {
                let groups = grouping.group(&views);
                for group in groups {
                    if group.len() > 1 {
                        ui.tree_node(&format!("{1:>0$}  ", format_width, Self::get_view_short_name(group[0])))
                            .exec(|ui, is_expanded| {
                                render(ui, register, group[0]);
                                if !is_expanded {
                                    return;
                                }
                                for view in group[1..].iter() {
                                    ui.text(&format!("{1:>0$}  ", format_width, Self::get_view_short_name(*view)));
                                    render(ui, register, *view);
                                }
                            });
                    } else {
                        ui.text(&format!("  {1:>0$}  ", format_width, Self::get_view_short_name(group[0])));
                        render(ui, register, group[0]);
                    }
                }
            } else {
                for (&view, name) in views.iter().zip(format_names.iter()) {
                    ui.text(&format!("{1:>0$}  ", format_width, name));
                    render(ui, register, view);
                }
            }
        });
        ui.pop_style_var(1);
    }

    fn render_bars_picker(&mut self, ui: &mut Ui) {
        const VARIANTS: [Option<usize>; 5] = [None, Some(1), Some(2), Some(4), Some(8)];
        const NAMES: [&'static str; 5] = ["No columns", "1 byte columns", "2 byte columns", "4 byte columns", "8 byte columns"];
        if let Some(val) = combo(ui, "##bars", &VARIANTS, &NAMES, &self.bars_byte_count) {
            self.bars_byte_count = *val;
        }
    }

    fn render_view_picker(&mut self, ui: &mut Ui) {
        const VARIANTS: [Option<Grouping>; 3] = [None, Some(Grouping::Size), Some(Grouping::Representation)];
        const NAMES: [&'static str; 3] = ["No grouping", "Group by size", "Group by representation"];
        if let Some(val) = combo(ui, "##grouping", &VARIANTS, &NAMES, &self.grouping) {
            self.grouping = *val;
        }
    }

    fn render_align_picker(&mut self, ui: &mut Ui) {
        ui.checkbox("Align columns", &mut self.align_columns);
    }

    fn render_header(&mut self, ui: &mut Ui) {
        self.render_view_picker(ui);
        ui.same_line(0, -1);
        self.render_bars_picker(ui);
        ui.same_line(0, -1);
        self.render_align_picker(ui);
    }

    pub fn render(&mut self, ui: &mut Ui) {
        self.render_header(ui);
        let register_name_width = self.registers.iter().map(|r| r.name.len()).max().unwrap_or(0usize);
        ui.begin_child("##body", None, false, PDUIWindowFlags_::empty());
        for register in self.registers.iter() {
            self.render_register(ui, register_name_width, register);
        }
        ui.end_child();
    }
}

impl View for RegistersView {
    fn new(_: &Ui, _: &Service) -> Self {
        RegistersView {
            registers: Vec::new(),
            bars_byte_count: None,
            grouping: None,
            align_columns: false,
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, _: &mut Writer) {
        self.process_events(reader);
        self.render(ui);
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Registers View 2\0", RegistersView);
    plugin_handler.register_view(&PLUGIN);
}
