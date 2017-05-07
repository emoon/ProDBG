pub mod ffi_gen;
pub mod traits_gen;
pub mod ui_gen;
pub mod widgets_gen;

use std::os::raw::{c_void};
use std::mem::transmute;

pub use ui_gen::Ui;
pub use traits_gen::Widget;

pub fn connect<D>(object: *const ffi_gen::PUObject, signal: &[u8], data: &D, fun: extern fn(*mut c_void)) {
    unsafe {
        ((*(*object).object_funcs).connect)(object as *const c_void, signal.as_ptr() as *const i8, transmute(data), fun as *const c_void);
    }
}

