#[macro_use]
extern crate prodbg_api;
extern crate gdb_remote;
extern crate amiga_hunk_parser;
extern crate nfd;

mod debug_info;

use prodbg_api::*;
use std::str;
use std::io::Result;
use std::os::raw::c_void;
use gdb_remote::GdbRemote;
use debug_info::DebugInfo;
use nfd::Response;
use std::path::{Path, PathBuf};

//const MENU_CONNECT: u32 = 0;
//const MENU_ENABLE_DMA: u32 = 1;

struct Segment {
    address: u32,
    size: u32,
}

struct AmigaUaeBackend {
    capstone: Capstone,
    conn: GdbRemote,
    exception_location: u32,
    id_amiga_uae_dma_time: u16,
    amiga_exe_file_path: String,
    uae_partition_path: String,
    break_at_start: bool,
    debug_info: DebugInfo,
    segments: Vec<Segment>,
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

        let mut scratch_string = String::with_capacity(256);

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
            writer.array_begin("disassembly");
            let mut c = 0;

            for i in insns.iter() {
                let text = format!("{0: <10} {1: <10}",
                                   i.mnemonic().unwrap(),
                                   i.op_str().unwrap_or(""));
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

    // Write source/line if we can find debug info for it

    fn write_source_line_location(&mut self, writer: &mut Writer) {
        let location = self.exception_location;

        for seg in &self.segments {
            let seg_start = seg.address;
            let seg_end = seg.address + seg.size;

            if location >= seg_start && location < seg_end {
                println!("Found pc within segment {:x} - {:x}", location, seg_start);
                if let Some(src_line) = self.debug_info.resolve_file_line(location - seg_start, 0) {
                    writer.write_string("filename", &src_line.0);
                    writer.write_u32("line", src_line.1);
                }
                //println!("Found pc within segment {:?}", source_line); 
                return;
            }
        }

        println!("Location not found :(");
    }

    fn write_exception_location(&mut self, writer: &mut Writer) {
        writer.event_begin(EventType::SetExceptionLocation as u16);
        writer.write_u64("address", self.exception_location as u64);
        writer.write_u8("size", 4);

        self.write_source_line_location(writer);

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
        }

        if should_break {
            self.get_registers(writer);
        }
    }

    fn connect(&mut self) -> Result<()> {
        if self.conn.is_connected() {
            return Ok(());
        }

        try!(self.conn.connect("127.0.0.1:6860"));
        try!(self.conn.request_no_ack_mode());

        Ok(())
    }

    /*
    fn on_menu(&mut self, reader: &mut Reader) {
        let menu_id = reader.find_u32("menu_id").ok().unwrap();

        println!("menu id {}", menu_id);

        match menu_id {
            MENU_CONNECT => {
                self.connect();
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
    */

    fn store_segments(&mut self, segment_reply: &[u8]) {
        let segs_name = str::from_utf8(segment_reply).unwrap();
        let segs: Vec<&str> = segs_name.split(";").collect();

        println!("store segments {:?}", segs);

        self.segments = Vec::new();

        if segs.len() == 0 || segs[0] != "AS" {
            return;
        }

        for i in 0..(segs.len()-1)/2 {
            let index = 1 + i * 2;
            let address = segs[index];
            let size = segs[index + 1];
            self.segments.push(Segment {
                address: address.parse::<u32>().unwrap(),
                size: size.parse::<u32>().unwrap(),
            });
        }
    }

    fn show_file_dir_select(name: &str, temp: &mut [u8], path: &mut String, ui: &Ui) {
        temp[..path.len()].copy_from_slice(path.as_bytes());

        if ui.input_text(name, temp.as_mut(), 
                      PDUIINPUTTEXTFLAGS_ENTERRETURNSTRUE | 
                      PDUIINPUTTEXTFLAGS_AUTOSELECTALL,
                      None) {
            let null_index = temp.iter().position(|c| *c == 0).unwrap_or(temp.len());
            if let Ok(parsed) = str::from_utf8(&temp[..null_index]) {
                *path = parsed.to_string();
            }
        }
    }

    fn get_sub_path(&mut self, source: &str) {
        let mut new_path = PathBuf::new();
        let source_path = Path::new(source);
        let needle_path = Path::new(&self.uae_partition_path);

        let mut n = source_path.components();

        for _ in needle_path.components() {
            n.next();
        }

        for t in n {
            match t {
                std::path::Component::Normal(path) => new_path.push(path),
                _ => (),
            }
        }

        self.amiga_exe_file_path = "dh0:".to_owned() + new_path.as_path().to_string_lossy().as_ref();
    }

    fn set_file_exe_path(&mut self, filename: &str) {
        if self.uae_partition_path == "" {
            println!("Set path to UAE HDD first");
            return;
        }

        // Make sure that file is within the partition path 

        if !filename.contains(&self.uae_partition_path) {
            println!("File {} isn't within the set partition path", filename);
            return;
        }

        self.get_sub_path(filename);
    }
}

