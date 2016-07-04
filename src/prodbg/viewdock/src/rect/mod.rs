mod serialize;

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Direction {
    Vertical,
    Horizontal,
}

impl Direction {
    pub fn opposite(&self) -> Direction {
        match *self {
            Direction::Vertical => Direction::Horizontal,
            Direction::Horizontal => Direction::Vertical,
        }
    }
}

#[derive(Debug, Clone)]
pub enum ShrinkSide {
    Lower,
    Higher,
    Both
}

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

    pub fn split(&self, direction: Direction, size: f32) -> [Rect; 2] {
        match direction {
            Direction::Horizontal => [
                Rect::new(self.x, self.y, self.width, size),
                Rect::new(self.x, self.y + self.height - size, self.width, size),
            ],
            Direction::Vertical => [
                Rect::new(self.x, self.y, size, self.height),
                Rect::new(self.x + self.width - size, self.y, size, self.height),
            ]
        }
    }

    pub fn dimension(&self, direction: Direction) -> f32 {
        match direction {
            Direction::Horizontal => self.width,
            Direction::Vertical => self.height,
        }
    }

    pub fn shifted(&self, direction: Direction, dist: f32) -> Rect {
        match direction {
            Direction::Horizontal => Rect::new(self.x + dist, self.y, self.width, self.height),
            Direction::Vertical => Rect::new(self.x, self.y + dist, self.width, self.height),
        }
    }

    pub fn shifted_clip(&self, direction: Direction, dist: f32) -> Rect {
        match direction {
            Direction::Horizontal => Rect::new(self.x + if dist>0.0 {dist} else {0.0}, self.y, self.width - dist.abs(), self.height),
            Direction::Vertical => Rect::new(self.x, self.y + if dist>0.0 {dist} else {0.0}, self.width, self.height - dist.abs()),
        }
    }

    pub fn shrinked(&self, direction: Direction, size: f32, side: ShrinkSide) -> Rect {
        let mut res = self.clone();
        {
            let (start, dimesion) = match direction {
                Direction::Horizontal => (&mut res.x, &mut res.width),
                Direction::Vertical => (&mut res.y, &mut res.height),
            };
            match side {
                ShrinkSide::Lower => {
                    *start += size;
                    *dimesion -= size;
                },
                ShrinkSide::Higher => {
                    *dimesion -= size;
                },
                ShrinkSide::Both => {
                    *start += size;
                    *dimesion -= size * 2.0;
                }
            }
        }
        return res;
    }

    pub fn area_around_splits(&self, direction: Direction, ratios: &[f32], width: f32) -> Vec<Rect> {
        match direction {
            Direction::Horizontal => {
                ratios.iter().map(|ratio| {
                    let y_start = self.y + self.height * ratio - width / 2.0;
                    return Rect::new(self.x, y_start, self.width, width);
                }).collect()
            },
            Direction::Vertical => {
                ratios.iter().map(|ratio| {
                    let x_start = self.x + self.width * ratio - width / 2.0;
                    return Rect::new(x_start, self.y, width, self.height);
                }).collect()
            },
        }
    }

    pub fn split_by_direction(&self, direction: Direction, ratios: &[f32]) -> Vec<Rect> {
        match direction {
            Direction::Horizontal => Rect::split_horizontally(self, ratios),
            Direction::Vertical => Rect::split_vertically(self, ratios),
        }
    }

    pub fn split_horizontally(rect: &Rect, ratios: &[f32]) -> Vec<Rect> {
        let mut prev_height = 0.0;
        return ratios.iter().map(|ratio| {
            let next_height = rect.height * ratio;
            let rect_height = next_height - prev_height;
            let res = Rect::new(rect.x, rect.y + prev_height, rect.width, rect_height);
            prev_height = next_height;
            return res;
        }).collect();
    }

    pub fn split_vertically(rect: &Rect, ratios: &[f32]) -> Vec<Rect> {
        let mut prev_width = 0.0;
        return ratios.iter().map(|ratio| {
            let next_width = rect.width * ratio;
            let rect_width = next_width - prev_width;
            let res = Rect::new(rect.x + prev_width, rect.y, rect_width, rect.height);
            prev_width = next_width;
            return res;
        }).collect();
    }
}

#[cfg(test)]
mod test {
    extern crate serde_json;
    use {Rect, Direction};

