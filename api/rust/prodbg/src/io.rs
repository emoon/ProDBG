use libc::{c_void, c_char};

#[repr(C)]
pub enum LoadState {
	Ok,
	Fail,
	Converted,
	Truncated,
	OutOfData,
}

#[repr(C)]
pub struct CPDSaveState {
    pub priv_data: *mut c_void,
    pub write_int: fn(priv_data: *mut c_void, data: i64),
    pub write_double: fn(priv_data: *mut c_void, data: f64),
    pub write_string: fn(priv_data: *mut c_void, data: *const c_char),
}

pub struct CPDLoadState {
    pub priv_data: *mut c_void,
    pub read_int: fn(priv_data: *mut c_void, dest: *mut i64) -> LoadState,
    pub read_double: fn(priv_data: *mut c_void, dest: *mut f64) -> LoadState,
    pub read_string: fn(priv_data: *mut c_void, dest: *mut c_char, max_len: i32) -> LoadState,
}

