use libc::{c_uchar, c_void};
use std::mem::transmute;
use Capstone;
use CCapstone1;
use Messages;
use CMessageFuncs1;

pub struct Service {
    pub service_func: extern "C" fn(data: *const c_uchar) -> *mut c_void,
}

impl Service {
    // TODO: Handle different versions

    pub fn get_messages(&self) -> Messages {
        unsafe {
            let api: &mut CMessageFuncs1 = transmute(((*self).service_func)(b"Dialogs 1\0".as_ptr()));
            Messages { api: api }
        }
    }

    pub fn get_capstone(&self) -> Capstone {
        unsafe {
            let api: &mut CCapstone1  = transmute(((*self).service_func)(b"Capstone Service 1\0".as_ptr()));
            Capstone {
                api: api,
                handle: ::std::ptr::null(),
            }
        }
    }
}