    fn check_range(inv: f32, value: f32, delta: f32) -> bool {
        (inv - value).abs() < delta
    }

//    #[test]
//    fn test_calc_rect_horz_half() {
//        let rects = Rect::split_horizontally(&Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.5);
//
//        assert_eq!(check_range(rects.0.x, 0.0, 0.001), true);
//        assert_eq!(check_range(rects.0.y, 0.0, 0.001), true);
//        assert_eq!(check_range(rects.0.width, 1024.0, 0.001), true);
//        assert_eq!(check_range(rects.0.height, 512.0, 0.001), true);
//
//        assert_eq!(check_range(rects.1.x, 0.0, 0.001), true);
//        assert_eq!(check_range(rects.1.y, 512.0, 0.001), true);
//        assert_eq!(check_range(rects.1.width, 1024.0, 0.001), true);
//        assert_eq!(check_range(rects.1.height, 512.0, 0.001), true);
//    }
//
//    #[test]
//    fn test_calc_rect_horz_25_per() {
//        let rects = Rect::split_horizontally(&Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.25);
//
//        assert_eq!(check_range(rects.0.x, 0.0, 0.001), true);
//        assert_eq!(check_range(rects.0.y, 0.0, 0.001), true);
//        assert_eq!(check_range(rects.0.width, 1024.0, 0.001), true);
//        assert_eq!(check_range(rects.0.height, 256.0, 0.001), true);
//
//        assert_eq!(check_range(rects.1.x, 0.0, 0.001), true);
//        assert_eq!(check_range(rects.1.y, 256.0, 0.001), true);
//        assert_eq!(check_range(rects.1.width, 1024.0, 0.001), true);
//        assert_eq!(check_range(rects.1.height, 768.0, 0.001), true);
//    }
//
//    #[test]
//    fn test_calc_rect_horz_25_per_2() {
//        let rects = Rect::split_horizontally(&Rect::new(16.0, 32.0, 512.0, 1024.0), 0.25);
//
//        assert_eq!(check_range(rects.0.x, 16.0, 0.001), true);
//        assert_eq!(check_range(rects.0.y, 32.0, 0.001), true);
//        assert_eq!(check_range(rects.0.width, 512.0, 0.001), true);
//        assert_eq!(check_range(rects.0.height, 256.0, 0.001), true);
//
//        assert_eq!(check_range(rects.1.x, 16.0, 0.001), true);
//        assert_eq!(check_range(rects.1.y, 288.0, 0.001), true);
//        assert_eq!(check_range(rects.1.width, 512.0, 0.001), true);
//        assert_eq!(check_range(rects.1.height, 768.0, 0.001), true);
//    }

    #[test]
    fn test_rect_serialization() {
        let rect_in = Rect { x: 1.0, y: 2.0, width: 1024.0, height: 768.0 };
        let serialized = serde_json::to_string(&rect_in).unwrap();
        let rect_out: Rect = serde_json::from_str(&serialized).unwrap();

        assert_eq!(rect_in.x as i32, rect_out.x as i32);
        assert_eq!(rect_in.y as i32, rect_out.y as i32);
        assert_eq!(rect_in.width as i32, rect_out.width as i32);
        assert_eq!(rect_in.height as i32, rect_out.height as i32);
    }

    #[test]
    fn test_direction_serialize() {
        let dir_in_0 = Direction::Horizontal;
        let dir_in_1 = Direction::Vertical;

        let s0 = serde_json::to_string(&dir_in_0).unwrap();
        let s1 = serde_json::to_string(&dir_in_1).unwrap();

        let dir_out_0: Direction = serde_json::from_str(&s0).unwrap();
        let dir_out_1: Direction = serde_json::from_str(&s1).unwrap();

        assert_eq!(dir_in_0, dir_out_0);
        assert_eq!(dir_in_1, dir_out_1);
    }
//
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

//    #[test]
//    fn test_inside_horizontal() {
//        let border_size = 4.0;
//        let rect = Rect::new(10.0, 20.0, 30.0, 40.0);
//        let rect_horz = Split::get_sizer_from_rect_horizontal(rect, border_size);
//
//        assert_eq!(Rect::is_inside((9.0, 61.0), rect_horz), false);
//        assert_eq!(Rect::is_inside((11.0, 61.0), rect_horz), true);
//    }

}