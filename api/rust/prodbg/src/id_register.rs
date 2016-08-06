use std::os::raw::{c_char, c_void};
use std::ffi::CString;
use std::slice;
use std::mem;

extern "C" fn string_hash(id_name: *const c_char) -> u16 {
    let slice = unsafe { slice::from_raw_parts(id_name, 16834) };
    let mut hash = 0u32;

    for i in slice {
        let v = *i as u32;
        if v == 0 {
            break;
        }
        hash.wrapping_add(v as u32);
        hash.wrapping_add(hash << 10);
        hash ^= hash >> 6;
    }

    hash.wrapping_add(hash << 3);
    hash ^= hash >> 11;
    hash.wrapping_add(hash << 15);

    // id start at 0x1000 and remove highest bit to prevent overflow
    hash &= 0x7fff;
    hash += 0x1000;
    hash as u16
}

pub struct IdFuncs {
    pub api: *mut CIdFuncs1,
}

impl IdFuncs {
    pub fn register_id(&self, id_name: &str) -> u16 {
        let d = CString::new(id_name).unwrap();
        unsafe { ((*self.api).register_id)(d.as_ptr()) }
    }
}

#[repr(C)]
pub struct CIdFuncs1 {
    register_id: extern "C" fn(id_name: *const c_char) -> u16,
}

static ID_FUNCS: CIdFuncs1 = CIdFuncs1 { register_id: string_hash };

pub fn get_id_register_funcs() -> *mut c_void {
    unsafe { mem::transmute(&ID_FUNCS) }
}
