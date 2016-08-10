//! View to show registers.
//! All data that backend sends is in network (big) endianness.

#[macro_use]
extern crate prodbg_api;

mod number_view;

use prodbg_api::{View, Ui, Service, Reader, Writer, PluginHandler, CViewCallbacks, ReadStatus, EventType};
use number_view::*;

#[derive(Debug)]
struct Register {
    name: String,
    read_only: bool,
    value: Vec<u8>,
}

struct RegistersView {
    registers: Vec<Register>
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

    fn render_view_short_name(ui: &Ui, view: NumberView) {
        ui.text(view.representation.as_short_str());
        ui.same_line(0, 0);
        ui.text(view.size.as_bit_len_str());
        ui.same_line(0, 0);
    }

    fn render_register_data(ui: &Ui, register: &Register, view: NumberView) {
        for chunk in register.value.chunks(view.size.byte_count()) {
            ui.same_line(0, 0);
            let value = view.format(chunk);
            ui.text(&value);
            ui.same_line(0, 0);
            ui.text(" ");
        }
    }

    fn render_register_view(ui: &Ui, register: &Register, view: NumberView) {
            Self::render_view_short_name(ui, view);
            ui.text("  ");
            Self::render_register_data(ui, register, view);
    }

    fn render_register(ui: &mut Ui, width: usize, register: &Register) {
        let default_view = NumberView {
            representation: NumberRepresentation::Hex,
            size: NumberSize::OneByte,
            endianness: Endianness::Big,
        };
        let is_shown = ui.tree_node(&format!("{1:>0$}", width, register.name)).show(|ui: &Ui| {
            for size in [NumberSize::OneByte, NumberSize::TwoBytes, NumberSize::FourBytes, NumberSize::EightBytes].iter().filter(|size| size.byte_count() <= register.value.len()) {
                for repr in [NumberRepresentation::Hex, NumberRepresentation::UnsignedDecimal, NumberRepresentation::SignedDecimal, NumberRepresentation::Float].iter().filter(|repr| repr.can_be_of_size(*size)) {
                    let view = NumberView {
                        representation: *repr,
                        size: *size,
                        endianness: Endianness::Big,
                    };
                    Self::render_register_view(ui, register, view);
                }
            }
        });
        if !is_shown {
            ui.same_line(0, 0);
            ui.text("  ");
            ui.same_line(0, 0);
            Self::render_register_data(ui, register, default_view);
        }
    }

    pub fn render(&mut self, ui: &mut Ui) {
        let register_name_width = self.registers.iter().map(|r| r.name.len()).max().unwrap_or(0usize);
        for register in self.registers.iter() {
            Self::render_register(ui, register_name_width, register);
        }
    }
}

impl View for RegistersView {
    fn new(_: &Ui, _: &Service) -> Self {
        RegistersView {
            registers: Vec::new(),
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
