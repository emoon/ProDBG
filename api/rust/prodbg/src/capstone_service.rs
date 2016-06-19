use std::os::raw::{c_int, c_uint, c_void};
use std::fmt::{Debug, Formatter};
use std::ffi::CStr;
use std::mem::transmute;
use std::slice;
use std::ptr;
use std::str::from_utf8;
use capstone_m68k::cs_detail;

#[repr(C)]
pub struct Insn {
    pub id: c_uint,
    pub address: u64,
    pub size: u16,
    pub bytes: [u8; 16],
    pub mnemonic: [i8; 32],
    pub op_str: [i8; 160],
    pub detail: *const c_void,
}

#[repr(C)]
pub struct CCapstone1 {
    version: extern "C" fn(major: *mut c_int, minor: *mut c_int) -> c_uint,
    support: extern "C" fn(query: c_int) -> c_int,
    open: extern "C" fn(arch: c_int, mode: c_int, handle: *mut *const c_void) -> c_int,
    close: extern "C" fn(handle: *mut *const c_void) -> c_int,
    option: extern "C" fn(handle: *const c_void, _type: c_int, value: usize) -> c_int,
    err: extern "C" fn(handle: *const c_void) -> c_int,
    disasm: extern "C" fn(handle: *const c_void,
                          code: *const u8,
                          code_size: usize,
                          address: u64,
                          count: usize,
                          insn: &mut *const Insn)
                          -> usize,
    free: extern "C" fn(insn: *const Insn, count: usize),

    disasm_iter: extern "C" fn(handle: *const c_void,
                              code: *const u8,
                              code_size: *const usize) -> usize,
    regname: extern "C" fn(handle: *const c_void, id: u16) -> *const i8,
}

#[derive(Clone, Copy, Debug)]
pub enum Arch {
    Arm = 0,
    Arm64,
    MIPS,
    X86,
    PowerPC,
    Sparc,
    SystemZ,
    XCore,
    M68K,
}

bitflags!(
    flags Mode: u32 {
        const MODE_LITTLE_ENDIAN = 0,
        const MODE_ARM = 0,
        const MODE_16 = 1 << 1,
        const MODE_32 = 1 << 2,
        const MODE_64 = 1 << 3,
        const MODE_THUMB = 1 << 4,
        const MODE_MCLASS = 1 << 5,
        const MODE_V8 = 1 << 6,
        const MODE_MICRO = 1 << 4,
        const MODE_MIPS3 = 1 << 5,
        const MODE_MIPS32R6 = 1 << 6,
        const MODE_MIPSGP64 = 1 << 7,
        const MODE_V9 = 1 << 4,
        const CS_MODE_QPX = 1 << 4,
        const CS_MODE_M68K_000 = 1 << 1,
        const CS_MODE_M68K_010 = 1 << 2,
        const CS_MODE_M68K_020 = 1 << 3,
        const CS_MODE_M68K_030 = 1 << 4,
        const CS_MODE_M68K_040 = 1 << 5,
        const CS_MODE_M68K_060 = 1 << 6,
        const MODE_BIG_ENDIAN= 1 << 31,
        const MODE_MIPS32 = 1 << 2,
        const MODE_MIPS64 = 1 << 3,
    }
);

#[derive(Clone, Copy, Debug)]
pub enum Opt {
    Syntax = 1,
    Detail,
    Mode, // OptMem
}

#[repr(i32)]
pub enum Error {
    /// No error: everything was fine
    Ok,
    /// Out-Of-Memory error: cs_open(), cs_disasm(), cs_disasm_iter()
    Mem,
    /// Unsupported architecture: cs_open()
    Arch,
    /// Invalid handle: cs_op_count(), cs_op_index()
    Handle,
    /// Invalid csh argument: cs_close(), cs_errno(), cs_option()
    Csh,
    /// Invalid/unsupported mode: cs_open()
    Mode,
    /// Invalid/unsupported option: cs_option()
    InvOption,
    /// Information is unavailable because detail option is OFF
    Detail,
    /// Dynamic memory management uninitialized (see CS_OPT_MEM)
    MemSetup,
    /// Unsupported version (bindings)
    Version,
    /// Access irrelevant data in "diet" engine
    Diet,
    /// Access irrelevant data for "data" instruction in SKIPDATA mode
    Skipdata,
    /// X86 AT&T syntax is unsupported (opt-out at compile time)
    Att,
    /// X86 Intel syntax is unsupported (opt-out at compile time)
    Intel,
    /// X86 Intel syntax is unsupported (opt-out at compile time)
    Mask,
}

pub struct Capstone {
    pub api: *mut CCapstone1,
    pub handle: *const c_void,
}

