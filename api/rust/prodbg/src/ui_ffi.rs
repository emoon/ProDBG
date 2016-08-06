
// Disable this warning until all code has been fixed
#[allow(non_upper_case_globals)]

extern crate bitflags;

use std::os::raw::{c_char, c_float, c_int, c_uint, c_ushort, c_void};
use scintilla::PDUISCInterface;

include!("ui_ffi_autogen.rs");

#[repr(C)]
pub enum ImguiKey {
    Tab, // for tabbing through fields
    LeftArrow, // for text edit
    RightArrow, // for text edit
    UpArrow, // for text edit
    DownArrow, // for text edit
    PageUp,
    PageDown,
    Home, // for text edit
    End, // for text edit
    Delete, // for text edit
    Backspace, // for text edit
    Enter, // for text edit
    Escape, // for text edit
    A, // for text edit CTRL+A: select all
    C, // for text edit CTRL+C: copy
    V, // for text edit CTRL+V: paste
    X, // for text edit CTRL+X: cut
    Y, // for text edit CTRL+Y: redo
    Z, // for text edit CTRL+Z: undo
}
