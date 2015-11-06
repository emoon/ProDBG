use libc::*;
use std::ffi::CString;

#[repr(C)]
pub struct CPDReaderAPI {
    private_data: *mut c_void,
    read_u8: extern "C" fn(data: *mut c_void),
}

#[repr(C)]
pub enum WriteStatus {
    Ok,
    Fail,
}

#[repr(C)]
pub struct CPDWriterAPI {
    private_data: *mut c_void,
    pub write_event_begin: extern fn(writer: *mut c_void, event: uint16_t) -> WriteStatus,
    pub write_event_end: extern fn(writer: *mut c_void) -> WriteStatus,
    pub write_header_array_begin: extern fn(writer: *mut c_void, ids: *mut *const c_char)
                                         -> WriteStatus,
    pub write_header_array_end: extern fn(writer: *mut c_void) -> WriteStatus,
    pub write_array_begin: extern fn(writer: *mut c_void, name: *const c_char) -> WriteStatus,
    pub write_array_end: extern fn(writer: *mut c_void) -> WriteStatus,
    pub write_array_entry_begin: extern fn(writer: *mut c_void) -> WriteStatus,
    pub write_array_entry_end: extern fn(writer: *mut c_void) -> WriteStatus,
    pub write_s8: extern fn(writer: *mut c_void, id: *const c_char, v: int8_t) -> WriteStatus,
    pub write_u8: extern fn(writer: *mut c_void, id: *const c_char, v: uint8_t) -> WriteStatus,
    pub write_s16: extern fn(writer: *mut c_void, id: *const c_char, v: int16_t) -> WriteStatus,
    pub write_u16: extern fn(writer: *mut c_void, id: *const c_char, v: uint16_t) -> WriteStatus,
    pub write_s32: extern fn(writer: *mut c_void, id: *const c_char, v: int32_t) -> WriteStatus,
    pub write_u32: extern fn(writer: *mut c_void, id: *const c_char, v: uint32_t) -> WriteStatus,
    pub write_s64: extern fn(writer: *mut c_void, id: *const c_char, v: int64_t) -> WriteStatus,
    pub write_u64: extern fn(writer: *mut c_void, id: *const c_char, v: uint64_t) -> WriteStatus,
    pub write_float: extern fn(writer: *mut c_void, id: *const c_char, v: c_float) -> WriteStatus,
    pub write_double: extern fn(writer: *mut c_void, id: *const c_char, v: c_double) -> WriteStatus,
    pub write_string: extern fn(writer: *mut c_void, id: *const c_char, v: *const c_char)
                             -> WriteStatus,
    pub write_data: extern fn(w: *mut c_void, id: *const c_char, d: *const uint8_t, l: c_uint)
                            -> WriteStatus,
}

pub struct Reader {
    c_reader_api: *mut CPDReaderAPI,
}

pub struct Writer {
    c_writer_api: *mut CPDWriterAPI,
}

#[repr(C)]
pub enum ReadType {
    None,
    S8,
    U8,
    S16,
    U16,
    S32,
    U32,
    S64,
    U64,
    Float,
    Double,
    EndNumericTypes,
    Str,
    Data,
    Event,
    Array,
    ArrayEntry,
    Count,
}

#[repr(C)]
pub enum ReadStatus {
    Ok = 1 << 8,
    Converted = 2 << 8,
    IllegalType = 3 << 8,
    NotFound = 4 << 8,
    Fail = 5 << 8,
    TypeMask = 0xff,
}


impl Reader {
    pub fn read_u8(&self) {
        unsafe { ((*self.c_reader_api).read_u8)((*self.c_reader_api).private_data) }
    }
}

impl Writer {
    pub fn write_event_begin(&mut self, event: u16) {
        unsafe {
            ((*self.c_writer_api).write_event_begin)((*self.c_writer_api).private_data, event);
        }
    }

    pub fn write_event_end(&mut self) {
        unsafe {
            ((*self.c_writer_api).write_event_end)((*self.c_writer_api).private_data);
        }
    }

    pub fn write_array_begin(&mut self, name: &str) {
        let s = CString::new(name).unwrap();
        unsafe {
            ((*self.c_writer_api).write_array_begin)((*self.c_writer_api).private_data, s.as_ptr());
        }
    }

    pub fn write_array_end(&mut self) {
        unsafe {
            ((*self.c_writer_api).write_array_end)((*self.c_writer_api).private_data);
        }
    }

    pub fn write_array_entry_begin(&mut self) {
        unsafe {
            ((*self.c_writer_api).write_array_entry_begin)((*self.c_writer_api).private_data);
        }
    }

    pub fn write_array_entry_end(&mut self) {
        unsafe {
            ((*self.c_writer_api).write_array_entry_end)((*self.c_writer_api).private_data);
        }
    }

    pub fn write_s8(&mut self, id: &str, v: i8) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_s8)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_u8(&mut self, id: &str, v: u8) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_u8)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_s16(&mut self, id: &str, v: i16) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_s16)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_u16(&mut self, id: &str, v: i16) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_s16)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_s32(&mut self, id: &str, v: i32) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_s32)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_u32(&mut self, id: &str, v: i32) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_s32)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_s64(&mut self, id: &str, v: i64) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_s64)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_u64(&mut self, id: &str, v: i64) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_s64)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_float(&mut self, id: &str, v: f32) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_float)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_double(&mut self, id: &str, v: f64) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_double)((*self.c_writer_api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_string(&mut self, id: &str, v: &str) {
        let id_s = CString::new(id).unwrap();
        let v_s = CString::new(v).unwrap();
        unsafe {
            ((*self.c_writer_api).write_string)((*self.c_writer_api).private_data,
                                                id_s.as_ptr(),
                                                v_s.as_ptr());
        }
    }

    pub fn write_data(&mut self, id: &str, data: &[u8]) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.c_writer_api).write_data)((*self.c_writer_api).private_data,
                                              s.as_ptr(),
                                              data.as_ptr(),
                                              data.len() as u32);
        }
    }

// pub write_data: extern fn(w: *mut c_void, id: *const c_char, d: *mut c_void,
// l: c_uint)



//
// pub write_s16: extern fn(writer: *mut c_void, id: *const c_char, v: int16_t)
// -> WriteStatus,
// pub write_u16: extern fn(writer: *mut c_void, id: *const c_char, v:
// uint16_t) -> WriteStatus,
// pub write_s32: extern fn(writer: *mut c_void, id: *const c_char, v: int32_t)
// -> WriteStatus,
// pub write_u32: extern fn(writer: *mut c_void, id: *const c_char, v:
// uint32_t) -> WriteStatus,
// pub write_s64: extern fn(writer: *mut c_void, id: *const c_char, v: int64_t)
// -> WriteStatus,
// pub write_u64: extern fn(writer: *mut c_void, id: *const c_char, v:
// uint64_t) -> WriteStatus,
// pub write_float: extern fn(writer: *mut c_void, id: *const c_char, v:
// c_float) -> WriteStatus,
// pub write_double: extern fn(writer: *mut c_void, id: *const c_char, v:
// c_double) -> WriteStatus,
// pub write_string: extern fn(writer: *mut c_void, id: *const c_char, v:
// *const c_char)
// -> WriteStatus,
// pub write_data: extern fn(w: *mut c_void, id: *const c_char, d: *mut c_void,
// l: c_uint)
// -> WriteStatus,
//

}
