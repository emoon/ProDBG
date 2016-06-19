use std::os::raw::{c_void, c_char, c_uchar};
use std::ffi::CStr;
use std::ptr;

pub extern "C" fn get_services(type_name: *const c_uchar) -> *mut c_void {
    unsafe {
        let name = CStr::from_ptr(type_name as *const c_char).to_str().unwrap();

        match name {
            "Capstone Service 1" => get_capstone_service_1(),
            _ =>  ptr::null_mut(),
        }
    }
}

extern "C" {
    fn get_capstone_service_1() -> *mut c_void;
}

