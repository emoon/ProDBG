#[macro_use]
extern crate prodbg_api;
extern crate libc;
use prodbg_api::*;
use libc::c_void;
//use std::ptr;

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

    fn register_menu(&mut self, menu_funcs: &mut MenuFuncs) -> *mut c_void {
        let menu = menu_funcs.create_menu("Amiga UAE Debugger");

        menu_funcs.add_menu_item(menu, "Connect to UAE...", 0, 0, 0);

        menu
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_backend_plugin!(PLUGIN, b"Amiga UAE Debugger", AmigaUaeBackend);
    plugin_handler.register_backend(&PLUGIN);
}

