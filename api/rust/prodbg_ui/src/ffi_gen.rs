use std::os::raw::{c_void, c_char};

#[repr(C)]
pub struct PURect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

#[repr(C)]
pub struct PUWidget {
    pub show: extern "C" fn(self_c: *const c_void),
    pub privd: *const c_void,
}

#[repr(C)]
pub struct PUPushButton {
    pub connect_released: extern "C" fn(self_c: *const c_void, user_data: *const c_void,
                                        callback: extern "C" fn(self_c: *const c_void)),
    pub set_text: extern "C" fn(self_c: *const c_void, text: *const c_char),
    pub set_flat: extern "C" fn(self_c: *const c_void, flat: bool),
    pub privd: *const c_void,
}

#[repr(C)]
pub struct PUSlider {
    pub connect_value_changed: extern "C" fn(self_c: *const c_void, user_data: *const c_void,
                                        callback: extern "C" fn(self_c: *const c_void)),
    pub privd: *const c_void,
}

#[repr(C)]
pub struct PUPainter {
    pub draw_line: extern "C" fn(self_c: *const c_void, x1: i32, y1: i32, x2: i32, y2: i32),
    pub privd: *const c_void,
}

#[repr(C)]
pub struct PU {
    pub create_widget: extern "C" fn(priv_data: *const c_void) -> *const PUWidget,
    pub create_push_button: extern "C" fn(priv_data: *const c_void) -> *const PUPushButton,
    pub create_slider: extern "C" fn(priv_data: *const c_void) -> *const PUSlider,
    pub create_painter: extern "C" fn(priv_data: *const c_void) -> *const PUPainter,
    pub privd: *const c_void,
}

