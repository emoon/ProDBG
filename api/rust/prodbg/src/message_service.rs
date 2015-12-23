use libc::{c_char};
use std::ffi::CString;

#[repr(C)]
pub struct CMessageFuncs1 {
	info: extern "C" fn(title: *const c_char, message: *const c_char),
	error: extern "C" fn(title: *const c_char, message: *const c_char),
	warning: extern "C" fn(title: *const c_char, message: *const c_char),
}

pub struct Messages {
    pub api: *mut CMessageFuncs1,
}

macro_rules! message_fun {
    ($name:ident) => {
        pub fn $name(&mut self, title: &str, message: &str) {
            let ts = CString::new(title).unwrap();
            let ms = CString::new(message).unwrap();
            unsafe {
                ((*self.api).$name)(ts.as_ptr(), ms.as_ptr());
            }
        }
    }
}

impl Messages {
    message_fun!(info);
    message_fun!(error);
    message_fun!(warning);
}


