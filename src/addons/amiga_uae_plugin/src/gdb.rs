use std::sync::mpsc::{channel, Receiver, Sender};
use std::net::TcpStream;
use std::io::{Read, Write};
use std::sync::{Arc, RwLock};
use std::io;
use std::thread;

pub enum NeedsAck {
    Yes,
    No,
}

#[derive(Copy, Clone, PartialEq)]
pub enum State {
    Disconnected,
    Connected,
    SendingData,
    WaitingForData,
    Close,
}

pub struct GdbRemote {
    pub needs_ack: NeedsAck,
    pub writer: Sender<Vec<u8>>,
    pub reader: Receiver<Vec<u8>>,
    temp_data: Vec<u8>,
    state: Arc<RwLock<State>>,
}

static HEX_CHARS: &'static [u8; 16] = b"0123456789abcdef";

impl GdbRemote {
    pub fn new(needs_ack: NeedsAck) -> GdbRemote {
        let (tx, rx) = channel::<Vec<u8>>();

        GdbRemote {
            writer: tx,
            reader: rx,
            needs_ack: needs_ack,
            temp_data: Vec::new(),
            state: Arc::new(RwLock::new(State::Disconnected)),
        }
    }

    fn read_write_data(reader: &Receiver<Vec<u8>>, writer: &Sender<Vec<u8>>, stream: &mut TcpStream) -> io::Result<()> {
        match reader.recv() {
            Ok(data) => {
                // TODO: Less allocs
                let mut buffer = [0; 1024];
                let mut t = Vec::new();
                try!(stream.write_all(&data.into_boxed_slice()));
                let len = try!(stream.read(&mut buffer));
                for i in 0..len { t.push(buffer[i]); }
                match writer.send(t) {
                    Ok(()) => Ok(()),
                    Err(_) => Err(io::Error::new(io::ErrorKind::Other, "channel write failed")),
                }
            }

            Err(_) => Err(io::Error::new(io::ErrorKind::Other, "channel reade error")),
        }
    }

    fn set_state(&mut self, state: State) {
        *(*self.state).write().unwrap() = state
    }

    pub fn connect(&mut self, addr: &str) -> io::Result<()> {
        let mut stream = try!(TcpStream::connect(addr));

        self.set_state(State::Connected);

        let (main_tx, main_rx) = channel::<Vec<u8>>();
        let (thread_tx, thread_rx) = channel::<Vec<u8>>();

        self.writer = main_tx;
        self.reader = thread_rx;

        let state = self.state.clone();

        thread::spawn(move || {
            loop {
                match Self::read_write_data(&main_rx, &thread_tx, &mut stream) {
                    Err(e) => {
                        *(*state).write().unwrap() = State::Disconnected;
                        println!("Error reading/sending data {:?}", e);
                        break;
                    }

                    _ => (),
                }
            }
        });

        Ok(())
    }

    pub fn update(&mut self) {
        match self.reader.try_recv() {
            Ok(data) => {
                println!("got data {}", String::from_utf8(data).unwrap());
            }

            _ => (),
        }
    }

    #[inline]
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
        self.send_command_raw(&command)
    }

    pub fn send_command_raw(&mut self, data: &str) -> io::Result<usize> {
        // TEMP fix me:
        self.temp_data.clear();
        for i in data.as_bytes().iter() { self.temp_data.push(*i) }
        let _ = self.writer.send(self.temp_data.clone());
        Ok((0))
    }

    fn get_state(&mut self) -> State {
        *(*self.state).read().unwrap()
    }

    pub fn send_no_ack_request(&mut self)-> io::Result<usize> {
        self.send_command_raw("+$QStartNoAckMode+#db")
    }

    pub fn get_data_sync(&mut self) -> Option<Vec<u8>> {
        match self.reader.recv() {
            Ok(data) => {
                self.temp_data.clear();
                for i in &data { self.temp_data.push(*i) }
                Some(self.temp_data.clone())
            }

            Err(_) => None,
        }
    }

    pub fn send_command_sync(&mut self, command: &str) -> Option<Vec<u8>> {
        if self.get_state() == State::Connected {
            self.send_command(command).unwrap();
            println!("waiting for data");
            let data = self.get_data_sync();
            println!("got data {}", data.as_ref().unwrap().len());
            data
        } else {
            None
        }
    }

    pub fn step_sync(&mut self) -> Option<Vec<u8>> {
        self.send_command_sync("s")
    }

    pub fn get_registers_sync(&mut self) -> Option<Vec<u8>> {
        self.send_command_sync("g")
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

