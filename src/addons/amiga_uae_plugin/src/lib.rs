#[macro_use]
extern crate prodbg_api;
extern crate gdb_remote;
extern crate amiga_hunk_parser;

mod debug_info;

use prodbg_api::*;
use std::str;
use std::io::Result;
use gdb_remote::GdbRemote;
use debug_info::DebugInfo;
//use std::path::{Path, PathBuf};

struct Breakpoint {
    file_line: Option<(String, u32)>,
    address: Option<u32>,
}

struct Segment {
    address: u32,
    size: u32,
}

struct AmigaUaeBackend {
    capstone: Capstone,
    conn: GdbRemote,
    exception_location: u32,
    id_amiga_uae_dma_time: u16,
    id_amiga_uae_set_file: u16,
    id_amiga_uae_set_hdd_path: u16,
    amiga_exe_file_path: String,
    uae_partition_path: String,
    _break_at_start: bool,
    debug_info: DebugInfo,
    segments: Vec<Segment>,
    status: String,
    breakpoints: Vec<Breakpoint>,
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

    fn write_register(writer: &mut Writer, name: &str, data: &[u8], read_only: bool) {
        writer.array_entry_begin();
        writer.write_string("name", name);

        if read_only {
            writer.write_u8("read_only", 1);
        }

        writer.write_data("register", &data[..4]);
        writer.array_entry_end();
    }

