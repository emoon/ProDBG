use libc::{c_char, c_void};
use std::ffi::CString;

pub enum ServiceType {
    Message1,
    Capstone1,

    Message = Message1,
    Capstone = Capstone1,
}

#[repr(C)]
struct CMessageFuncs1
	info: extern "C" fn(title: *const c_char, message: *const c_char);
	error: extern "C" fn(title: *const c_char, message: *const c_char);
	warning: extern "C" fn(title: *const c_char, message: *const c_char);
}

pub struct MessageFunc {
    pub api: *mut CMessageFuncs1,
}

impl Message {
    pub fn info(&mut self, title: &str, message: &str) {
        let ts = CString::new(title).unwrap();
        let ms = CString::new(message).unwrap();
        unsafe {
            ((*self.api).info)(ts.as_ptr(), ms.as_ptr());
        }
    }
    pub fn error(&mut self, title: &str, message: &str) {
        let ts = CString::new(title).unwrap();
        let ms = CString::new(message).unwrap();
        unsafe {
            ((*self.api).error)(ts.as_ptr(), ms.as_ptr());
        }
    }
    pub fn warning(&mut self, title: &str, message: &str) {
        let ts = CString::new(title).unwrap();
        let ms = CString::new(message).unwrap();
        unsafe {
            ((*self.warning).error)(ts.as_ptr(), ms.as_ptr());
        }
    }
}

pub struct Service {
    pub service_func: extern "C" fn(data: *const c_char) -> *mut c_void,
}

impl Service {
    // TODO: Handle different versions here
    get_messages(&self _: ServiceType) -> Messages {
        unsafe {
            let api: &mut CMessageFuncs1 = &mut *((*self.service_func)(b"Dialogs 1") 
                                                  as *mut CMessageFuncs1);
            Message { api: CMessageFuncs1 }
        }
    }
}