impl Capstone {
    pub fn open(&mut self, arch: Arch, mode: Mode) -> Result<(), Error> {
        // If handle is already setup we just return from here
        if self.handle != ptr::null_mut() {
            return Ok(());
        }


        let mut handle: *const c_void = 0 as *const c_void;
        unsafe {
            match ((*self.api).open)(arch as c_int, mode.bits as c_int, &mut handle) {
                0 => {
                    self.handle = handle;
                    Ok(())
                }
                e => {
                    let err: Error = transmute(e);
                    Err(err)
                }
            }
        }
    }

    pub fn set_option(&self, option: Opt, value: usize) -> Result<(), Error> {
        unsafe {
            match ((*self.api).option)(self.handle, option as c_int, value as usize) {
                0 => Ok(()),
                e => {
                    let err: Error = transmute(e);
                    Err(err)
                }
            }
        }
    }

    pub fn disasm(&self, code: &[u8], addr: u64, count: usize) -> Result<Instructions, Error> {
        let mut ptr: *const Insn = ptr::null();
        let insn_count;
        unsafe {
            insn_count = ((*self.api).disasm)(self.handle,
                                              code.as_ptr(),
                                              code.len() as usize,
                                              addr,
                                              count as usize,
                                              &mut ptr) as isize;
        }

        if insn_count == 0 {
            unsafe {
                let num = ((*self.api).err)(self.handle);
                let err: Error = transmute(num);
                return Err(err);
            }
        }

        Ok(Instructions::from_raw_parts(self.api, ptr, insn_count as isize))
    }

    pub fn reg_name(&self, reg_id: u16) -> &str {
        unsafe {
            let name = ((*self.api).regname)(self.handle, reg_id);

            // very lange slice so this is kinda hacky but should work
            let t = slice::from_raw_parts(name, 16834);
            let mut count = 0;

            loop {
                if t[count] == 0 { break; }
                count += 1;
            }

            let slice = slice::from_raw_parts(name as *const u8, count); 
            from_utf8(slice).unwrap()
        }
    }
}

// Using an actual slice is causing issues with auto deref, instead implement a custom iterator and
// drop trait
pub struct Instructions {
    pub api: *mut CCapstone1,
    ptr: *const Insn,
    len: isize,
}

impl Instructions {
    // This method really shouldn't be public, but it was unclear how to make it visible in lib.rs
    // but not globally visible.
    fn from_raw_parts(api: *mut CCapstone1, ptr: *const Insn, len: isize) -> Instructions {
        Instructions {
            api: api,
            ptr: ptr,
            len: len,
        }
    }

    pub fn len(&self) -> isize {
        self.len
    }

    pub fn iter(&self) -> InstructionIterator {
        InstructionIterator {
            insns: &self,
            cur: 0,
        }
    }
}

impl Drop for Instructions {
    fn drop(&mut self) {
        unsafe {
            ((*self.api).free)(self.ptr, self.len as usize);
        }
    }
}

pub struct InstructionIterator<'a> {
    insns: &'a Instructions,
    cur: isize,
}

impl<'a> Iterator for InstructionIterator<'a> {
    type Item = Insn;

    fn next(&mut self) -> Option<Insn> {
        if self.cur == self.insns.len {
            None
        } else {
            let obj = unsafe { self.insns.ptr.offset(self.cur) };
            self.cur += 1;
            Some(unsafe { ptr::read(obj) })
        }
    }
}

impl Insn {
    pub fn mnemonic(&self) -> Option<&str> {
        let cstr = unsafe { CStr::from_ptr(self.mnemonic.as_ptr()) };
        from_utf8(cstr.to_bytes()).ok()
    }

    pub fn op_str(&self) -> Option<&str> {
        let cstr = unsafe { CStr::from_ptr(self.op_str.as_ptr()) };
        from_utf8(cstr.to_bytes()).ok()
    }

    pub fn detail(&self) -> Option<&cs_detail> {
        unsafe {
            if self.detail == ptr::null() {
                return None;
            }

            let d: &cs_detail = transmute(self.detail);
            Some(d)
        }
    }

    pub fn regs_read(&self) -> Option<&[u16]> {
        unsafe {
            if self.detail == ptr::null() {
                return None;
            }

            let detail: &cs_detail = transmute(self.detail);
            Some(&slice::from_raw_parts(detail.regs_read.as_ptr(), detail.regs_read_count as usize))
            //Some(&detail.regs_read)
        }
    }

    pub fn regs_write(&self) -> Option<&[u16]> {
        unsafe {
            if self.detail == ptr::null() {
                return None;
            }

            let detail: &cs_detail = transmute(self.detail);
            Some(&slice::from_raw_parts(detail.regs_write.as_ptr(), detail.regs_write_count as usize))
        }
    }
}

impl Debug for Insn {
    fn fmt(&self, fmt: &mut Formatter) -> Result<(), ::std::fmt::Error> {
        fmt.debug_struct("Insn")
           .field("address", &self.address)
           .field("size", &self.size)
           .field("mnemonic", &self.mnemonic())
           .field("op_str", &self.op_str())
           .finish()
    }
}
