use std::os::raw::{c_uchar, c_void};
use std::mem::transmute;

use Capstone;
use CCapstone1;

use Dialogs;
use CDialogFuncs1;

use IdFuncs;
use CIdFuncs1;

pub struct Service {
    pub service_func: extern "C" fn(data: *const c_uchar) -> *mut c_void,
}

impl Service {
    pub fn get_capstone(&self) -> Capstone {
        unsafe {
            let api: &mut CCapstone1 = transmute(((*self).service_func)(b"Capstone Service 1\0".as_ptr()));
            Capstone {
                api: api,
                handle: ::std::ptr::null(),
            }
        }
    }

    pub fn get_dialogs(&self) -> Dialogs {
        unsafe {
            let api: &mut CDialogFuncs1 = transmute(((*self).service_func)(b"Dialogs 1\0".as_ptr()));
            Dialogs { api: api }
        }
    }

    pub fn get_id_register(&self) -> IdFuncs {
        unsafe {
            let api: &mut CIdFuncs1 = transmute(((*self).service_func)(b"IdFuncs 1\0".as_ptr()));
            IdFuncs { api: api }
        }
    }
}
