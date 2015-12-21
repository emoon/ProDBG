use libc::{c_char, c_int, c_uint, c_void, size_t};
use std::ffi::CStr;
use std::str::from_utf8;

pub type c_bool = c_int;

pub type csh = *const c_void;
pub type cs_err = c_int;
pub type cs_opt_type = c_int;

#[repr(C)]
pub struct CsInsn {
    pub id: c_uint,
    pub address: u64,
    pub size: u16,
    pub bytes: [u8; 16],
    pub mnemonic: [u8; 32],
    pub op_str: [u8; 160],
    pub detail: *const c_void,
}

#[repr(C)]
struct CCapstone1 {
    version: extern "C" fn (major: *mut c_int, minor: *mut c_int) -> c_uint,
    support: extern "C" fn(query: c_int) -> c_bool,
    open: extern "C" fn(arch: c_int, mode: c_int, handle: *mut csh) -> cs_err,
    close: extern "C" fn(handle: *mut csh) -> cs_err,
    option: extern "C" fn(handle: csh, _type: cs_opt_type, value: size_t) -> cs_err,
    err: extern "C" fn(handle: csh) -> cs_err,
    disasm: extern "C" fn(handle: csh, code: *const u8, code_size: size_t, address: u64, count: size_t, insn: *mut *mut cs_insn) -> size_t,
    /*
     To be implemented
    dissam_iter: extern "C" fn(handle: csh, reg_id: c_uint) -> *const c_char;
    reg_name: extern "C" fn(handle: csh, reg_id: c_uint) -> *const c_char;
    insn_name: extern "C" fn(handle: csh, insn_id: c_uint) -> *const c_char;
    insn_group: extern "C" fn(handle: csh, insn: *mut cs_insn, group_id: c_uint) -> c_bool;
    reg_read: extern "C" fn(handle: csh, insn: *mut cs_insn, reg_id: c_uint) -> c_bool;
    reg_write: extern "C" fn(handle: csh, insn: *mut cs_insn, reg_id: c_uint) -> c_bool;
    op_count: extern "C" fn(handle: csh, insn: *mut cs_insn, op_type: c_uint) -> c_int;
    op_index: extern "C" fn(handle: csh, insn: *mut cs_insn, op_type: c_uint, position: c_uint) -> c_int;
    */
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
    Mode,
    // OptMem
}

enum Error {
	Ok, // No error: everything was fine
	Mem, // Out-Of-Memory error: cs_open(), cs_disasm(), cs_disasm_iter()
	Arch, // Unsupported architecture: cs_open()
	Handle, // Invalid handle: cs_op_count(), cs_op_index()
	Csh, // Invalid csh argument: cs_close(), cs_errno(), cs_option()
	Mode, // Invalid/unsupported mode: cs_open()
	InvOption, // Invalid/unsupported option: cs_option()
	Detail, // Information is unavailable because detail option is OFF
	MemSetup, // Dynamic memory management uninitialized (see CS_OPT_MEM)
	Version, // Unsupported version (bindings)
	Diet, // Access irrelevant data in "diet" engine
	Skipdata, // Access irrelevant data for "data" instruction in SKIPDATA mode
	Att, // X86 AT&T syntax is unsupported (opt-out at compile time)
	Intel, // X86 Intel syntax is unsupported (opt-out at compile time)
	Mask, // X86 Intel syntax is unsupported (opt-out at compile time)
}

pub struct Capstone {
    pub api: *mut CCapstone1,
    handle: *const c_void
}



impl Capstone {
    pub fn open(&mut self, arch: Arch, mode: Mode) -> CsErr {
        let mut handle : *const c_void = 0 as *const c_void;
        unsafe {
            let err: Error = ((*self.api).open)(arch as c_int, mode.bits as c_int, &mut handle) as Error;
            self.handle = handle;
            err
        }
    }


    


}

