extern crate prodbg_ui;

use std::mem::transmute;
use std::os::raw::{c_void};

//use prodbg_ui::*;
//

struct MyPlugin {
    _dummy: i32,
}

macro_rules! create_button {
    ($ui:tt, $data:tt, $data_type:ty, $($rest:tt)*) => {{
        let button = $ui.push_button_create(); 
        create_button_inner!(button, $data, $data_type, $($rest)*);
        button
    }}
}

macro_rules! create_button_inner {
    ($w:expr, ) => {};
    ($w:expr, $data:tt, ) => {};
    ($w:expr, $data:tt, $data_type:ty, ) => {};

    ($w:expr, $data:tt, $data_type:ty, title: $value:expr, $($rest:tt)*) => {{
        $w.set_title($value);
        create_button_inner!($w, $data, $data_type, $($rest)*);
    }};

    ($w:expr, $data:tt, $data_type:ty, on_released: $callback:path, $($rest:tt)*) => {{
        extern "C" fn temp_call(target: *mut std::os::raw::c_void) {
            unsafe {
                let app = target as *mut $data_type;
                $callback(&mut *app);
            }
        }

        unsafe {
            let object = (*(*$w.get_obj()).base).object;
            prodbg_ui::connect(object, b"2released()\0", $data, temp_call);
        }

        create_button_inner!($w, $data, $data_type, $($rest)*);
    }};
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
        let _button = create_button! {
            ui, self, MyPlugin,
            title: "foo",
            on_released: MyPlugin::pressed_button, 
        };
    }

        /*
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
    */
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
