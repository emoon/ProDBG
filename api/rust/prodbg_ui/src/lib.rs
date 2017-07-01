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
            ((*self.obj).show)((*self.obj).privd)));
        }
    }

}

impl PushButton {
    pub fn show(&self) {
        unsafe {
            ((*self.obj).show)((*self.obj).privd)));
        }
    }

    pub fn set_text(&self, text: &str) {
        let str_in_0 = CString::new(text).unwrap();
        unsafe {
            ((*self.obj).set_text)((*self.obj).privd)), str_in_0.get_ptr());
        }
    }

    pub fn set_flat(&self, flat: bool) {
        unsafe {
            ((*self.obj).set_flat)((*self.obj).privd)), flat);
        }
    }

}

impl Slider {
}

impl Painter {
    pub fn draw_line(&self, x1: i32, y1: i32, x2: i32, y2: i32) {
        unsafe {
            ((*self.obj).draw_line)((*self.obj).privd)), x1, y1, x2, y2);
        }
    }

}

pub struct Ui {
    pu: *const PU
}

impl Ui {
    pub fn new(pu: *const PU) -> Ui { Ui { pu: pu } }

    pub fn create_widget(&self) -> Widget {
        Widget { obj: (*self.obj)(create_widget)() }
    }

    pub fn create_push_button(&self) -> PushButton {
        PushButton { obj: (*self.obj)(create_push_button)() }
    }

    pub fn create_slider(&self) -> Slider {
        Slider { obj: (*self.obj)(create_slider)() }
    }

    pub fn create_painter(&self) -> Painter {
        Painter { obj: (*self.obj)(create_painter)() }
    }

}
