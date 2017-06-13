use rust_ffi::*;

pub struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

pub struct Widget {
    obj: const* PUWidget,
}

pub struct PushButton {
    obj: const* PUPushButton,
}

pub struct Slider {
    obj: const* PUSlider,
}

pub struct Painter {
    obj: const* PUPainter,
}

impl Widget {
    pub fn show(&self) {
        unsafe {
            ((*self.obj).show)((*self.obj).priv_data));
        }
    }

}

impl PushButton {
    pub fn show(&self) {
        unsafe {
            ((*self.obj).show)((*self.obj).priv_data));
        }
    }

    pub fn set_text(&selftext: &str) {
        let str_in_0 = CString::new(text).unwrap();
        unsafe {
            ((*self.obj).set_text)(str_in_0.get_ptr(), (*self.obj).priv_data));
        }
    }

    pub fn set_flat(&selfflat: bool) {
        unsafe {
            ((*self.obj).set_flat)(flat, (*self.obj).priv_data));
        }
    }

}

impl Slider {
}

impl Painter {
    pub fn draw_line(&selfx1: i32, y1: i32, x2: i32, y2: i32) {
        unsafe {
            ((*self.obj).draw_line)(x1, y1, x2, y2, (*self.obj).priv_data));
        }
    }

}

