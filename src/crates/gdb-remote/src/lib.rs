pub mod incoming_result;

use std::net::{TcpStream, ToSocketAddrs};
use std::io::{Read, Write};
use std::io;
use std::time::Duration;
use incoming_result::IncomingResult;
use std::str;

#[cfg(target_os = "windows")]
use std::os::windows::io::AsRawSocket;

#[cfg(any(target_os="linux",
    target_os="macos",
    target_os="freebsd",
    target_os="dragonfly",
    target_os="netbsd",
    target_os="openbsd"))]
use std::os::unix::io::AsRawFd;

#[derive(PartialEq)]
pub enum NeedsAck {
    Yes,
    No,
}

pub struct GdbRemote {
    pub stream: Option<TcpStream>,
    temp_data: Vec<u8>,
    temp_string: String,
    needs_ack: NeedsAck,
}

pub struct Memory {
    pub address: u64,
    pub data: Vec<u8>
}

impl Default for GdbRemote {
    fn default() -> Self {
        Self::new()
    }
}

extern "C" {
    fn c_poll_socket(socket: i32) -> i32;
}

const PACKET_SIZE: usize = 1024;
static HEX_CHARS: &'static [u8; 16] = b"0123456789abcdef";
static HEX_TO_BYTE: [u8; 256] = [
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
];

#[inline]
fn calc_checksum(data: &[u8]) -> u8 {
    let mut checksum = 0u32;
    for i in data {
        checksum += *i as u32;
    }
    (checksum & 0xff) as u8
}

#[inline]
fn get_checksum(checksum: u8) -> (u8, u8) {
    (HEX_CHARS[((checksum >> 4) & 0xf) as usize],
     HEX_CHARS[(checksum & 0xf) as usize])
}

/*
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

    0
}

fn build_hex_to_byte_table() -> [u8; 256] {
    let mut table = [0; 256];
    for (i, item) in table.iter_mut().enumerate().take(256) {
        *item = from_hex(i as u8)
    }
    table
}
*/

fn from_pair_hex(data: (u8, u8)) -> u8 {
    let t0 = HEX_TO_BYTE[data.0 as usize];
    let t1 = HEX_TO_BYTE[data.1 as usize];
    (t0 << 4) | t1
}

impl GdbRemote {
    pub fn new() -> GdbRemote {
        GdbRemote {
            needs_ack: NeedsAck::Yes,
            temp_string: String::with_capacity(PACKET_SIZE + 4), // + 4 for header and checksum
            temp_data: Vec::with_capacity(16 * 1024), // 16 temp buffer for incoming data
            stream: None,
            //hex_to_byte: build_hex_to_byte_table(),
        }
    }

    pub fn set_ack(&mut self, ack: NeedsAck) {
        self.needs_ack = ack;
    }

    pub fn connect<A: ToSocketAddrs>(&mut self, addr: A) -> io::Result<()> {
        let stream = try!(TcpStream::connect(addr));
        // 2 sec of time-out to make sure we never gets infitite blocked
        try!(stream.set_read_timeout(Some(Duration::from_secs(2))));
        self.stream = Some(stream);
        Ok(())
    }

    pub fn build_processed_string(dest: &mut String, source: &str) {
        let checksum = calc_checksum(source.as_bytes());
        let csum = get_checksum(checksum);

        dest.clear();
        dest.push('$');
        dest.push_str(source);
        dest.push('#');
        dest.push(csum.0 as char);
        dest.push(csum.1 as char);
    }

