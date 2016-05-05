#[macro_use]
extern crate prodbg_api;
use prodbg_api::*;

//static CODE: &'static [u8] = b"\x55\x48\x8b\x05\xb8\x13\x00\x00";

struct AmigaUaeBackend {
    _capstone: Capstone,
}

impl AmigaUaeBackend {
    /*
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
    */
}

impl Backend for AmigaUaeBackend {
    fn new(service: &Service) -> Self {
        AmigaUaeBackend { 
            _capstone: service.get_capstone(),
        }
    }

    // TODO: Something about action action as i32
    fn update(&mut self, _: i32, _reader: &mut Reader, _writer: &mut Writer) {

    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_backend_plugin!(PLUGIN, b"amiga_uae_backend", AmigaUaeBackend);
    plugin_handler.register_backend(&PLUGIN);
}

