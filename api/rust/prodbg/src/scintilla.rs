use std::os::raw;

#[repr(C)]
pub struct PDUISCInterface {
    pub send_command: extern "C" fn(*mut raw::c_void, raw::c_uint, u64, u64) -> u64,
    pub update: extern "C" fn(*mut raw::c_void),
    pub draw: extern "C" fn(*mut raw::c_void),
    pub private_data: *mut raw::c_void,
}

pub struct Scintilla {
    api: *mut PDUISCInterface,
}

impl Scintilla {
    pub fn new(api: *mut PDUISCInterface) -> Scintilla {
        Scintilla { api: api }
    }

    #[inline]
    pub fn send_command(&self, message: u32, p0: u64, p1: u64) -> u64 {
        unsafe { ((*self.api).send_command)((*self.api).private_data, message, p0, p1) }
    }

    #[inline]
    pub fn update(&self) {
        unsafe { ((*self.api).update)((*self.api).private_data) }
    }

    #[inline]
    pub fn draw(&self) {
        unsafe { ((*self.api).draw)((*self.api).private_data) }
    }
}
