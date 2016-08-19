use std::os::raw::{c_char, c_void};
use std::mem::transmute;
use super::CFixedString;

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

pub struct StateSaver<'a>(&'a mut CPDSaveState);

impl<'a> StateSaver<'a> {
    pub fn new(api: *mut CPDSaveState) -> StateSaver<'a> {
        unsafe { StateSaver(&mut *api) }
    }

    pub fn write_int(&mut self, data: i64) {
        ((*self.0).write_int)((*self.0).priv_data, data)
    }

    pub fn write_double(&mut self, data: f64) {
        ((*self.0).write_double)((*self.0).priv_data, data)
    }

    pub fn write_str(&mut self, data: &str) {
        let str = CFixedString::from_str(data);
        ((*self.0).write_string)((*self.0).priv_data, str.as_ptr())
    }
}

#[repr(C)]
pub struct CPDLoadState {
    pub priv_data: *mut c_void,
    pub read_int: fn(priv_data: *mut c_void, dest: *mut i64) -> LoadState,
    pub read_double: fn(priv_data: *mut c_void, dest: *mut f64) -> LoadState,
    pub read_string: fn(priv_data: *mut c_void, dest: *mut c_char, max_len: i32) -> LoadState,
    pub read_string_len: fn(priv_data: *const c_void, len: *mut i32) -> LoadState,
}

pub enum LoadResult<T> {
    Ok(T),
    Fail,
    Converted(T),
    Truncated(T),
    OutOfData,
}

impl<T> LoadResult<T> {
    pub fn from_state(state: LoadState, val: T) -> LoadResult<T> {
        match state {
            LoadState::Ok => LoadResult::Ok(val),
            LoadState::Converted => LoadResult::Converted(val),
            LoadState::Truncated => LoadResult::Truncated(val),
            LoadState::Fail => LoadResult::Fail,
            LoadState::OutOfData => LoadResult::OutOfData,
        }
    }
}

pub struct StateLoader<'a>(&'a mut CPDLoadState);

impl<'a> StateLoader<'a> {
    pub fn new(api: *mut CPDLoadState) -> StateLoader<'a> {
        unsafe { StateLoader(&mut *api) }
    }

    pub fn read_int(&mut self) -> LoadResult<i64> {
        let mut res: i64 = 0;
        let state = ((*self.0).read_int)((*self.0).priv_data, &mut res);
        LoadResult::from_state(state, res)
    }

    pub fn read_f64(&mut self) -> LoadResult<f64> {
        let mut res: f64 = 0.0;
        let state = ((*self.0).read_double)((*self.0).priv_data, &mut res);
        LoadResult::from_state(state, res)
    }

    pub fn read_string(&mut self) -> LoadResult<String> {
        let mut len: i32 = 0;
        let len_state = ((*self.0).read_string_len)((*self.0).priv_data, &mut len);
        match len_state {
            LoadState::Fail => return LoadResult::Fail,
            LoadState::OutOfData => return LoadResult::OutOfData,
            _ => {}
        }
        let mut buf = vec!(0u8; len as usize);
        let state = unsafe {
            ((*self.0).read_string)((*self.0).priv_data, transmute(buf.as_mut_ptr()), len)
        };
        LoadResult::from_state(state, String::from_utf8(buf).unwrap())
    }
}
