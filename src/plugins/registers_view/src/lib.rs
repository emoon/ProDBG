//! View to show registers.
//! All data that backend sends is in network (big) endianness.

#[macro_use]
extern crate prodbg_api;

mod number_view;

use prodbg_api::{View, Ui, Service, Reader, Writer, PluginHandler, CViewCallbacks, ReadStatus, EventType};
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

struct RegistersView {
    registers: Vec<Register>,
    bars_byte_count: Option<usize>,
    group_by_size: bool,
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

    fn render_register(&self, ui: &mut Ui, width: usize, register: &Register) {
        static ALL_REPRESENTATIONS: [NumberRepresentation; 4] = [NumberRepresentation::Hex, NumberRepresentation::UnsignedDecimal, NumberRepresentation::SignedDecimal, NumberRepresentation::Float];
        static ALL_SIZES: [NumberSize; 4] = [NumberSize::OneByte, NumberSize::TwoBytes, NumberSize::FourBytes, NumberSize::EightBytes];
        let default_view = NumberView {
            representation: NumberRepresentation::Hex,
            size: NumberSize::OneByte,
            endianness: Endianness::Big,
        };
        let mut views = Vec::new();
        for size in ALL_SIZES.iter().filter(|size| size.byte_count() <= register.value.len()) {
            let cur_views: Vec<NumberView> = ALL_REPRESENTATIONS
                .iter()
                .filter(|repr| repr.can_be_of_size(*size))
                .map(|&repr| NumberView {
                    representation: repr,
                    size: *size,
                    endianness: Endianness::Big,
                })
                .collect();
            if !cur_views.is_empty() {
                views.push(cur_views);
            }
        }
        // TODO: do not create format names for every register since they are the same.
        let format_names: Vec<Vec<String>> = views
            .iter()
            .map(|group| group.iter().map(|view| Self::get_view_short_name(*view)).collect())
            .collect();

        let format_width = format_names.iter().map(|f| f[0].len()).max().unwrap_or(0);
        let render = |ui: &Ui, register: &Register, view: NumberView| {
            if let Some(bar_bytes) = self.bars_byte_count {
                let bar_width = views.iter().map(|group| {
                    group
                        .iter()
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
                }).max().unwrap_or(0);
                self.render_register_data(ui, register, view, bar_width)
            } else {
                self.render_register_data_no_alignment(ui, register, view)
            };
        };

        ui.tree_node(&format!("{1:>0$}", width, register.name)).exec(|ui, is_expanded| {
            if !is_expanded {
                ui.same_line(0, 0);
                ui.text("  ");
                ui.same_line(0, 0);
                render(ui, register, default_view);
                return;
            }
            for (group, names) in views.iter().zip(format_names.iter()) {
                if self.group_by_size && group.len() > 1 {
                    ui.tree_node(&format!("{1:>0$}", format_width, names[0]))
                        .exec(|ui, is_expanded| {

                        ui.same_line(0, 0);
                        ui.text("  ");
                        render(ui, register, group[0]);
                        if !is_expanded {
                            return;
                        }
                        for (&view, name) in group[1..].iter().zip(names[1..].iter()) {
                            ui.text(" ");
                            ui.same_line(0, 0);
                            ui.text(&format!("{1:>0$}", format_width, name));
                            ui.same_line(0, 0);
                            ui.text("  ");
                            render(ui, register, view);
                        }
                    });
                } else {
                    for (&view, name) in group.iter().zip(names.iter()) {
                        ui.text(&format!("{1:>0$}", format_width, name));
                        ui.same_line(0, 0);
                        ui.text("  ");
                        render(ui, register, view);
                    }
                }
            }
        });
    }

    fn render_bars_picker(&mut self, ui: &mut Ui) {
        const VARIANTS: [Option<usize>; 5] = [None, Some(1), Some(2), Some(4), Some(8)];
        const NAMES: [&'static str; 5] = ["No columns", "1 byte columns", "2 bytes columns", "4 bytes columns", "8 bytes columns"];
        if let Some(val) = combo(ui, "##bars", &VARIANTS, &NAMES, &self.bars_byte_count) {
            self.bars_byte_count = *val;
        }
    }

    fn render_view_picker(&mut self, ui: &mut Ui) {
        ui.checkbox("Group by size", &mut self.group_by_size);
    }

    fn render_header(&mut self, ui: &mut Ui) {
        self.render_bars_picker(ui);
        ui.same_line(0, -1);
        self.render_view_picker(ui);
    }

    pub fn render(&mut self, ui: &mut Ui) {
        self.render_header(ui);
        let register_name_width = self.registers.iter().map(|r| r.name.len()).max().unwrap_or(0usize);
        for register in self.registers.iter() {
            self.render_register(ui, register_name_width, register);
        }
    }
}

impl View for RegistersView {
    fn new(_: &Ui, _: &Service) -> Self {
        RegistersView {
            registers: Vec::new(),
            bars_byte_count: None,
            group_by_size: false,
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
