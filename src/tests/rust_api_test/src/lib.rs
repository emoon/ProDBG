#[macro_use]
extern crate prodbg;
use prodbg::*;

static CODE: &'static [u8] = b"\x55\x48\x8b\x05\xb8\x13\x00\x00";

struct MyBackend {
    capstone: Capstone,
    current_test: u32,
}

impl MyBackend {
    fn write_all_types(&mut self, writer: &mut Writer) {
        let data: [u8; 6] = [1, 2, 3, 80, 50, 60];

        writer.event_begin(3);

        writer.write_s8("my_s8", -2);
        writer.write_u8("my_u8", 3);

        writer.write_s16("my_s16", -2000);
        writer.write_u16("my_u16", 56);

        writer.write_s32("my_s32", -300000);
        writer.write_u32("my_u32", 4000000);

        writer.write_s64("my_s64", -1400000);
        writer.write_u64("my_u64", 6000000);

        writer.write_float("my_float", 14.0);
        writer.write_float("my_float2", -24.0);

        writer.write_double("my_double", 23.0);
        writer.write_double("my_double2", 63.0);

        writer.write_string("my_string", "foobar1337");
        writer.write_data("my_data", &data);

        writer.event_end();
    }

    fn test_read_data(&mut self, reader: &mut Reader) {
        while let Some(e) = reader.get_event() {
            assert!(e == 3);

            // so while unwrapping here might not be very nice
            // this is actually ok in this case because a painc
            // will tell us that something is wrong and fail the
            // test case
            
            assert!(reader.find_s8("my_s8").ok().unwrap() == -2i8);
            assert!(reader.find_s8("my_s8").ok().unwrap() == -2);
            assert!(reader.find_u8("my_u8").ok().unwrap() == 3);

            assert!(reader.find_s16("my_s16").ok().unwrap() == -2000);
            assert!(reader.find_u16("my_u16").ok().unwrap() == 56);

            assert!(reader.find_s32("my_s32").ok().unwrap() == -300000);
            assert!(reader.find_u32("my_u32").ok().unwrap() == 4000000);

            assert!(reader.find_s64("my_s64").ok().unwrap() == -1400000);
            assert!(reader.find_u64("my_u64").ok().unwrap() == 6000000);

            assert!(reader.find_float("my_float").ok().unwrap() == 14.0);
            assert!(reader.find_float("my_float2").ok().unwrap() == -24.0);

            assert!(reader.find_double("my_double").ok().unwrap() == 23.0);
            assert!(reader.find_double("my_double2").ok().unwrap() == 63.0);
        }
    }

    fn test_capstone(&mut self) {
        match self.capstone.open(Arch::X86, MODE_64) {
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
}

impl Backend for MyBackend {
    fn new(service: &Service) -> Self {
        MyBackend { 
            capstone: service.get_capstone(),
            current_test: 0 
        }
    }

    // fn update(&mut self, reader: &prodbg::Reader)
    // TODO: Something about action action as i32
    fn update(&mut self, _: i32, reader: &mut Reader, writer: &mut Writer) {
        println!("running update");
        match self.current_test {
            0 => Self::write_all_types(self, writer),
            1 => Self::test_read_data(self, reader),
            2 => Self::test_capstone(self),
            _ => (),
        }

        self.current_test += 1;
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_backend_plugin!(PLUGIN, b"backend_test\0", MyBackend);
    plugin_handler.register_backend(&PLUGIN);
}
