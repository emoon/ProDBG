#[macro_use]
extern crate prodbg_api;
extern crate gdb_remote;

use prodbg_api::*;
use std::os::raw::{c_void};
use gdb_remote::GdbRemote;

const MENU_CONNECT: u32 = 0;
const MENU_ENABLE_DMA: u32 = 1;

struct AmigaUaeBackend {
    capstone: Capstone,
    conn: GdbRemote,
    exception_location: u32,
    id_amiga_uae_dma_time: u16,
}

impl AmigaUaeBackend {
    #[inline]
    fn get_u32(data: &[u8]) -> u32 {
        let t0 = data[0] as u32;
        let t1 = data[1] as u32;
        let t2 = data[2] as u32;
        let t3 = data[3] as u32;
        (t0 << 24) | (t1 << 16) | (t2 << 8) | t3
    }

    #[inline]
    fn get_u16(data: &[u8]) -> u16 {
        let t0 = data[0] as u16;
        let t1 = data[1] as u16;
        (t0 << 8) | t1
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

    fn write_disassembly(&mut self, reader: &mut Reader, writer: &mut Writer) {
        match self.capstone.open(Arch::M68K, CS_MODE_M68K_000) {
            Err(e) => {
                println!("Unable to open Capstone {}", e as i32);
                return;
            }
            _ => (),
        }

        let mut scratch_string = String::with_capacity(256);

        let address = reader.find_u64("address_start").ok().unwrap();
        let count = reader.find_u32("instruction_count").ok().unwrap();

        let mut data = Vec::<u8>::with_capacity(256 * 1024);
        let memory_fetch_size = count * 4;

        if self.conn.get_memory(&mut data, address, memory_fetch_size as u64).is_err() {
            println!("Unable to fetch memory from {:x} - size {}", address, memory_fetch_size);
            return;
        }

        println!("disasm data address {:x} - len {}", address, data.len());

        if let Ok(insns) = self.capstone.disasm(&data, address as u64, 0) {
            writer.event_begin(EventType::SetDisassembly as u16);
            writer.array_begin("disassembly");
            let mut c = 0;

            for i in insns.iter() {
                let text = format!("{0: <10} {1: <10}", i.mnemonic().unwrap(), i.op_str().unwrap_or(""));
                writer.array_entry_begin();
                writer.write_u32("address", i.address as u32);
                writer.write_string("line", &text);

                scratch_string.clear();

                for register in i.regs_read().unwrap() {
                    let reg_name = self.capstone.reg_name(*register);
                    scratch_string.push_str(reg_name);
                    scratch_string.push(' ');
                }

                if scratch_string.len() > 0 {
                    let t = scratch_string.trim_right();
                    writer.write_string("registers_read", t);
                }

                scratch_string.clear();

                for register in i.regs_write().unwrap() {
                    let reg_name = self.capstone.reg_name(*register);
                    scratch_string.push_str(reg_name);
                    scratch_string.push(' ');
                }

                if scratch_string.len() > 0 {
                    let t = scratch_string.trim_right();
                    writer.write_string("registers_write", t);
                }

                writer.array_entry_end();

                c += 1;
            }

            println!("reported {} instructions", c);

            writer.array_end();
            writer.event_end();
        } else {
            println!("No instructions :(");
        }
    }

    fn get_memory(&mut self, reader: &mut Reader, writer: &mut Writer) {
        let mut data = Vec::<u8>::with_capacity(256 * 1024);

        let address = reader.find_u64("address_start").ok().unwrap();
        let size = reader.find_u32("size").ok().unwrap();

        if self.conn.get_memory(&mut data, address, size as u64).is_err() {
            println!("Unable to fetch memory from {:x} - size {}", address, size);
            return;
        }

        writer.event_begin(EventType::SetMemory as u16);
        writer.write_u64("address", address);
        writer.write_data("data", &data);
        writer.event_end();
    }

    //
    fn write_exception_location(&mut self, writer: &mut Writer) {
        writer.event_begin(EventType::SetExceptionLocation as u16);
        writer.write_u64("address", self.exception_location as u64);
        writer.write_u8("size", 4);
        writer.event_end();
    }

    fn set_breakpoint(&mut self, reader: &mut Reader, _writer: &mut Writer) {
       if let Some(address) = reader.find_u64("address").ok() {
           if self.conn.set_breakpoint_at_address(address).is_err() {
               println!("Unable to set breakpoint at 0x{:08x}", address);
           }
       }
    }

    fn delete_breakpoint(&mut self, reader: &mut Reader, _writer: &mut Writer) {
       if let Some(address) = reader.find_u64("address").ok() {
           if self.conn.remove_breakpoint_at_address(address).is_err() {
               println!("Unable to remove breakpoint at 0x{:08x}", address);
           }
       }
    }

    fn get_registers(&mut self, writer: &mut Writer) {
        let mut register_data = [0; 1024];
        if self.conn.get_registers(&mut register_data).is_ok() {
            println!("setting registers!");
            self.write_registers(writer, &register_data);
        }

        self.write_exception_location(writer);
    }


    // TODO: Would be nice to provide some better way to read the data from the gdb
    // backend using iterators or such

    fn process_dma_time(event_id: u16, gdb_data: &[u8], writer: &mut Writer) {
        let mut index = 4;
        let mut data = [0; 4096];

        GdbRemote::convert_hex_data_to_binary(&mut data, gdb_data);

        let line = Self::get_u16(&data[0..]);
        let count = Self::get_u16(&data[2..]);

        writer.event_begin(event_id);
        writer.write_u16("line", line);
        writer.write_u16("count", count);
        writer.array_begin("events");

        println!("processing dma event line {} - {}", line, count);

        for _ in 0..count {
            let event = Self::get_u16(&data[index..]);
            let t = Self::get_u16(&data[index + 2..]);

            writer.array_entry_begin();
            writer.write_u16("event", event);
            writer.write_u16("type", t);
            writer.array_entry_end();

            index += 4;
        }

        writer.array_end();
        writer.event_end();
    }

    fn update_conn_incoming(&mut self, writer: &mut Writer) {
        let mut should_break = false;

        if self.conn.is_connected() {
            if let Some(ref event) = self.conn.read_incoming_event() {
                if let Some(ref data) = event.begins_with("QDmaTime:") {
                    Self::process_dma_time(self.id_amiga_uae_dma_time, &data, writer);
                } else if let Some(ref _data) = event.begins_with("S") {
                    should_break = true;
                }
            }
        }

        if should_break {
            self.get_registers(writer);
        }
    }

    fn on_menu(&mut self, reader: &mut Reader) {
        let menu_id = reader.find_u32("menu_id").ok().unwrap();

        println!("menu id {}", menu_id);

        match menu_id {
            MENU_CONNECT => {
                if self.conn.connect("127.0.0.1:6860").is_ok() {
                    if self.conn.request_no_ack_mode().is_ok() {
                        println!("Connected. Ready to go!");
                        if self.conn.cont().is_err() {
                            println!("Failed to cont");
                        }
                    } else {
                        println!("no ack request failed");
                    }
                } else {
                    println!("Unable to connect to UAE");
                }
            }

            MENU_ENABLE_DMA => {
                if self.conn.is_connected() {
                    let mut res = [0; 256];
                    self.conn.send_command_wait_reply_raw(&mut res, "QDmaTimeEnable").unwrap();
                }
            }

            _ => (),
        }
    }
}

impl Backend for AmigaUaeBackend {
    fn new(service: &Service) -> Self {
        AmigaUaeBackend {
            capstone: service.get_capstone(),
            id_amiga_uae_dma_time: service.get_id_register().register_id("AmigaUAEDmaTime"),
            conn: GdbRemote::new(),
            exception_location: 0,
        }
    }