    #[cfg(any(target_os="linux",
        target_os="macos",
        target_os="freebsd",
        target_os="dragonfly",
        target_os="netbsd",
        target_os="openbsd"))]
    pub fn get_socket(stream: &mut TcpStream) -> i32 {
        stream.as_raw_fd() as i32
    }

    #[cfg(target_os = "windows")]
    pub fn get_socket(stream: &mut TcpStream) -> i32 {
        stream.as_raw_socket() as i32
    }


    pub fn has_incoming_data(&mut self) -> bool {
        let mut t = 0;
        if let Some(ref mut stream) = self.stream {
            unsafe {
                t = c_poll_socket(Self::get_socket(stream) as i32);
            }
        }

        if t == 1 { true } else { false }
    }

    pub fn send_internal(&mut self) -> io::Result<()> {
        if let Some(ref mut stream) = self.stream {
            stream.write_all(self.temp_string.as_bytes())
        } else {
            Err(io::Error::new(io::ErrorKind::NotConnected, "No connection with a server."))
        }
    }

    pub fn send_command(&mut self, data: &str) -> io::Result<()> {
        Self::build_processed_string(&mut self.temp_string, data);
        self.send_internal()
    }

    fn handle_send_ack(stream: &mut TcpStream, resend_data: &str, dest: &mut [u8]) -> io::Result<usize> {
        loop {
            let mut v = [0; 1];
            try!(stream.read(&mut v));

            match v[0] {
                b'+' => return stream.read(dest),
                b'-' => {
                    try!(stream.write_all(resend_data.as_bytes()));
                }
                _ => {
                    println!("illegal reply {} - {} - {:?}", v[0], v[0] as char, v);
                    return Err(io::Error::new(io::ErrorKind::InvalidData, "Illegal reply from server."))
                }
            }
        }
    }

    pub fn read_reply(&mut self, dest: &mut [u8]) -> io::Result<usize> {
        if let Some(ref mut stream) = self.stream {
            match self.needs_ack {
                NeedsAck::Yes => Self::handle_send_ack(stream, &self.temp_string.clone(), dest),
                NeedsAck::No => stream.read(dest),
            }
        } else {
            Err(io::Error::new(io::ErrorKind::NotConnected, "No connection with a server."))
        }
    }

    pub fn send_command_wait_reply_raw(&mut self, res: &mut [u8], command: &str) -> io::Result<usize> {
        let mut temp_buffer = [0; PACKET_SIZE];
        //println!("Send command {}", command);
        try!(self.send_command(command));
        let len = try!(self.read_reply(&mut temp_buffer));
        //println!("Reply {}", str::from_utf8(&temp_buffer).unwrap());
        // If len returned 0 it means that we got disconnected from the server
        if len == 0 {
            self.stream = None;
            return Err(io::Error::new(io::ErrorKind::ConnectionAborted, "Disconnected from server."));
        }
        try!(Self::validate_checksum(&temp_buffer, len));
        Self::clone_slice(res, &temp_buffer[1..len - 3]);
        Ok((len - 4))
    }

    pub fn is_connected(&self) -> bool {
        self.stream.is_some()
    }

    pub fn get_supported(&mut self, res: &mut [u8]) -> io::Result<usize> {
        self.send_command_wait_reply_raw(res, "qSupported")
    }

    pub fn step(&mut self, res: &mut [u8]) -> io::Result<usize> {
        self.send_command_wait_reply_raw(res, "s")
    }

    pub fn step_over(&mut self) -> io::Result<()> {
        self.send_command("n")
    }

    pub fn cont(&mut self) -> io::Result<(usize)> {
        let mut res = [0; 16];
        self.send_command_wait_reply_raw(&mut res, "c")
    }

    pub fn request_no_ack_mode(&mut self) -> io::Result<usize> {
        let mut res = [0; PACKET_SIZE];
        let len = try!(self.send_command_wait_reply_raw(&mut res, "QStartNoAckMode"));

        if res[0] == b'O' && res[1] == b'K' {
            self.needs_ack = NeedsAck::No;
        }

        Ok((len))
    }

    pub fn get_registers(&mut self, res: &mut [u8]) -> io::Result<usize> {
        let mut temp_buffer = [0; PACKET_SIZE];
        let len = try!(self.send_command_wait_reply_raw(&mut temp_buffer, "g"));
        Self::convert_hex_data_to_binary(res, &temp_buffer[0..len]);
        Ok((len / 2))
    }

    pub fn read_incoming_event(&mut self) -> Option<IncomingResult> {
        let mut size = 0;

        self.temp_data.clear();

        let mut temp_buffer = [0; PACKET_SIZE];

        while self.has_incoming_data() {
            let len = match self.read_reply(&mut temp_buffer) {
                Err(e) => {
                    println!("read_incoming error {:?}", e);
                    return None;
                }
                Ok(len) => len,
            } as usize;

            if len == 0 {
                break;
            }

            self.temp_data.extend_from_slice(&temp_buffer);

            size += len;
        }

        if size > 0 {
            Some(IncomingResult {
                data: &self.temp_data[1..size - 3],
            })
        } else {
            None
        }
    }

    pub fn get_memory(&mut self, dest: &mut Vec<u8>, address: u64, size: u64) -> io::Result<usize> {
        let mut temp_buffer = [0; PACKET_SIZE];
        let mut temp_unpack = [0; PACKET_SIZE/2];

        if size == 0 {
            return Ok((0));
        }

        dest.clear();

        // Text hexdata and 4 bytes of header bytes gives this max amount of requested data / loop
        let size_per_loop = ((PACKET_SIZE - 4) / 2) as u64;
        let mut data_size = size;
        let mut addr = address;
        let mut trans_size = 0u64;

        loop {
            let current_size = if data_size < size_per_loop { data_size } else { size_per_loop };

            // TODO: This allocates memory, would be nice to format to existing string.
            let mem_req = format!("m{:x},{:x}", addr, current_size);

            let len = try!(self.send_command_wait_reply_raw(&mut temp_buffer, &mem_req));

            if temp_buffer[0] == b'E' {
                // Ok, something went wrong here. If we have already got some data we just
                // return what we got (it might be the case that some of the data was ok
                // to read but the other data wasn't (un-mapped memory for example)
                if trans_size == 0 {
                    return Err(io::Error::new(io::ErrorKind::Other, "Unable to read memory"))
                } else {
                    return Ok(trans_size as usize);
                }
            }

            Self::convert_hex_data_to_binary(&mut temp_unpack, &temp_buffer[0..len]);
            dest.extend_from_slice(&temp_unpack[0..len/2]);

            // Bump size and prepare for next chunk

            data_size -= current_size;
            addr += current_size;
            trans_size += current_size;

            if data_size == 0 {
                break;
            }
        }

        Ok(trans_size as usize)
    }

    pub fn set_breakpoint_at_address(&mut self, address: u64) -> io::Result<usize> {
        let mut temp_buffer = [0; PACKET_SIZE];
        // TODO: This allocates memory, would be nice to format to existing string.
        let breakpoint_req = format!("Z0,{:x}", address);
        self.send_command_wait_reply_raw(&mut temp_buffer, &breakpoint_req)
    }

    pub fn remove_breakpoint_at_address(&mut self, address: u64) -> io::Result<usize> {
        let mut temp_buffer = [0; PACKET_SIZE];
        // TODO: This allocates memory, would be nice to format to existing string.
        let breakpoint_req = format!("z0,{:x}", address);
        self.send_command_wait_reply_raw(&mut temp_buffer, &breakpoint_req)
    }

    fn clone_slice(dst: &mut [u8], src: &[u8]) {
        for (d, s) in dst.iter_mut().zip(src.iter()) {
            *d = *s;
        }
    }

    fn validate_checksum(data: &[u8], len: usize) -> io::Result<usize> {
        // - 3 to skip $xx in the end
        let data_len = len;
        let checksum = calc_checksum(&data[1..len - 3]);
        let data_check = from_pair_hex((data[data_len - 2], data[data_len - 1]));

        if data_check == checksum {
            Ok((data_len))
        } else {
            Err(io::Error::new(io::ErrorKind::Other, "Checksum missmatch for data"))
        }
    }

    pub fn convert_hex_data_to_binary(dest: &mut [u8], src: &[u8]) {
        for (d, s) in dest.iter_mut().zip(src.chunks(2)) {
            let v0 = s[0] as usize;
            let v1 = s[1] as usize;
            *d = (HEX_TO_BYTE[v0] << 4) | HEX_TO_BYTE[v1];
        }
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    use std::thread;
    use std::net::{TcpStream, TcpListener};
    use std::sync::{Arc, Mutex};
    use std::io::{Read, Write};
    use std::time::Duration;
    use std::str;

    //const NOT_STARTED: u32 = 0;
    const STARTED: u32 = 1;
    const READ_DATA: u32 = 2;
    const QUIT_SERVER: u32 = 3;
    const SHOULD_QUIT: u32 = 4;
    const TEST_RESEND: u32 = 5;
    const TEST_BAD_SERVER_DATA: u32 = 6;
    const TEST_WAIT_AND_THEN_SEND: u32 = 7;
    const TEST_WAIT_AND_THEN_SEND_LARGE: u32 = 8;
    //const REPLY_SUPPORT: u32 = 2;

    #[test]
    fn test_checksum_calc() {
        let data: [u8; 8] = [32, 32, 32, 32, 64, 64, 64, 64];
        assert_eq!(::calc_checksum(&data), 128);
    }

    #[test]
    fn test_process_string() {
        let mut dest = String::new();
        GdbRemote::build_processed_string(&mut dest, "f");
        assert_eq!(dest, "$f#66");
    }

    fn check_mutex_complete(mutex: &Arc<Mutex<u32>>) -> bool {
        let data = mutex.lock().unwrap();
        if *data != 0 { true } else { false }
    }

    fn update_mutex(mutex: &Arc<Mutex<u32>>, value: u32) {
        let mut data = mutex.lock().unwrap();
        *data = value
    }

    fn get_mutex_value(mutex: &Arc<Mutex<u32>>) -> u32 {
        *mutex.lock().unwrap()
    }

    fn wait_for_thread_init(mutex: &Arc<Mutex<u32>>) {
        loop {
            if check_mutex_complete(mutex) {
                return;
            }

            thread::sleep(Duration::from_millis(1));
        }
    }

    fn setup_listener(server_lock: &Arc<Mutex<u32>>, state: u32, port: u16) {
        let listener = TcpListener::bind(("127.0.0.1", port)).unwrap();
        update_mutex(&server_lock, state as u32);

        loop {
            for stream in listener.incoming() {
                match stream {
                    Ok(stream) => {
                        server(stream, server_lock);
                        return;
                    }
                    _ => (),
                }
            }
        }
    }

    fn get_string_from_buf(buffer: &[u8], size: usize) -> String {
        String::from_utf8_lossy(&buffer[0..size]).into_owned()
    }

    fn get_string_from_buf_trim(buffer: &[u8], size: usize) -> String {
        String::from_utf8_lossy(&buffer[1..size-3]).into_owned()
    }

    fn parse_memory_req(req: &String) -> (usize, usize) {
        let split: Vec<&str> = req[1..].split(',').collect();
        let addr = usize::from_str_radix(split[0], 16).unwrap();
        let size = usize::from_str_radix(split[1], 16).unwrap();
        (addr, size)
    }

    fn convert_binary_to_hex_data(dest: &mut [u8], src: &[u8]) {
        for (d, s) in dest.chunks_mut(2).zip(src.iter()) {
            d[0] = ::HEX_CHARS[(s >> 4) as usize];
            d[1] = ::HEX_CHARS[(s & 0xf) as usize];
        }
    }

    fn server(mut stream: TcpStream, state: &Arc<Mutex<u32>>) {
        let mut temp_send_data = [0; 4096];
        let mut buffer = [0; 2048];
        let value = get_mutex_value(state);
        let mut needs_ack = NeedsAck::Yes;

        // fill in some temp_data used for sending

        for (i, item) in temp_send_data.iter_mut().enumerate().take(4096) {
            *item = (i & 0xff) as u8;
        }

        if value == STARTED {
            return;
        }

        if value == TEST_RESEND {
            stream.read(&mut buffer).unwrap();
            stream.write(b"-").unwrap();
        }

        if value == TEST_BAD_SERVER_DATA {
            stream.read(&mut buffer).unwrap();
            stream.write(b"s").unwrap();
        }

        if value == TEST_WAIT_AND_THEN_SEND {
            // Waiting for 300 ms before sending anything
            thread::sleep(Duration::from_millis(300));
            stream.write(b"somedata").unwrap();
            return;
        }

        if value == TEST_WAIT_AND_THEN_SEND_LARGE {
            let mut data = Vec::<u8>::new();
            data.push(b'$');
            data.push(b'Q');
            data.push(b'T');
            data.push(b'e');
            data.push(b's');
            data.push(b't');

            for i in 0..16000 {
                let s = (i & 0xff) as u8;
                let t0 = ::HEX_CHARS[(s >> 4) as usize];
                let t1 = ::HEX_CHARS[(s & 0xf) as usize];
                data.push(t0);
                data.push(t1);
            }

            // dummy
            data.push(b'#');
            data.push(b'0');
            data.push(b'0');

            // Waiting for 300 ms before sending anything
            thread::sleep(Duration::from_millis(500));

            stream.write(&data).unwrap();
            return;
        }

        loop {
            if get_mutex_value(state) == SHOULD_QUIT {
                break;
            }

            let len = stream.read(&mut buffer).unwrap();

            if len == 0 {
                continue;
            }

            let data = get_string_from_buf_trim(&buffer, len);

            if needs_ack == NeedsAck::Yes {
                // reply that we got the package
                stream.write(b"+").unwrap();
            }

            // fake that server decided to go away
            if value == QUIT_SERVER {
                thread::sleep(Duration::from_millis(5));
                return;
            }

            match data.as_ref() {
                "QStartNoAckMode" => {
                    let mut dest = String::new();
                    GdbRemote::build_processed_string(&mut dest, "OK");
                    stream.write_all(dest.as_bytes()).unwrap();
                    needs_ack = NeedsAck::No;
                },

                "qSupported" => {
                    let mut dest = String::new();
                    GdbRemote::build_processed_string(&mut dest, "PacketSize=1fff");
                    stream.write_all(dest.as_bytes()).unwrap();
                }

                _ => {
                    match buffer[1] {
                        b'm' => {
                            let mut dest = String::new();
                            let (addr, size) = parse_memory_req(&data);

                            // Reply error back if we can't read from here
                            if addr >= 4096 {
                                GdbRemote::build_processed_string(&mut dest, "E01");
                                stream.write_all(dest.as_bytes()).unwrap();
                            } else {
                                convert_binary_to_hex_data(&mut buffer, &temp_send_data[addr..addr+size]);
                                // * 2 in size here because converted from binary -> hex
                                GdbRemote::build_processed_string(&mut dest, str::from_utf8(&buffer[..size*2]).unwrap());
                                stream.write_all(dest.as_bytes()).unwrap();
                            }
                        },

                        b'g' => {
                            let mut dest = String::new();
                            GdbRemote::build_processed_string(&mut dest, "1122aa");
                            stream.write_all(dest.as_bytes()).unwrap();
                        }

                        b's' => {
                            let mut dest = String::new();
                            GdbRemote::build_processed_string(&mut dest, "S01");
                            stream.write_all(dest.as_bytes()).unwrap();
                        }

                        b'z' => {
                            let mut dest = String::new();
                            GdbRemote::build_processed_string(&mut dest, "OK");
                            stream.write_all(dest.as_bytes()).unwrap();
                        }

                        b'Z' => {
                            let mut dest = String::new();
                            GdbRemote::build_processed_string(&mut dest, "OK");
                            stream.write_all(dest.as_bytes()).unwrap();
                        }

                        b'c' => {
                            let mut dest = String::new();
                            GdbRemote::build_processed_string(&mut dest, "OK");
                            stream.write_all(dest.as_bytes()).unwrap();
                            // do nothing for continue
                        }

                        _ => (),
                    }
                }
            }
        }
    }

    #[test]
    fn test_connect() {
        let port = 6860u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, STARTED, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
    }

    #[test]
    fn test_qsupported() {
        let mut res = [0; 1024];
        let port = 6861u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        let size = gdb.get_supported(&mut res).unwrap();

        let supported = get_string_from_buf(&res, size);

        assert_eq!(supported, "PacketSize=1fff");

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_registers() {
        let mut res = [0; 1024];
        let port = 6862u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        let size = gdb.get_registers(&mut res).unwrap();

        assert_eq!(res[0], 0x11);
        assert_eq!(res[1], 0x22);
        assert_eq!(res[2], 0xaa);
        assert_eq!(size, 3);

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_memory() {
        let mut res = Vec::<u8>::with_capacity(2048);
        let port = 6863u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        let size = gdb.get_memory(&mut res, 0, 2048).unwrap();

        assert_eq!(size, 2048);

        for (i, item) in res.iter().enumerate().take(2048) {
            assert_eq!(i as u8, *item as u8);
        }

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_memory_error() {
        let mut res = Vec::<u8>::with_capacity(2048);
        let port = 6864u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        assert_eq!(gdb.get_memory(&mut res, 4096, 128).is_err(), true);

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_memory_server_quit() {
        let mut res = Vec::<u8>::with_capacity(2048);
        let port = 6865u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, QUIT_SERVER, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        assert_eq!(gdb.get_memory(&mut res, 0, 128).is_err(), true);
        assert_eq!(gdb.is_connected(), false);

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_default_zero_mem() {
        let mut res = Vec::<u8>::with_capacity(2048);
        let port = 6866u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::default();
        gdb.connect(("127.0.0.1", port)).unwrap();
        assert_eq!(gdb.get_memory(&mut res, 0, 0).is_ok(), true);
        assert_eq!(gdb.is_connected(), true);
        assert_eq!(res.len(), 0);

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_memory_no_ack() {
        let mut res = Vec::<u8>::with_capacity(2048);
        let port = 6867u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        gdb.request_no_ack_mode().unwrap();
        let size = gdb.get_memory(&mut res, 0, 2048).unwrap();

        assert_eq!(size, 2048);

        for (i, item) in res.iter().enumerate().take(2048) {
            assert_eq!(i as u8, *item as u8);
        }

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_memory_test_resend() {
        let port = 6869u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, TEST_RESEND, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        gdb.request_no_ack_mode().unwrap();

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_memory_test_bad_server_data() {
        let port = 6810u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, TEST_BAD_SERVER_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        assert_eq!(gdb.request_no_ack_mode().is_err(), true);

        update_mutex(&lock, SHOULD_QUIT);
    }


    #[test]
    fn test_step() {
        let mut res = [0; 1024];
        let port = 6811u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        let size = gdb.step(&mut res).unwrap();

        let step_reply = get_string_from_buf(&res, size);

        assert_eq!(step_reply, "S01");

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_memory_send_without_connection() {
        let mut gdb = GdbRemote::new();
        assert_eq!(gdb.request_no_ack_mode().is_err(), true);
    }

    #[test]
    fn test_memory_read_reply_without_connection() {
        let mut res = [0; 1024];
        let mut gdb = GdbRemote::new();
        assert_eq!(gdb.read_reply(&mut res).is_err(), true);
    }

    #[test]
    fn test_cont() {
        let port = 6812u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        gdb.cont().unwrap();

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_incoming() {
        let port = 6813u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();
        let mut has_got_data = false;
        let mut count = 0;

        thread::spawn(move || { setup_listener(&thread_lock, TEST_WAIT_AND_THEN_SEND, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();

        loop {
            if gdb.has_incoming_data() {
                has_got_data = true;
                break;
            }

            count += 1;
            thread::sleep(Duration::from_millis(1));

            if count > 400 {
                break;
            }
        }

        // we expect here that we have updated count a bit and that we have got data and if time is
        // above 300 something has went wrong

        assert!(count > 10 && count < 300);
        assert_eq!(has_got_data, true);

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_set_breakpoint() {
        let port = 6814u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        gdb.set_breakpoint_at_address(0x4444).unwrap();

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_remove_breakpoint() {
        let port = 6815u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();

        thread::spawn(move || { setup_listener(&thread_lock, READ_DATA, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.connect(("127.0.0.1", port)).unwrap();
        gdb.remove_breakpoint_at_address(0x4444).unwrap();

        update_mutex(&lock, SHOULD_QUIT);
    }

    #[test]
    fn test_read_incoming() {
        let port = 6816u16;
        let lock = Arc::new(Mutex::new(0));
        let thread_lock = lock.clone();
        let mut has_got_data = false;
        let mut count = 0;

        thread::spawn(move || { setup_listener(&thread_lock, TEST_WAIT_AND_THEN_SEND_LARGE, port) });
        wait_for_thread_init(&lock);

        let mut gdb = GdbRemote::new();
        gdb.set_ack(NeedsAck::No);
        gdb.connect(("127.0.0.1", port)).unwrap();

        loop {
            if let Some(ref data) = gdb.read_incoming_event() {
                let data = data.begins_with("QTest").unwrap();
                has_got_data = true;

                for i in 0..16000 {
                    let t0 = data[(i * 2) + 0];
                    let t1 = data[(i * 2) + 1];
                    let t = ::from_pair_hex((t0, t1));
                    assert_eq!(t, (i & 0xff) as u8);
                }
            }

            count += 1;
            thread::sleep(Duration::from_millis(1));

            if count > 600 || has_got_data {
                break;
            }
        }

        // we expect here that we have updated count a bit and that we have got data and if time is
        // above 300 something has went wrong

        assert!(count > 10 && count < 600);
        assert_eq!(has_got_data, true);

        update_mutex(&lock, SHOULD_QUIT);
    }


    #[test]
    fn test_parse_memory_1() {
        let data = "m77,22".to_owned();
        let (addr, size) = parse_memory_req(&data);
        assert_eq!(addr, 0x77);
        assert_eq!(size, 0x22);
    }

    #[test]
    fn test_validate_mem_wrong_checksum() {
        assert_eq!(GdbRemote::validate_checksum(b"$some_data#00", 13).is_err(), true);
    }

    #[test]
    fn test_parse_memory_2() {
        let data = "m1177,875".to_owned();
        let (addr, size) = parse_memory_req(&data);
        assert_eq!(addr, 0x1177);
        assert_eq!(size, 0x875);
    }

    #[test]
    fn convert_binary_to_hex_data_test() {
        let t = [0x11, 0x23, 0xab];
        let mut res = [0; 6];
        convert_binary_to_hex_data(&mut res, &t);
        let s = get_string_from_buf(&res, 6);
        assert_eq!("1123ab", s);
    }
}

