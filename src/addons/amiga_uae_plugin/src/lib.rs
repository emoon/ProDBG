#[macro_use]
extern crate prodbg_api;
extern crate gdb_remote;
extern crate libc;
use prodbg_api::*;
use libc::c_void;
use gdb_remote::GdbRemote;

struct AmigaUaeBackend {
    _capstone: Capstone,
    conn: GdbRemote,
    exception_location: u32,
}

impl AmigaUaeBackend {
    fn get_u32(data: &[u8]) -> u32 {
        let t0 = data[0] as u32;
        let t1 = data[1] as u32;
        let t2 = data[2] as u32;
        let t3 = data[3] as u32;
        (t0 << 24) | (t1 << 16) | (t2 << 8) | t3
    }

    fn write_register(writer: &mut Writer, name: &str, data: u32, read_only: bool) {
        writer.array_entry_begin();
        writer.write_string("name", name);
        writer.write_u8("size", 4);

        if read_only {
            writer.write_u8("read_only", 1);
        }

        writer.write_u32("register", data);

        writer.array_entry_end();
    }

    fn write_registers(&mut self, writer: &mut Writer, data: &[u8]) {
        let mut index = 0;

        writer.event_begin(EventType::SetRegisters as u16);
        writer.array_begin("registers");

        // d registers

        for i in 0..8 {
            let name = format!("d{}", i);
            let reg = Self::get_u32(&data[index..]);
            Self::write_register(writer, &name, reg, false);
            index += 4;
        }

        // a registers

        for i in 0..8 {
            let name = format!("a{}", i);
            let reg = Self::get_u32(&data[index..]);
            Self::write_register(writer, &name, reg, false);
            index += 4;
        }

        // Status registers & pc

        let sr = Self::get_u32(&data[index..]);
        let pc = Self::get_u32(&data[index + 4..]);

        self.exception_location = pc;

        Self::write_register(writer, "sr", sr, true);
        Self::write_register(writer, "pc", pc, true);

        writer.array_end();
        writer.event_end();
    }

    /*
    fn write_disassembly(&mut self, writer: &mut Writer, mem: &Memory) {
        match self.capstone.open(Arch::M68K, CS_MODE_M68K_000) {
            Err(e) => {
                println!("Unable to open Capstone {}", e as i32);
                return;
            }
            _ => (),
        }

        if let Ok(insns) = self.capstone.disasm(&mem.data, mem.address, 0) {
            writer.event_begin(EventType::SetDisassembly as u16);
            writer.array_begin("disassembly");

            for i in insns.iter() {
                let text = format!("{0: <10} {1: <10}", i.mnemonic().unwrap(), i.op_str().unwrap_or(""));
                writer.array_entry_begin();
                writer.write_u32("address", i.address as u32);
                writer.write_string("line", &text);

                writer.array_entry_end();
            }

            writer.array_end();
            writer.event_end();
        } else {
            println!("No instructions :(");
        }
    }
    */

    fn write_exception_location(&mut self, writer: &mut Writer) {
        writer.event_begin(EventType::SetExceptionLocation as u16);
        writer.write_u64("address", self.exception_location as u64);
        writer.write_u8("size", 4);
        writer.event_end();
    }
}

impl Backend for AmigaUaeBackend {
    fn new(service: &Service) -> Self {
        AmigaUaeBackend {
            _capstone: service.get_capstone(),
            conn: GdbRemote::new(),
            exception_location: 0,
        }
    }

    fn update(&mut self, action: i32, reader: &mut Reader, writer: &mut Writer) {
        for event in reader.get_event() {
            match event {
                35 => {
                    if self.conn.connect("127.0.0.1:6860").is_ok() {
                        if self.conn.request_no_ack_mode().is_ok() {
                            println!("Connected. Ready to go!");
                        } else {
                            println!("no ack request failed");
                        }
                    } else {
                        println!("Unable to connect to UAE");
                    }
                }
                _ => (),
            }
        }

        match action {
            ACTION_BREAK => {
                println!("Break");
            }

            ACTION_STEP => {
                let mut step_res = [0; 16];
                let mut register_data = [0; 1024];
                self.conn.step(&mut step_res).unwrap();

                    println!("steping ok!");
                    if self.conn.get_registers(&mut register_data).is_ok() {
                        println!("setting registers!");
                        self.write_registers(writer, &register_data);
                    }

                self.write_exception_location(writer);
            }
            _ => (),
        }
    }

    fn register_menu(&mut self, menu_funcs: &mut MenuFuncs) -> *mut c_void {
        let menu = menu_funcs.create_menu("Amiga UAE Debugger");
        menu_funcs.add_menu_item(menu, "Connect to UAE...", 0, 0, 0);
        menu
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_backend_plugin!(PLUGIN, b"Amiga UAE Debugger\0", AmigaUaeBackend);
    plugin_handler.register_backend(&PLUGIN);
}

