#[macro_use]
extern crate prodbg_api;
extern crate libc;
use prodbg_api::*;
use libc::c_void;
pub mod gdb;
use gdb::{GdbRemote, Memory, NeedsAck};
//use std::net::{SocketAddr, SocketAddrV4, Ipv4Addr};
//use std::io::Read;
//use std::{slice, str};

//use std::ptr;

//static CODE: &'static [u8] = b"\xd4\x40\x87\x5a\x4e\x71\x02\xb4\xc0\xde\xc0\xde\x5c\x00\x1d\x80\x71\x12\x01\x23\xf2\x3c\x44\x22\x40\x49\x0e\x56\x54\xc5\xf2\x3c\x44\x00\x44\x7a\x00\x00\xf2\x00\x0a\x28\x4E\xB9\x00\x00\x00\x12\x4E\x75";

struct AmigaUaeBackend {
    capstone: Capstone,
    conn: gdb::GdbRemote,
    exception_location: u32,
}

impl AmigaUaeBackend {
    fn from_hex(ch: u8) -> u8 {
        if (ch >= b'a') && (ch <= b'f') {
            return ch - b'a' + 10;
        }

        if (ch >= b'0') && (ch <= b'9') {
            return ch - b'0';
        }

        if (ch >= b'A') && (ch <= b'F') {
            return ch - b'A' + 10;
        }

        return 0;
    }

    fn get_u32(data: &[u8]) -> u32 {
        let t0 = ((Self::from_hex(data[0]) << 4) | Self::from_hex(data[1])) as u32;
        let t1 = ((Self::from_hex(data[2]) << 4) | Self::from_hex(data[3])) as u32;
        let t2 = ((Self::from_hex(data[4]) << 4) | Self::from_hex(data[5])) as u32;
        let t3 = ((Self::from_hex(data[6]) << 4) | Self::from_hex(data[7])) as u32;
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

    fn write_registers(&mut self, writer: &mut Writer, data: &Vec<u8>) {
        let mut index = 1;  // 1 to skip starting $

        writer.event_begin(EventType::SetRegisters as u16);
        writer.array_begin("registers");

        // a registers

        for i in 0..8 {
            let name = format!("a{}", i);
            let reg = Self::get_u32(&data[index..]);
            Self::write_register(writer, &name, reg, false);
            index += 8;
        }

        // d registers

        for i in 0..8 {
            let name = format!("d{}", i);
            let reg = Self::get_u32(&data[index..]);
            Self::write_register(writer, &name, reg, false);
            index += 8;
        }

        // Status registers & pc

        let sr = Self::get_u32(&data[index..]);
        let pc = Self::get_u32(&data[index + 8..]);

        self.exception_location = pc;

        Self::write_register(writer, "sr", sr, true);
        Self::write_register(writer, "pc", pc, true);

        writer.array_end();
        writer.event_end();
    }

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
                let text = format!("{}\t{}", i.mnemonic().unwrap(), i.op_str().unwrap_or(""));
                writer.array_entry_begin();
                writer.write_u32("address", i.address as u32);
                writer.write_string("line", &text);

                //println!("{:?}", i);

                writer.array_entry_end();
            }

            writer.array_end();
            writer.event_end();
        } else {
            println!("No instructions :(");
        }
    }

    fn write_exception_location(&mut self, writer: &mut Writer) {
        writer.event_begin(EventType::SetExceptionLocation as u16);
        writer.write_u64("address", self.exception_location as u64);
        writer.write_u8("size", 4);
        writer.event_end();
    }

    /*
    fn test_capstone(&mut self) {
        match self.capstone.open(Arch::M68K, CS_MODE_M68K_000) {
            Err(e) => {
                println!("Unable to open Capstone {}", e as i32);
                return;
            }
            _ => (),
        }

        if let Ok(insns) = self.capstone.disasm(CODE, 0x1000, 0) {
            println!("Got {} instructions", insns.len());

            for i in insns.iter() {
                println!("{:?}", i);
            }
        } else {
            println!("No instructions :(");
        }
    }
    */
}

impl Backend for AmigaUaeBackend {
    fn new(service: &Service) -> Self {
        AmigaUaeBackend {
            capstone: service.get_capstone(),
            conn: GdbRemote::new(NeedsAck::No),
            exception_location: 0,
        }
    }

    // TODO: Something about action action as i32
    fn update(&mut self, action: i32, reader: &mut Reader, writer: &mut Writer) {
        for event in reader.get_event() {
            match event {
                 35 => {
                    if self.conn.connect("127.0.0.1:6860").is_ok() {
                        println!("Connected!");
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
                self.conn.step_sync();

                if let Some(data) = self.conn.get_registers_sync() {
                    self.write_registers(writer, &data.clone());
                }

                if let Some(memory) = self.conn.get_memory_sync(self.exception_location - 128, 256) {
                    self.write_disassembly(writer, &memory);
                }

                self.write_exception_location(writer);
            }
            _ => (),
        }


        //self.conn.update();
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

