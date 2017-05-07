extern crate prodbg_ui;

use std::mem::transmute;
use std::os::raw::{c_void};

//use prodbg_ui::*;
//

struct MyPlugin {
    _dummy: i32,
}

impl MyPlugin {
    fn new() -> Self {
        MyPlugin {
            _dummy: 0,
        }
    }

    fn pressed_button(&mut self) {
        println!("Pressed button");
    }

    fn create_ui(&mut self, ui: &prodbg_ui::ui_gen::Ui) {
        let button = ui.push_button_create();
        button.set_title("Rust: Test");

        extern "C" fn temp_call(target: *mut std::os::raw::c_void) {
            unsafe {
                let app = target as *mut MyPlugin;
                MyPlugin::pressed_button(&mut *app);
            }
        }

        unsafe {
            let object = (*(*button.get_obj()).base).object;
            prodbg_ui::connect(object, b"2released()\0", &self, temp_call);
        }

        //button.on_pressed(self, MyPlugin::pressed_button);
    }
}

macro_rules! connect_released {
    ($sender:expr, $data:expr, $call_type:ident, $callback:path) => {
        {
            extern "C" fn temp_call(target: *mut std::os::raw::c_void) {
                unsafe {
                    let app = target as *mut $call_type;
                    $callback(&mut *app);
                }
            }

            unsafe {
                let object = (*(*$sender.get_obj()).base).object;
                wrui::connect(object, b"2released()\0", $data, temp_call);
            }
        }
    }
}

#[no_mangle]
pub fn init_plugin(ui_ptr: *const prodbg_ui::ffi_gen::PU) {
    // This stuff here is really hidden from the plugin
    // but we do this temporary when testing out the apis
    let ui = prodbg_ui::ui_gen::Ui::new(ui_ptr);

    // This is some tricker to (later on) send back a instance of the plugin to C and then on the
    // Rust side again cast it back to a Rust instance

    let instance: *mut c_void  = unsafe { transmute(Box::new(MyPlugin::new())) };
    let plugin: &mut MyPlugin = unsafe { &mut *(instance as *mut MyPlugin) };

    plugin.create_ui(&ui);


    println!("Rust: memory_view_2");
}
