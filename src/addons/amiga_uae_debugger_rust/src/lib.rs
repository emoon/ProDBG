#![allow(non_snake_case)]

extern crate libc;
use std::ffi::CString;

#[repr(C)]
pub struct plugin_backend {
    test: i32,
}

static TEST: plugin_backend = plugin_backend { test: 0 };

#[no_mangle]
pub extern fn InitPlugin(register_plugin: extern fn(data: *const libc::c_char, &plugin_backend, *const libc::c_void), pdata: *const libc::c_void) {
    let t = CString::new("ProDBG Backend 1").unwrap();
    register_plugin(t.as_ptr(), &TEST, pdata); 
}