    fn write_registers(&mut self, writer: &mut Writer, data: &[u8]) {
        let mut index = 0;

        writer.event_begin(EventType::SetRegisters as u16);
        writer.array_begin("registers");

        // d registers
        for i in 0..8 {
            let name = format!("d{}", i);
            Self::write_register(writer, &name, &data[index..], false);
            index += 4;
        }

        // a registers
        for i in 0..8 {
            let name = format!("a{}", i);
            Self::write_register(writer, &name, &data[index..], false);
            index += 4;
        }

        // Status registers & pc

        let pc = Self::get_u32(&data[index + 4..]);
        self.exception_location = pc;

        Self::write_register(writer, "sr", &data[index..], true);
        Self::write_register(writer, "pc", &data[index + 4..], true);

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

        //let mut scratch_string = String::with_capacity(256);

        let address = reader.find_u64("address_start").ok().unwrap();
        let count = reader.find_u32("instruction_count").ok().unwrap();

        let mut data = Vec::<u8>::with_capacity(256 * 1024);
        let memory_fetch_size = count * 4;

        if self.conn.get_memory(&mut data, address, memory_fetch_size as u64).is_err() {
            println!("Unable to fetch memory from {:x} - size {}",
                     address,
                     memory_fetch_size);
            return;
        }

        println!("disasm data address {:x} - len {}", address, data.len());

        if let Ok(insns) = self.capstone.disasm(&data, address as u64, 0) {
            writer.event_begin(EventType::SetDisassembly as u16);
            writer.write_u32("address_width", 4);

            writer.array_begin("disassembly");
            let mut c = 0;

            for i in insns.iter() {
                let text = format!("{0: <10} {1: <10}",
                                   i.mnemonic().unwrap(),
                                   i.op_str().unwrap_or(""));
                writer.array_entry_begin();
                writer.write_u32("address", i.address as u32);
                writer.write_string("line", &text);

                /*
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
                */

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

    // Write source/line if we can find debug info for it

    fn write_source_line_location(&mut self, writer: &mut Writer) {
        let location = self.exception_location;

        for seg in &self.segments {
            let seg_start = seg.address;
            let seg_end = seg.address + seg.size;

            if location >= seg_start && location < seg_end {
                if let Some(src_line) = self.debug_info.resolve_file_line(location - seg_start, 0) {
                    writer.write_string("filename", &src_line.0);
                    writer.write_u32("line", src_line.1);
                }
                return;
            }
        }

        //println!("Location not found :(");
    }

    fn write_exception_location(&mut self, writer: &mut Writer) {
        writer.event_begin(EventType::SetExceptionLocation as u16);
        writer.write_u64("address", self.exception_location as u64);
        writer.write_u8("size", 4);

        self.write_source_line_location(writer);

        writer.event_end();
    }

    fn toggle_breakpoint_fileline(conn: &mut GdbRemote, debug_info: &DebugInfo, filename: &str, line: u32, add: bool) {
        if let Some((seg, offset)) = debug_info.get_address_seg(filename, line) {
            let mut res = [0; 64];
            let command;

            if add {
                command = format!("Z0,{:x},{}", offset, seg);
            } else {
                command = format!("z0,{:x},{}", offset, seg);
            }

            if let Err(err) = conn.send_command_wait_reply_raw(&mut res, &command) {
                println!("Unable to send breakpoint to UAE {} - {:?}", command, err)
            }
        } else {
            println!("No debug info, storing breakpoint and send it later");
        }
    }

    fn set_breakpoint_file_line(&mut self, filename: &str, line: u32) {
        Self::toggle_breakpoint_fileline(&mut self.conn, &self.debug_info, filename, line, true);

        self.breakpoints.push(Breakpoint {
            file_line: Some((filename.to_owned(), line)),
            address: None
        });
    }

    fn set_breakpoint(&mut self, reader: &mut Reader, _writer: &mut Writer) {
        if let Ok(filename) = reader.find_string("filename") {
            if let Ok(line) = reader.find_u32("line") {
                println!("trying to add breakpoint {} - {}", filename, line);
                self.set_breakpoint_file_line(filename, line);
            }
        } else if let Some(address) = reader.find_u64("address").ok() {
            self.breakpoints.push(Breakpoint {
                file_line: None,
                address: Some(address as u32),
            });

            if self.conn.set_breakpoint_at_address(address as u64).is_err() {
                println!("Unable to set breakpoint at 0x{:08x}", address);
            }
        }
    }

    fn delete_breakpoint(&mut self, reader: &mut Reader, _writer: &mut Writer) {
        if let Ok(address) = reader.find_u64("address") {
            if self.conn.remove_breakpoint_at_address(address).is_err() {
                println!("Unable to remove breakpoint at 0x{:08x}", address);
            }
        }
    }

    fn send_breakpoints(&mut self) {
        for breakpoint in &self.breakpoints {
            if let Some((ref file, line)) = breakpoint.file_line {
                println!("AmigaUAE: Sending breakpoint {} - {}", file, line);
                Self::toggle_breakpoint_fileline(&mut self.conn, &self.debug_info, file, line, true);
            } else if let Some(address) = breakpoint.address {
                if self.conn.set_breakpoint_at_address(address as u64).is_err() {
                    println!("Unable to set breakpoint at 0x{:08x}", address);
                }
            }
        }
    }

    fn get_registers(&mut self, writer: &mut Writer) {
        let mut register_data = [0; 1024];
        if self.conn.get_registers(&mut register_data).is_ok() {
            self.write_registers(writer, &register_data);
        } else {
            println!("Unable to get registers from UAE");
        }
    }


    // TODO: Would be nice to provide some better way to read the data from the gdb
    // backend using iterators or such

    fn process_dma_frame(event_id: u16, gdb_data: &[u8], writer: &mut Writer) {
        let mut data = [0; 512 * 1024];

        println!("DataLen {}", gdb_data.len());

        GdbRemote::convert_hex_data_to_binary(&mut data, gdb_data);

        let line = Self::get_u16(&data[0..]);
        let count = Self::get_u16(&data[2..]);

        println!("Dma frame {} - {} - len {}", line, count, gdb_data.len());

        writer.event_begin(event_id);
        writer.write_u16("line", line);
        writer.write_u16("xcount", count);
        writer.write_data("data", &data[4..((line as usize * count as usize) * 2)]);
        writer.event_end();
    }

    fn update_conn_incoming(&mut self, writer: &mut Writer) {
        let mut should_break = false;

        if self.conn.is_connected() {
            if let Some(ref event) = self.conn.read_incoming_event() {
                if let Some(ref data) = event.begins_with("QDmaFrame:") {
                    Self::process_dma_frame(self.id_amiga_uae_dma_time, &data, writer);
                } else if let Some(ref _data) = event.begins_with("S") {
                    should_break = true;
                }
            }
        } else {
            self.status = "Not Connected".to_owned();
        }

        if should_break {
            println!("Should break!");
            self.write_exception_location(writer);
        }
    }

    fn step(&mut self, writer: &mut Writer) {
        let mut step_res = [0; 16];
        if self.conn.step(&mut step_res).is_err() {
            println!("Unable to step!");
            return;
        }

        let mut register_data = [0; 1024];
        if self.conn.get_registers(&mut register_data).is_err() {
            println!("Unable to get registers!");
            return;
        }

        self.exception_location = Self::get_u32(&register_data[64 + 4..]);

        self.write_exception_location(writer);
    }

    fn connect(&mut self) -> Result<()> {
        if self.conn.is_connected() {
            return Ok(());
        }

        try!(self.conn.connect("127.0.0.1:6860"));
        try!(self.conn.request_no_ack_mode());

        Ok(())
    }

    fn set_file(&mut self, reader: &mut Reader) {
        if let Ok(filename) = reader.find_string("text") {
            println!("AmigaUAEBackend: Set file path {}", filename);
            self.amiga_exe_file_path = filename.to_owned();
        }
    }

    fn set_hdd_path(&mut self, reader: &mut Reader) {
        if let Ok(path) = reader.find_string("text") {
            println!("AmigaUAEBackend: Set uae path {}", path);
            self.uae_partition_path = path.to_owned();
        }
    }

    fn store_segments(&mut self, segment_reply: &[u8]) {
        let segs_name = str::from_utf8(segment_reply).unwrap();
        let segs: Vec<&str> = segs_name.split(";").collect();

        println!("store segments {:?}", segs);

        self.segments = Vec::new();

        if segs.len() == 0 || segs[0] != "AS" {
            return;
        }

        for i in 0..(segs.len() - 1) / 2 {
            let index = 1 + i * 2;
            let address = segs[index];
            let size = segs[index + 1];
            self.segments.push(Segment {
                address: address.parse::<u32>().unwrap(),
                size: size.parse::<u32>().unwrap(),
            });
        }
    }
}

impl Backend for AmigaUaeBackend {
    fn new(service: &Service) -> Self {
        let id_service = service.get_id_register();

        AmigaUaeBackend {
            capstone: service.get_capstone(),
            id_amiga_uae_dma_time: id_service.register_id("AmigaUAE_DmaTime"),
            id_amiga_uae_set_file: id_service.register_id("AmigaUAE_SetFile"),
            id_amiga_uae_set_hdd_path: id_service.register_id("AmigaUAE_SetHddPath"),
            conn: GdbRemote::new(),
            exception_location: 0,
            amiga_exe_file_path: "".to_owned(),
            uae_partition_path: "".to_owned(),
            _break_at_start: false,
            debug_info: DebugInfo::new(),
            segments: Vec::new(),
            status: "Not Connected".to_owned(),
            breakpoints: Vec::new(),
        }
    }

    fn update(&mut self, action: i32, reader: &mut Reader, writer: &mut Writer) {
        self.update_conn_incoming(writer);

        for event in reader.get_event() {
            //println!("getting event {}", event);

            match event {
                EVENT_GET_REGISTERS => {
                    self.get_registers(writer);
                }

                EVENT_GET_DISASSEMBLY => {
                    self.write_disassembly(reader, writer);
                }

                EVENT_GET_MEMORY => {
                    self.get_memory(reader, writer);
                }

                EVENT_SET_BREAKPOINT => {
                    println!("Set breakpoint");
                    self.set_breakpoint(reader, writer);
                }

                EVENT_DELETE_BREAKPOINT => {
                    self.delete_breakpoint(reader, writer);
                }

                EVENT_GET_EXCEPTION_LOCATION => {
                    self.write_exception_location(writer);
                }

                _ => {
                    if event as u16 == self.id_amiga_uae_set_file {
                        self.set_file(reader);
                    } else if event as u16 == self.id_amiga_uae_set_hdd_path {
                        self.set_hdd_path(reader);
                    }
                }
            }
        }

        match action {
            ACTION_BREAK => {
                self.step(writer);
            }

            ACTION_RUN => {
                let mut res = [0; 1024];

                if self.amiga_exe_file_path == "" {
                    println!("No executable to run");
                    return;
                }

                self.debug_info.load_info(&self.uae_partition_path, &self.amiga_exe_file_path);

                if let Err(err) = self.connect() {
                    println!("Unable to connect {:?}", err);
                    return;
                }

                println!("Sending breakpoints...");
                self.send_breakpoints();
                println!("Sending breakpoints... Done");

                let run_cmd = format!("vRun;{};", self.amiga_exe_file_path);

                if self.conn.send_command_wait_reply_raw(&mut res, &run_cmd).is_ok() {
                    let null_index = res.iter().position(|c| *c == 0).unwrap_or(res.len());
                    self.store_segments(&res[..null_index]);
                } else {
                    println!("Didn't get relpy with segmnts");
                }

                self.status = format!("Connected (127.0.0.1) Running {}", &self.amiga_exe_file_path);
            }

            ACTION_STEP => {
                self.step(writer);
            }

            ACTION_STEP_OVER => {
                // Really don't think 'n' is correct here but use it for now
                if self.conn.send_command("n").is_ok() {
                    println!("Step over instruction");
                }
            }

            ACTION_STOP => {
                // Set kill command
                if self.conn.send_command("k").is_err() {
                    println!("Unable to send kill command");
                } else {
                    // clear debug info
                    self.debug_info = DebugInfo::new();
                    self.status = "Connected (127.0.0.1)".to_owned();
                }
            }
            _ => (),
        }

        //writer.event_begin(EVENT_SET_STATUS as u16);
        //writer.write_string("status", &self.status);
        //writer.event_end();
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_backend_plugin!(PLUGIN, b"Amiga UAE Debugger\0", AmigaUaeBackend);
    plugin_handler.register_backend(&PLUGIN);
}
