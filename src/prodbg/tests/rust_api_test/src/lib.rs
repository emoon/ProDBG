#[macro_use]
extern crate prodbg;

use prodbg::backend::*;
use prodbg::read_write::*;
use prodbg::plugin_handler::*;

struct MyBackend {
    some_data: i32,
}

impl Backend for MyBackend {
    fn new() -> Self {
        MyBackend { some_data: 30 }
    }

    //fn update(&mut self, reader: &prodbg::Reader)
    fn update(&mut self, _: &Reader, _: &Writer) {
        println!("update instance! {}", self.some_data);
        self.some_data += 1;
    }
}

#[no_mangle]
#[allow(unused_mut)] // likely due to compiler bug
pub fn init_plugin(plugin_handler: &mut PluginHandler) 
{
    println!("R: init_plugin");
    let mut plugin = define_backend_plugin!(MyBackend);
    plugin_handler.register(&mut plugin);
}