    fn update(&mut self, action: i32, reader: &mut Reader, writer: &mut Writer) {
        self.update_conn_incoming(writer);

        for event in reader.get_event() {
            match event {
                EVENT_MENU_EVENT => {
                    self.on_menu(reader);
                }
                EVENT_GET_DISASSEMBLY => {
                    self.write_disassembly(reader, writer);
                }

                EVENT_GET_MEMORY => {
                    self.get_memory(reader, writer);
                }

                EVENT_SET_BREAKPOINT => {
                    self.set_breakpoint(reader, writer);
                }

                EVENT_DELETE_BREAKPOINT => {
                    self.delete_breakpoint(reader, writer);
                }

                EVENT_GET_EXCEPTION_LOCATION => {
                    self.write_exception_location(writer);
                }

                _ => (),
            }
        }

        match action {
            ACTION_BREAK => {
                println!("Break");
            }

            ACTION_RUN => {
                if self.conn.cont().is_err() {
                    println!("Unable to run");
                }
            }

            ACTION_STEP => {
                let mut step_res = [0; 16];
                self.conn.step(&mut step_res).unwrap();
                    println!("step res {:?}", step_res);
                    self.get_registers(writer);
                }
            _ => (),
        }

    }

    fn register_menu(&mut self, menu_funcs: &mut MenuFuncs) -> *mut c_void {
        let menu = menu_funcs.create_menu("Amiga UAE Debugger");
        menu_funcs.add_menu_item(menu, "Connect to UAE...", MENU_CONNECT as usize, 0, 0);
        menu_funcs.add_menu_item(menu, "Enable DMA Stream", MENU_ENABLE_DMA as usize, 0, 0);
        menu
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_backend_plugin!(PLUGIN, b"Amiga UAE Debugger\0", AmigaUaeBackend);
    plugin_handler.register_backend(&PLUGIN);
}

