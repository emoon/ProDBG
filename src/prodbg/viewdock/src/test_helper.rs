use rect::Rect;

pub fn floats_are_equal(inv: f32, value: f32) -> bool {
    (inv - value).abs() < 1.19209290e-07f32
}

pub fn rects_are_equal(first: Rect, second: Rect) -> bool {
    floats_are_equal(first.x, second.x)
    && floats_are_equal(first.y, second.y)
    && floats_are_equal(first.width, second.width)
    && floats_are_equal(first.height, second.height)
}
