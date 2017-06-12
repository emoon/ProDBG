use rust_ffi::*;

pub struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

pub struct Widget {
  obj: const* PU Widget,
}

pub struct PushButton {
  obj: const* PU PushButton,
}

pub struct Slider {
  obj: const* PU Slider,
}

pub struct Painter {
  obj: const* PU Painter,
}

