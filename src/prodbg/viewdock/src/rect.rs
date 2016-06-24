use area::Direction;

/// Data structure for rectangles
#[derive(Debug, Default, Clone, Copy)]
pub struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

impl Rect {
    pub fn new(x: f32, y: f32, width: f32, height: f32) -> Rect {
        Rect {
            x: x,
            y: y,
            width: width,
            height: height
        }
    }

    pub fn point_is_inside(&self, pos: (f32, f32)) -> bool {
        let (x, y) = pos;
        return
            self.x <= x &&
            self.x + self.width >= x &&
            self.y <= y &&
            self.y + self.height >= y;
    }

    pub fn split_by_direction(&self, direction: Direction, ratio: f32) -> (Rect, Rect) {
        match direction {
            Direction::Horizontal => Rect::split_horizontally(self, ratio),
            Direction::Vertical => Rect::split_vertically(self, ratio),
        }
    }

    pub fn split_horizontally(rect: &Rect, ratio: f32) -> (Rect, Rect) {
        let h = rect.height * ratio;

        let rect_top = Rect::new(rect.x, rect.y, rect.width, h);
        let rect_bottom = Rect::new(rect.x, rect.y + h, rect.width, rect.height - h);

        (rect_top, rect_bottom)
    }

    pub fn split_vertically(rect: &Rect, ratio: f32) -> (Rect, Rect) {
        let w = rect.width * ratio;

        let rect_left = Rect::new(rect.x, rect.y, w, rect.height);
        let rect_right = Rect::new(rect.x + w, rect.y, rect.width - w, rect.height);

        (rect_left, rect_right)
    }
}
