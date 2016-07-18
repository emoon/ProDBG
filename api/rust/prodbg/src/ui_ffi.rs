
// Disable this warning until all code has been fixed
#[allow(non_upper_case_globals)]

extern crate bitflags;

use std::os::raw::{c_char, c_uchar, c_float, c_int, c_uint, c_ushort, c_void};
use scintilla::PDUISCInterface;

// struct PDVec2
// (float) x
// (float) y
//
#[repr(C)]
#[derive(Debug)]
pub struct PDVec2 {
    pub x: c_float,
    pub y: c_float,
}

// struct PDVec4
// (float) x
// (float) y
// (float) z
// (float) w
//
#[repr(C)]
pub struct PDVec4 {
    pub x: c_float,
    pub y: c_float,
    pub z: c_float,
    pub w: c_float,
}

// struct PDUISCInterface
// (intptr_t (*)(void *, unsigned int, uintptr_t, intptr_t)) send_command [long long (*)(void *, unsigned int, unsigned long long, long long)]
// (void (*)(void *)) update [void (*)(void *)]
// (void (*)(void *)) draw [void (*)(void *)]
// (void *) private_data
//

// struct PDUIInputTextCallbackData
// (PDUIInputTextFlags) event_flag [unsigned int]
// (PDUIInputTextFlags) flags [unsigned int]
// (void *) user_data
// (uint16_t) event_char [unsigned short]
// (uint16_t) event_key [unsigned short]
// (char *) buf
// (int) buf_size
// (int) buf_dirty
// (int) cursor_pos
// (int) selection_start
// (int) selection_end
// (void (*)(struct PDUIInputTextCallbackData *, int, int)) delete_chars [void (*)(struct PDUIInputTextCallbackData *, int, int)]
// (void (*)(struct PDUIInputTextCallbackData *, int, const char *, const char *)) insert_chars [void (*)(struct PDUIInputTextCallbackData *, int, const char *, const char *)]
//
#[repr(C)]
pub struct PDUIInputTextCallbackData {
    pub event_flag: c_uint,
    pub flags: c_uint,
    pub user_data: *mut c_void,
    pub event_char: c_ushort,
    pub event_key: c_ushort,
    pub buf: *mut c_char,
    pub buf_size: c_int,
    pub buf_dirty: c_int,
    pub cursor_pos: c_int,
    pub selection_start: c_int,
    pub selection_end: c_int,
    pub delete_chars: Option<extern "C" fn(*mut PDUIInputTextCallbackData,
                                           c_int,
                                           c_int)>,
    pub insert_chars: Option<extern "C" fn(*mut PDUIInputTextCallbackData,
                                           c_int,
                                           *const c_char,
                                           *const c_char)>,
}

include!("ui_ffi_autogen.rs");

#[repr(C)]
pub struct PDRect {
    pub x: c_float,
    pub y: c_float,
    pub width: c_float,
    pub height: c_float,
}

bitflags! {
	flags PDUIWindowFlags_: c_uint {
		const PDUIWINDOWFLAGS_NOTITLEBAR = 1 as c_uint,
		const PDUIWINDOWFLAGS_NORESIZE = 2 as c_uint,
		const PDUIWINDOWFLAGS_NOMOVE =	4 as c_uint,
		const PDUIWINDOWFLAGS_NOSCROLLBAR =	8 as c_uint,
		const PDUIWINDOWFLAGS_NOSCROLLWITHMOUSE = 16 as c_uint,
		const PDUIWINDOWFLAGS_NOCOLLAPSE =	32 as c_uint,
		const PDUIWINDOWFLAGS_ALWAYSAUTORESIZE = 64 as c_uint,
		const PDUIWINDOWFLAGS_SHOWBORDERS =	128 as c_uint,
		const PDUIWINDOWFLAGS_NOSAVEDSETTINGS =	256 as c_uint,
		const PDUIWINDOWFLAGS_NOINPUTS = 512 as c_uint,
		const PDUIWINDOWFLAGS_MENUBAR =	1024 as c_uint,
	}
}

#[repr(C)]
pub enum ImguiKey {
    Tab,       // for tabbing through fields
    LeftArrow, // for text edit
    RightArrow,// for text edit
    UpArrow,   // for text edit
    DownArrow, // for text edit
    PageUp,
    PageDown,
    Home,      // for text edit
    End,       // for text edit
    Delete,    // for text edit
    Backspace, // for text edit
    Enter,     // for text edit
    Escape,    // for text edit
    A,         // for text edit CTRL+A: select all
    C,         // for text edit CTRL+C: copy
    V,         // for text edit CTRL+V: paste
    X,         // for text edit CTRL+X: cut
    Y,         // for text edit CTRL+Y: redo
    Z,         // for text edit CTRL+Z: undo
}


