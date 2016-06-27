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

    pub fn area_around_split(&self, direction: Direction, ratio: f32, width: f32) -> Rect {
        match direction {
            Direction::Horizontal => {
                let y_start = self.y + self.height * ratio - width / 2.0;
                return Rect::new(self.x, y_start, self.width, width);
            },
            Direction::Vertical => {
                let x_start = self.x + self.width * ratio - width / 2.0;
                return Rect::new(x_start, self.y, width, self.height);
            },
        }
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

#[cfg(test)]
mod test {
    use Rect;

    fn check_range(inv: f32, value: f32, delta: f32) -> bool {
        (inv - value).abs() < delta
    }

    #[test]
    fn test_calc_rect_horz_half() {
        let rects = Rect::split_horizontally(&Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.5);

        assert_eq!(check_range(rects.0.x, 0.0, 0.001), true);
        assert_eq!(check_range(rects.0.y, 0.0, 0.001), true);
        assert_eq!(check_range(rects.0.width, 1024.0, 0.001), true);
        assert_eq!(check_range(rects.0.height, 512.0, 0.001), true);

        assert_eq!(check_range(rects.1.x, 0.0, 0.001), true);
        assert_eq!(check_range(rects.1.y, 512.0, 0.001), true);
        assert_eq!(check_range(rects.1.width, 1024.0, 0.001), true);
        assert_eq!(check_range(rects.1.height, 512.0, 0.001), true);
    }

    #[test]
    fn test_calc_rect_horz_25_per() {
        let rects = Rect::split_horizontally(&Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.25);

        assert_eq!(check_range(rects.0.x, 0.0, 0.001), true);
        assert_eq!(check_range(rects.0.y, 0.0, 0.001), true);
        assert_eq!(check_range(rects.0.width, 1024.0, 0.001), true);
        assert_eq!(check_range(rects.0.height, 256.0, 0.001), true);

        assert_eq!(check_range(rects.1.x, 0.0, 0.001), true);
        assert_eq!(check_range(rects.1.y, 256.0, 0.001), true);
        assert_eq!(check_range(rects.1.width, 1024.0, 0.001), true);
        assert_eq!(check_range(rects.1.height, 768.0, 0.001), true);
    }

    #[test]
    fn test_calc_rect_horz_25_per_2() {
        let rects = Rect::split_horizontally(&Rect::new(16.0, 32.0, 512.0, 1024.0), 0.25);

        assert_eq!(check_range(rects.0.x, 16.0, 0.001), true);
        assert_eq!(check_range(rects.0.y, 32.0, 0.001), true);
        assert_eq!(check_range(rects.0.width, 512.0, 0.001), true);
        assert_eq!(check_range(rects.0.height, 256.0, 0.001), true);

        assert_eq!(check_range(rects.1.x, 16.0, 0.001), true);
        assert_eq!(check_range(rects.1.y, 288.0, 0.001), true);
        assert_eq!(check_range(rects.1.width, 512.0, 0.001), true);
        assert_eq!(check_range(rects.1.height, 768.0, 0.001), true);
    }
//    TODO: update following tests for area around split calculation
//    #[test]
//    fn test_gen_horizontal_size() {
//        let border_size = 4.0;
//        let rect_in = Rect::new(10.0, 20.0, 30.0, 40.0);
//        let rect = rect_in.area_around_split(border_size);
//
//        assert_eq!(check_range(rect.x, rect_in.x, 0.001), true);
//        assert_eq!(check_range(rect.y, 60.0, 0.001), true);
//        assert_eq!(check_range(rect.width, rect_in.width, 0.001), true);
//        assert_eq!(check_range(rect.height, border_size, 0.001), true);
//    }
//
//    #[test]
//    fn test_gen_vertical_size() {
//        let border_size = 4.0;
//        let rect_in = Rect::new(10.0, 20.0, 30.0, 40.0);
//        let rect = Split::get_sizer_from_rect_vertical(rect_in, border_size);
//
//        assert_eq!(check_range(rect.x, 40.0, 0.001), true);
//        assert_eq!(check_range(rect.y, rect_in.y, 0.001), true);
//        assert_eq!(check_range(rect.width, border_size, 0.001), true);
//        assert_eq!(check_range(rect.height, rect_in.height, 0.001), true);
//    }
}