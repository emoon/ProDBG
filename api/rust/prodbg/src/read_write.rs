use std::mem::transmute;
use std::slice;
use std::str;
use std::os::raw::*;
use CFixedString;

#[repr(C)]
pub struct CPDReaderAPI {
    pub data: *mut c_void,
    pub read_get_event: extern "C" fn(reader: *mut c_void) -> c_uint,
    pub read_iterator_next_event: extern "C" fn(reader: *mut c_void, it: *mut c_ulonglong) -> c_uint,
    pub read_iterator_begin: extern "C" fn(reader: *mut c_void,
                                           it: *mut c_ulonglong,
                                           keyName: *mut *const c_char,
                                           parentIt: c_ulonglong)
                                           -> c_uint,
    pub read_iterator_next: extern "C" fn(reader: *mut c_void,
                                          keyName: *mut *const c_char,
                                          it: *mut c_ulonglong)
                                          -> c_uint,
    pub read_next_entry: extern "C" fn(reader: *mut c_void, arrayIt: *mut c_ulonglong) -> c_int,
    pub read_find_s8: extern "C" fn(reader: *mut c_void,
                                    res: *mut c_char,
                                    id: *const c_char,
                                    it: c_ulonglong)
                                    -> c_uint,
    pub read_find_u8: extern "C" fn(reader: *mut c_void,
                                    res: *mut c_uchar,
                                    id: *const c_char,
                                    it: c_ulonglong)
                                    -> c_uint,
    pub read_find_s16: extern "C" fn(reader: *mut c_void,
                                     res: *mut c_short,
                                     id: *const c_char,
                                     it: c_ulonglong)
                                     -> c_uint,
    pub read_find_u16: extern "C" fn(reader: *mut c_void,
                                     res: *mut c_ushort,
                                     id: *const c_char,
                                     it: c_ulonglong)
                                     -> c_uint,
    pub read_find_s32: extern "C" fn(reader: *mut c_void,
                                     res: *mut c_int,
                                     id: *const c_char,
                                     it: c_ulonglong)
                                     -> c_uint,
    pub read_find_u32: extern "C" fn(reader: *mut c_void,
                                     res: *mut c_uint,
                                     id: *const c_char,
                                     it: c_ulonglong)
                                     -> c_uint,
    pub read_find_s64: extern "C" fn(reader: *mut c_void,
                                     res: *mut c_longlong,
                                     id: *const c_char,
                                     it: c_ulonglong)
                                     -> c_uint,
    pub read_find_u64: extern "C" fn(reader: *mut c_void,
                                     res: *mut c_ulonglong,
                                     id: *const c_char,
                                     it: c_ulonglong)
                                     -> c_uint,
    pub read_find_float: extern "C" fn(reader: *mut c_void,
                                       res: *mut c_float,
                                       id: *const c_char,
                                       it: c_ulonglong)
                                       -> c_uint,
    pub read_find_double: extern "C" fn(reader: *mut c_void,
                                        res: *mut c_double,
                                        id: *const c_char,
                                        it: c_ulonglong)
                                        -> c_uint,
    pub read_find_string: extern "C" fn(reader: *mut c_void,
                                        res: *mut *const c_char,
                                        id: *const c_char,
                                        it: c_ulonglong)
                                        -> c_uint,
    pub read_find_data: extern "C" fn(reader: *mut c_void,
                                      data: *mut *mut c_void,
                                      size: *mut c_ulonglong,
                                      id: *const c_char,
                                      it: c_ulonglong)
                                      -> c_uint,
    pub read_find_array: extern "C" fn(reader: *mut c_void,
                                       arrayIt: *mut c_ulonglong,
                                       id: *const c_char,
                                       it: c_ulonglong)
                                       -> c_uint,
    pub read_dump_data: extern "C" fn(reader: *mut c_void),
}

#[repr(C)]
pub enum WriteStatus {
    Ok,
    Fail,
}

