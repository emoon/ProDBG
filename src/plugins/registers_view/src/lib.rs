#[macro_use]
extern crate prodbg_api;

use prodbg_api::{View, Ui, Service, Reader, Writer, PluginHandler, CViewCallbacks, ReadStatus, EventType};

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
            let size = try!(reg_data.find_u8("size"));
            let read_only = reg_data.find_u8("read_only").unwrap_or(0) != 0;
            let mut data = Vec::with_capacity(size as usize);
            unsafe {
                match size {
                    1 => {
                        let val = try!(reg_data.find_u8("register"));
                        data.extend_from_slice(std::mem::transmute::<&u8, &[u8; 1]>(&val));
                    },
                    2 => {
                        let val = try!(reg_data.find_u16("register"));
                        data.extend_from_slice(std::mem::transmute::<&u16, &[u8; 2]>(&val));
                    },
                    _ => panic!("Don't know how to read register of size {}", size),
                }
            }
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
}

impl View for RegistersView {
    fn new(_: &Ui, _: &Service) -> Self {
        RegistersView {
            registers: Vec::new(),
        }
    }

    fn update(&mut self, _: &mut Ui, reader: &mut Reader, _: &mut Writer) {
        self.process_events(reader);
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Registers View 2\0", RegistersView);
    plugin_handler.register_view(&PLUGIN);
}
