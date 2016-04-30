use libc::{c_void, c_char};
use prodbg_api::io::{CPDLoadState, CPDSaveState, LoadState};
use prodbg_api::cfixed_string::CFixedString;
use std::ffi::CStr;
use std::mem::transmute;
use std::ptr;

struct WriterData {
    data: Vec<String>,
    read_index: usize,
}

impl WriterData {
    fn new() -> WriterData {
        WriterData {
            data: Vec::new(),
            read_index: 0,
        }
    }

    fn new_load(data: &Vec<String>) -> WriterData {
        WriterData {
            data: data.clone(),
            read_index: 0,
        }
    }
}

fn write_int(priv_data: *mut c_void, data: i64) {
    unsafe {
        println!("saving {}", data);
        let t = priv_data as *mut WriterData;
        let v = format!("{}", data);
        (*t).data.push(v);
    }
}

fn write_double(priv_data: *mut c_void, data: f64) {
    unsafe {
        println!("saving {}", data);
        let t = priv_data as *mut WriterData;
        let v = format!("{}", data);
        (*t).data.push(v);
    }
}

fn write_string(priv_data: *mut c_void, data: *const c_char) {
    unsafe {
        let t = priv_data as *mut WriterData;
        let v = CStr::from_ptr(data).to_string_lossy().into_owned();
        println!("saving {}", v);
        (*t).data.push(v);
    }
}

// TODO: error handling
fn read_int(priv_data: *mut c_void, data: *mut i64) -> LoadState {
    unsafe {
        let t = priv_data as *mut WriterData;
        let v = (*t).data[(*t).read_index].parse::<i64>().unwrap();
        (*t).read_index += 1;
        *data = v;
        LoadState::Ok
    }
}

fn read_double(priv_data: *mut c_void, data: *mut f64) -> LoadState {
    unsafe {
        let t = priv_data as *mut WriterData;
        let v = (*t).data[(*t).read_index].parse::<f64>().unwrap();
        (*t).read_index += 1;
        *data = v;
        LoadState::Ok
    }
}

fn read_string(priv_data: *mut c_void, data: *mut c_char, max_len: i32) -> LoadState {
    unsafe {
        let t = priv_data as *mut WriterData;
        let v = &(*t).data[(*t).read_index];
        (*t).read_index += 1;

        let mut len = v.len();

        if len > max_len as usize { len = max_len as usize; }

        let mut tstring = CFixedString::from_str(&v);

        ptr::copy(tstring.as_ptr(), data as *mut i8, len);

        LoadState::Ok
    }
}

pub fn get_writer_funcs() -> CPDSaveState {
    CPDSaveState {
        priv_data: unsafe { transmute(Box::new(WriterData::new())) },
        write_int: write_int,
        write_double: write_double,
        write_string: write_string,
    }
}

pub fn get_data(save_state: &mut CPDSaveState) -> Vec<String> {
    let writer_data: Box<WriterData> = unsafe { transmute(save_state.priv_data) };
    writer_data.data.clone()
}

pub fn get_loader_funcs(data: &Vec<String>) -> CPDLoadState {
    CPDLoadState  {
        priv_data: unsafe { transmute(Box::new(WriterData::new_load(data))) },
        read_int: read_int,
        read_double: read_double,
        read_string: read_string,
    }
}





