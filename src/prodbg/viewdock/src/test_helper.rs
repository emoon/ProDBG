use rect::Rect;
use {Area, DockHandle};

pub fn floats_are_equal(inv: f32, value: f32) -> bool {
    (inv - value).abs() < 1.19209290e-07f32
}

pub fn rects_are_equal(first: &Rect, second: &Rect) -> bool {
    floats_are_equal(first.x, second.x) && floats_are_equal(first.y, second.y) &&
    floats_are_equal(first.width, second.width) && floats_are_equal(first.height, second.height)
}

pub fn is_container_with_single_dock(target: &Area, id: u64) -> bool {
    match *target {
        Area::Container(ref c) => {
            if c.docks.len() != 1 {
                false
            } else {
                c.docks[0] == DockHandle(id)
            }
        }
        _ => false,
    }
}