impl Backend for AmigaUaeBackend {
    fn new(service: &Service) -> Self {
        AmigaUaeBackend {
            capstone: service.get_capstone(),
            id_amiga_uae_dma_time: service.get_id_register().register_id("AmigaUAEDmaTime"),
            conn: GdbRemote::new(),
            exception_location: 0,
            amiga_exe_file_path: "".to_owned(),
            uae_partition_path: "".to_owned(),
            break_at_start: false,
            debug_info: DebugInfo::new(),
            segments: Vec::new(),
        }
    }

    fn update(&mut self, action: i32, reader: &mut Reader, writer: &mut Writer) {
        self.update_conn_incoming(writer);

        for event in reader.get_event() {
            match event {
                /*
                EVENT_MENU_EVENT => {
                    self.on_menu(reader);
                }
                */
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
                let mut res = [0; 1024];

                if self.amiga_exe_file_path == "" {
                    // TODO: Report this back to UI
                    println!("No executable to run");
                    return;
                }

                self.debug_info.load_info(&self.uae_partition_path, &self.amiga_exe_file_path);

                println!("Start (trying to connect)");

                if let Err(err) = self.connect() {
                    println!("Unable to connect {:?}", err);
                    return;
                }

                let run_cmd = format!("vRun;{};", self.amiga_exe_file_path);

                println!("sending {}", run_cmd);

                if self.conn.send_command_wait_reply_raw(&mut res, &run_cmd).is_ok() {
                    let null_index = res.iter().position(|c| *c == 0).unwrap_or(res.len());
                    self.store_segments(&res[..null_index]);
                }
            }

            ACTION_STEP => {
                let mut step_res = [0; 16];
                if self.conn.step(&mut step_res).is_err() {
                    println!("Unable to step!");
                    return;
                }
                println!("step res {:?}", step_res);
                self.get_registers(writer);
            }

            ACTION_STEP_OVER => {
                // Really don't think 'n' is correct here but use it for now
                if self.conn.send_command("n").is_ok() {
                    println!("Step over instruction");
                }
                //self.get_registers(writer);
            }

            ACTION_STOP => {
                // Set kill command
                if self.conn.send_command("k").is_err() {
                    println!("Unable to send kill command");
                }
            }
            _ => (),
        }
    }

    fn show_config(&mut self, ui: &mut Ui) {
        let mut buf: [u8; 4096] = [0; 4096];
        let mut buf2: [u8; 4096] = [0; 4096];

        ui.align_first_text_height_to_widgets();

        ui.text("UAE Partition Path");
        ui.same_line(220, -1);
        Self::show_file_dir_select("##Partition", &mut buf2, &mut self.uae_partition_path, ui);

        ui.same_line(0, -1);
        if ui.button("...##1", None) {
            let result = nfd::open_pick_folder(Some(&self.uae_partition_path)).unwrap_or_else(|e| {
                panic!(e);
            });

            match result {
                Response::Okay(file_path) => self.uae_partition_path = file_path, 
                _ => (),
            }
        }

        ui.text("Executable (dh0:<exe>)");
        ui.same_line(220, -1);
        Self::show_file_dir_select("##Executable", &mut buf, &mut self.amiga_exe_file_path, ui);

        ui.same_line(0, -1);
        if ui.button("...##2", None) {
            let result = nfd::open_file_dialog(None, Some(&self.amiga_exe_file_path)).unwrap_or_else(|e| {
                panic!(e);
            });

            match result {
                Response::Okay(file_path) => self.set_file_exe_path(&file_path), 
                _ => (),
            }
        }

        /*
        if ui.button("...") {
            let result = nfd::open_pick_folder(Some(&self.amiga_exe_file_path)).unwrap_or_else(|e| {
                println!("Unable to open pick folder {:?}", e);
            });

            match result {
                Response::Okay(file_path) => self.amiga_exe_file_path = file_path, 
            }
        }
        */

        ui.checkbox("Break at Start", &mut self.break_at_start);
    }

    fn save_state(&mut self, mut saver: StateSaver) {
        let break_at_start = if self.break_at_start { 1 } else { 0 };
        saver.write_str(&self.amiga_exe_file_path);
        saver.write_str(&self.uae_partition_path);
        saver.write_int(break_at_start);
    }

    fn load_state(&mut self, mut loader: StateLoader) {
        let fp = loader.read_string();

        if let LoadResult::Ok(file_path) = fp {
            self.amiga_exe_file_path = file_path;
        }

        let res = loader.read_string();

        if let LoadResult::Ok(uae_part) = res {
            self.uae_partition_path = uae_part;
        }

        if let LoadResult::Ok(break_at_start) = loader.read_int() {
            self.break_at_start = break_at_start == 1; 
        }
    }

    fn register_menu(&mut self, _menu_funcs: &mut MenuFuncs) -> *mut c_void {
        /*
        let menu = menu_funcs.create_menu("Amiga UAE Debugger");
        menu_funcs.add_menu_item(menu, "Connect to UAE...", MENU_CONNECT as usize, 0, 0);
        menu_funcs.add_menu_item(menu, "Enable DMA Stream", MENU_ENABLE_DMA as usize, 0, 0);
        menu
        */
        std::ptr::null_mut() as *mut c_void
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_backend_plugin!(PLUGIN, b"Amiga UAE Debugger\0", AmigaUaeBackend);
    plugin_handler.register_backend(&PLUGIN);
}