#[repr(C)]
pub struct CPDWriterAPI {
    private_data: *mut c_void,
    pub write_event_begin: extern "C" fn(writer: *mut c_void, event: c_ushort) -> u64,
    pub write_event_end: extern "C" fn(writer: *mut c_void) -> WriteStatus,
    pub write_header_array_begin: extern "C" fn(writer: *mut c_void, ids: *mut *const c_char)
                                                -> WriteStatus,
    pub write_header_array_end: extern "C" fn(writer: *mut c_void) -> WriteStatus,
    pub write_array_begin: extern "C" fn(writer: *mut c_void, name: *const c_char) -> WriteStatus,
    pub write_array_end: extern "C" fn(writer: *mut c_void) -> WriteStatus,
    pub write_array_entry_begin: extern "C" fn(writer: *mut c_void) -> WriteStatus,
    pub write_array_entry_end: extern "C" fn(writer: *mut c_void) -> WriteStatus,
    pub write_s8: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_char) -> WriteStatus,
    pub write_u8: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_uchar) -> WriteStatus,
    pub write_s16: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_short) -> WriteStatus,
    pub write_u16: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_ushort) -> WriteStatus,
    pub write_s32: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_int) -> WriteStatus,
    pub write_u32: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_uint) -> WriteStatus,
    pub write_s64: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_longlong)
                                 -> WriteStatus,
    pub write_u64: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_ulonglong)
                                 -> WriteStatus,
    pub write_float: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_float) -> WriteStatus,
    pub write_double: extern "C" fn(writer: *mut c_void, id: *const c_char, v: c_double)
                                    -> WriteStatus,
    pub write_string: extern "C" fn(writer: *mut c_void, id: *const c_char, v: *const c_char)
                                    -> WriteStatus,
    pub write_data: extern "C" fn(w: *mut c_void,
                                  id: *const c_char,
                                  d: *const c_uchar,
                                  l: c_uint)
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
#[derive(Debug)]
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

pub struct EventIter {
    reader: Reader,
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

macro_rules! find_fun {
    ($c_name:ident, $name:ident, $data_type:ident) => {
        pub fn $name(&self, id: &str) -> Result<$data_type, ReadStatus> {
            let s = CFixedString::from_str(id).as_ptr();
            let mut res = 0 as $data_type;
            let ret;

            unsafe {
                ret = ((*self.api).$c_name)(transmute(self.api), &mut res, s, self.it);
            }

            return status_res(res, ret);
        }
    }
}

/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

impl Reader {
    pub fn new(in_api: *mut CPDReaderAPI, iter: u64) -> Self {
        return Reader {
            api: in_api,
            it: iter,
        };
    }

    pub fn get_events(&self) -> EventIter {
        EventIter { reader: self.clone() }
    }

    pub fn get_event(&self) -> Option<i32> {
        let event_id = unsafe { ((*self.api).read_get_event)(transmute(self.api)) as i32 };

        match event_id {
            0 => None,
            e => Some(e),
        }
    }

    find_fun!(read_find_s8, find_s8, i8);
    find_fun!(read_find_u8, find_u8, u8);
    find_fun!(read_find_s16, find_s16, i16);
    find_fun!(read_find_u16, find_u16, u16);
    find_fun!(read_find_s32, find_s32, i32);
    find_fun!(read_find_u32, find_u32, u32);
    find_fun!(read_find_s64, find_s64, i64);
    find_fun!(read_find_u64, find_u64, u64);
    find_fun!(read_find_float, find_float, f32);
    find_fun!(read_find_double, find_double, f64);

    unsafe fn strlen(t: *const i8) -> usize {
        // very lange slice so this is kinda hacky but should work
        let slice = slice::from_raw_parts(t, 16834);
        let mut count = 0;

        loop {
            if slice[count] == 0 {
                break;
            }
            count += 1;
        }

        count
    }

    pub fn find_string(&self, id: &str) -> Result<&str, ReadStatus> {
        let s = CFixedString::from_str(id).as_ptr();
        let mut temp = 0 as *const c_char;
        let ret;
        let mut res = "";

        unsafe {
            ret = ((*self.api).read_find_string)(transmute(self.api), &mut temp, s, self.it);
            let t = (ret >> 8) & 0xff;
            if t == 1 {
                let slice = slice::from_raw_parts(temp as *const u8, Self::strlen(temp));
                res = str::from_utf8(slice).unwrap();
            }
        }

        return status_res(res, ret);
    }

