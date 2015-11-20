use libc::*;
use std::ffi::CString;

#[repr(C)]
pub struct CPDReaderAPI {
    private_data: *mut c_void,
    read_u8: extern "C" fn(data: *mut c_void),

    pub data: *mut c_void,
    pub read_get_event: extern fn(reader: *mut c_void) -> uint32_t,
    pub read_iterator_next_event: extern fn(reader: *mut c_void,
                                            it: *mut uint64_t) -> uint32_t,
    pub read_iterator_begin: extern fn(reader: *mut c_void, it: *mut uint64_t,
                                       keyName: *mut *const c_char, parentIt: uint64_t)
                                    -> uint32_t,
    pub read_iterator_next: extern fn(reader: *mut c_void, keyName: *mut *const c_char,
                                      it: *mut uint64_t) -> uint32_t,
    pub read_next_entry: extern fn(reader: *mut c_void, arrayIt: *mut uint64_t)
                                -> int32_t,
    pub read_find_s8: extern fn(reader: *mut c_void, res: *mut int8_t, id: *const c_char,
                                it: uint64_t) -> uint32_t,
    pub read_find_u8: extern fn(reader: *mut c_void, res: *mut uint8_t, id: *const c_char,
                                it: uint64_t) -> uint32_t,
    pub read_find_s16: extern fn(reader: *mut c_void, res: *mut int16_t, id: *const c_char,
                                 it: uint64_t) -> uint32_t,
    pub read_find_u16: extern fn(reader: *mut c_void, res: *mut uint16_t, id: *const c_char,
                                 it: uint64_t) -> uint32_t,
    pub read_find_s32: extern fn(reader: *mut c_void, res: *mut int32_t, id: *const c_char,
                                 it: uint64_t) -> uint32_t,
    pub read_find_u32: extern fn(reader: *mut c_void, res: *mut uint32_t, id: *const c_char,
                                 it: uint64_t) -> uint32_t,
    pub read_find_s64: extern fn(reader: *mut c_void, res: *mut int64_t, id: *const c_char,
                                 it: uint64_t) -> uint32_t,
    pub read_find_u64: extern fn(reader: *mut c_void, res: *mut uint64_t, id: *const c_char,
                                 it: uint64_t) -> uint32_t,
    pub read_find_float: extern fn(reader: *mut c_void, res: *mut c_float, id: *const c_char,
                                   it: uint64_t) -> uint32_t,
    pub read_find_double: extern fn(reader: *mut c_void, res: *mut c_double, id: *const c_char,
                                    it: uint64_t) -> uint32_t,
    pub read_find_string: extern fn(reader: *mut c_void, res: *mut *const c_char, id: *const c_char,
                                    it: uint64_t) -> uint32_t,
    pub read_find_data: extern fn(reader: *mut c_void, data: *mut *mut c_void, size: *mut uint64_t,
                                  id: *const c_char, it: uint64_t) -> uint32_t,
    pub read_find_array: extern fn(reader: *mut c_void, arrayIt: *mut uint64_t, id: *const c_char,
                                   it: uint64_t) -> uint32_t,
    pub read_dump_data: extern fn(reader: *mut c_void),
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
    pub api: *mut CPDReaderAPI,
    pub it: u64,
}

pub struct Writer {
    pub api: *mut CPDWriterAPI,
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

pub struct ReaderIter {
    reader: Reader,
    curr_iter: u64,
}

impl Clone for Reader {
    fn clone(&self) -> Self {
        return Reader {
            api: self.api,
            it: self.it,
        };
    }
}

fn status_res<T>(res: T, s: u32) -> Result<T, ReadStatus> {
    match (s >> 8) & 0xff {
        1...2 => Ok(res),
        3 => Err(ReadStatus::IllegalType),
        _ => Err(ReadStatus::NotFound),
    }
}

impl Reader {
    pub fn new(in_api: *mut CPDReaderAPI, iter: u64) -> Self {
        return Reader {
            api: in_api,
            it: iter,
        };
    }

    pub fn find_u8(&self, id: &str) -> Result<u8, ReadStatus> {
        let s = CString::new(id).unwrap();
        let mut res = 0u8;
        let ret;

        unsafe {
            ret = ((*self.api).read_find_u8)((*self.api).private_data,
                                             &mut res,
                                             s.as_ptr(),
                                             self.it);
        }

        return status_res(res, ret);
    }

    pub fn find_array(&self, id: &str) -> ReaderIter {
        let s = CString::new(id).unwrap();
        let mut t = 0u64;

        unsafe {
            ((*self.api).read_find_array)((*self.api).private_data, &mut t, s.as_ptr(), 0);
        }

        ReaderIter {
            reader: self.clone(),
            curr_iter: t,
        }
    }
}

impl Iterator for ReaderIter {
    type Item = Reader;
    fn next(&mut self) -> Option<Reader> {
        let ret;
        unsafe {
            ret = ((*self.reader.api).read_next_entry)((*self.reader.api).private_data,
                                                       &mut self.curr_iter);
        }

        match ret {
            0 => None,
            _ => Some(Reader::new(self.reader.api, self.curr_iter)),
        }
    }
}

impl Writer {
    pub fn write_event_begin(&mut self, event: u16) {
        unsafe {
            ((*self.api).write_event_begin)((*self.api).private_data, event);
        }
    }

    pub fn write_event_end(&mut self) {
        unsafe {
            ((*self.api).write_event_end)((*self.api).private_data);
        }
    }

    pub fn write_array_begin(&mut self, name: &str) {
        let s = CString::new(name).unwrap();
        unsafe {
            ((*self.api).write_array_begin)((*self.api).private_data, s.as_ptr());
        }
    }

    pub fn write_array_end(&mut self) {
        unsafe {
            ((*self.api).write_array_end)((*self.api).private_data);
        }
    }

    pub fn write_array_entry_begin(&mut self) {
        unsafe {
            ((*self.api).write_array_entry_begin)((*self.api).private_data);
        }
    }

    pub fn write_array_entry_end(&mut self) {
        unsafe {
            ((*self.api).write_array_entry_end)((*self.api).private_data);
        }
    }

    pub fn write_s8(&mut self, id: &str, v: i8) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_s8)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_u8(&mut self, id: &str, v: u8) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_u8)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_s16(&mut self, id: &str, v: i16) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_s16)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_u16(&mut self, id: &str, v: i16) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_s16)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_s32(&mut self, id: &str, v: i32) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_s32)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_u32(&mut self, id: &str, v: i32) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_s32)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_s64(&mut self, id: &str, v: i64) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_s64)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_u64(&mut self, id: &str, v: i64) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_s64)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_float(&mut self, id: &str, v: f32) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_float)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_double(&mut self, id: &str, v: f64) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_double)((*self.api).private_data, s.as_ptr(), v);
        }
    }

    pub fn write_string(&mut self, id: &str, v: &str) {
        let id_s = CString::new(id).unwrap();
        let v_s = CString::new(v).unwrap();
        unsafe {
            ((*self.api).write_string)((*self.api).private_data, id_s.as_ptr(), v_s.as_ptr());
        }
    }

    pub fn write_data(&mut self, id: &str, data: &[u8]) {
        let s = CString::new(id).unwrap();
        unsafe {
            ((*self.api).write_data)((*self.api).private_data,
                                     s.as_ptr(),
                                     data.as_ptr(),
                                     data.len() as u32);
        }
    }
}
