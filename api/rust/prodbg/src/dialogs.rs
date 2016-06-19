use std::os::raw::{c_char, c_int};
use std::ffi::CString;

#[repr(C)]
pub struct CDialogFuncs1 {
    open_file: extern "C" fn(dest: *const c_char) -> c_int,
    save_file: extern "C" fn(dest: *const c_char) -> c_int,
    select_directory: extern "C" fn(dest: *const c_char) -> c_int,
}

pub struct Dialogs {
    pub api: *mut CDialogFuncs1,
}

macro_rules! dialog_fun {
    ($name:ident) => {
        pub fn $name(&mut self, dest: &str) -> i32 {
            let d = CString::new(dest).unwrap();
            unsafe {
                ((*self.api).$name)(d.as_ptr()) as i32
            }
        }
    }
}

impl Dialogs {
    dialog_fun!(open_file);
    dialog_fun!(save_file);
    dialog_fun!(select_directory);
}