    pub fn find_data(&self, id: &str) -> Result<&[u8], ReadStatus> {
        let s = CFixedString::from_str(id).as_ptr();
        let mut temp = 0 as *mut c_void;
        let mut size = 0 as c_ulonglong;

        unsafe {
            let ret =
                ((*self.api).read_find_data)(transmute(self.api), &mut temp, &mut size, s, self.it);
            let slice = slice::from_raw_parts(temp as *const u8, size as usize);
            status_res(slice, ret)
        }
    }

    pub fn find_array(&self, id: &str) -> Result<ReaderIter, ReadStatus> {
        let s = CFixedString::from_str(id).as_ptr();
        let mut t = 0u64;

        unsafe {
            let ret = ((*self.api).read_find_array)(transmute(self.api), &mut t, s, 0);
            let iter = ReaderIter {
                reader: self.clone(),
                curr_iter: t,
            };
            status_res(iter, ret)
        }
    }
}

impl Iterator for ReaderIter {
    type Item = Reader;
    fn next(&mut self) -> Option<Reader> {
        let ret;
        unsafe {
            ret = ((*self.reader.api).read_next_entry)(transmute(self.reader.api),
                                                       &mut self.curr_iter);
        }

        match ret {
            0 => None,
            _ => Some(Reader::new(self.reader.api, self.curr_iter)),
        }
    }
}

impl Iterator for EventIter {
    type Item = i32;
    fn next(&mut self) -> Option<i32> {
        self.reader.get_event()
    }
}

macro_rules! write_fun {
    ($name:ident, $data_type:ident) => {
        pub fn $name(&mut self, id: &str, v: $data_type) {
            let s = CFixedString::from_str(id).as_ptr();
            unsafe {
                ((*self.api).$name)(transmute(self.api), s, v);
            }
        }
    }
}

impl Writer {
    pub fn event_begin(&mut self, event: u16) -> u64 {
        unsafe {
            ((*self.api).write_event_begin)(transmute(self.api), event);
        }
    }

    pub fn event_end(&mut self) {
        unsafe {
            ((*self.api).write_event_end)(transmute(self.api));
        }
    }

    pub fn array_begin(&mut self, name: &str) {
        let s = CFixedString::from_str(name).as_ptr();
        unsafe {
            ((*self.api).write_array_begin)(transmute(self.api), s);
        }
    }

    pub fn array_end(&mut self) {
        unsafe {
            ((*self.api).write_array_end)(transmute(self.api));
        }
    }

    pub fn array_entry_begin(&mut self) {
        unsafe {
            ((*self.api).write_array_entry_begin)(transmute(self.api));
        }
    }

    pub fn array_entry_end(&mut self) {
        unsafe {
            ((*self.api).write_array_entry_end)(transmute(self.api));
        }
    }

    write_fun!(write_s8, i8);
    write_fun!(write_u8, u8);
    write_fun!(write_s16, i16);
    write_fun!(write_u16, u16);
    write_fun!(write_s32, i32);
    write_fun!(write_u32, u32);
    write_fun!(write_s64, i64);
    write_fun!(write_u64, u64);
    write_fun!(write_float, f32);
    write_fun!(write_double, f64);

    pub fn write_string(&mut self, id: &str, v: &str) {
        let id_s = CFixedString::from_str(id).as_ptr();
        let v_s = CFixedString::from_str(v).as_ptr();
        unsafe {
            ((*self.api).write_string)(transmute(self.api), id_s, v_s);
        }
    }

    pub fn write_data(&mut self, id: &str, data: &[u8]) {
        let s = CFixedString::from_str(id).as_ptr();
        unsafe {
            ((*self.api).write_data)(transmute(self.api), s, data.as_ptr(), data.len() as u32);
        }
    }
}
