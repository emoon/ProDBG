use std::os::raw::{c_void, c_char};

#[repr(C)]
pub struct PUObjectFuncs {
    pub connect: extern "C" fn(sender: *const c_void, id: *const c_char, reciver: *const c_void, func: *const c_void) -> i32,
}

#[repr(C)]
pub struct PUWidgetFuncs {
    pub set_size: extern "C" fn(widget: *const PUWidget, width: i32, height: i32),
    pub show: extern "C" fn(widget: *const PUWidget),
}

#[repr(C)]
pub struct PUPushButtonFuncs {
    pub set_title: extern "C" fn(button: *const PUPushButton, text: *const c_char),
}

#[repr(C)]
pub struct PUObject {
    pub p: *const c_void,
    pub object_funcs: *const PUObjectFuncs,
}

#[repr(C)]
pub struct PUWidget {
    pub object: *const PUObject,
}

#[repr(C)]
pub struct PUPushButton {
    pub base: *const PUWidget,
}

#[repr(C)]
pub struct PU {
    pub api_version: u64,
    pub push_button_create: extern "C" fn() -> *const PUPushButton,
    pub widget_funcs: *const PUWidgetFuncs,
    pub push_button_funcs: *const PUPushButtonFuncs,
}

