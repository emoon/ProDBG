use std::net::TcpStream;
use std::net::SocketAddr;
use std::io::Write;
use std::io;

pub enum NeedsAck {
    Yes,
    No,
}

pub struct GdbRemote {
    pub stream: Option<TcpStream>,
    pub needs_ack: NeedsAck,
}

static HEX_CHARS: &'static [u8; 16] = b"0123456789abcdef";

impl GdbRemote {
    pub fn new(needs_ack: NeedsAck) -> GdbRemote {
        GdbRemote {
            stream: None,
            needs_ack: needs_ack,
        }
    }

    pub fn new_and_connect(addr: &SocketAddr, needs_ack: NeedsAck) -> io::Result<GdbRemote> {
        match TcpStream::connect(addr).map(|stream| stream) {
            Ok(s) => Ok(GdbRemote { stream: Some(s), needs_ack: needs_ack }),
            Err(e) => Err(e),
        }
    }

    pub fn connect(&mut self, addr: &SocketAddr) -> io::Result<()> {
        self.stream = Some(try!(TcpStream::connect(addr)));
        Ok(())
    }

    fn calc_checksum(data: &[u8]) -> u8 {
        let mut checksum = 0u32;
        for i in data {
            checksum += *i as u32;
        }
        (checksum & 0xff) as u8
    }

    #[inline]
    pub fn get_checksum_array(checksum: u8) -> [u8; 2] {
        let cs_string: [u8; 2] = [HEX_CHARS[((checksum >> 4) & 0xf) as usize],
                                  HEX_CHARS[(checksum & 0xf) as usize]];
        cs_string
    }

    pub fn get_processed_string(data: &str) -> String {
        let checksum = Self::calc_checksum(data.as_bytes());
        let cs_array = Self::get_checksum_array(checksum);

        // len + 4 = # <string> $xx (4 bytes extra needed) 
        let mut gdb_string = String::with_capacity(data.len() + 4);

        gdb_string.push('$');
        gdb_string.push_str(data);
        gdb_string.push('#');
        gdb_string.push(cs_array[0] as char);
        gdb_string.push(cs_array[1] as char);

        gdb_string
    }

    pub fn send_command(&mut self, data: &str) -> io::Result<usize> {
        let command = Self::get_processed_string(data);
        if let Some(ref mut stream) = self.stream {
            stream.write(command.as_str().as_bytes())
        } else {
            Ok((0))
        }
    }

    pub fn send_simple_command(&mut self, command: char) -> io::Result<usize> {
        let cs = Self::get_checksum_array(command as u8);
        let data: [u8; 5] = [b'$', command as u8, b'#', cs[0], cs[1]];
        if let Some(ref mut stream) = self.stream {
            stream.write(&data)
        } else {
            Ok((0))
        }
    }
}

#[test]
fn test_checksum_calc() {
    let data: [u8; 8] = [32, 32, 32, 32, 64, 64, 64, 64];
    assert_eq!(GdbRemote::calc_checksum(&data), 128);
}

#[test]
fn test_process_string() {
    assert_eq!(GdbRemote::get_processed_string("f"), "$f#66");
}

