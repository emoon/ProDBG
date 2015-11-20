#[macro_use]
extern crate prodbg;

use prodbg::backend::*;
use prodbg::read_write::*;
use prodbg::plugin_handler::*;

struct MyBackend {
    current_test: u32,
}

impl MyBackend {
    fn write_all_types(&mut self, writer: &mut Writer) {
        let data: [u8; 5] = [1, 2, 3, 4, 5];

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
    }
}

impl Backend for MyBackend {
    fn new() -> Self {
        MyBackend { current_test: 0 }
    }

    // fn update(&mut self, reader: &prodbg::Reader)
    fn update(&mut self, _: &mut Reader, writer: &mut Writer) {
        match self.current_test {
            0 => Self::write_all_types(self, writer),
            _ => (),
        }
    }
}

#[no_mangle]
#[allow(unused_mut)] // likely due to compiler bug
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    println!("R: init_plugin");
    let mut plugin = define_backend_plugin!(MyBackend);
    plugin_handler.register(&mut plugin);
}
