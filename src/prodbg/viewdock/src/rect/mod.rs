mod serialize;

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Direction {
    Vertical,
    Horizontal,
}

/// Defines direction for most of rectangle methods
impl Direction {
    /// Returns opposite direction for given
    pub fn opposite(&self) -> Direction {
        match *self {
            Direction::Vertical => Direction::Horizontal,
            Direction::Horizontal => Direction::Vertical,
        }
    }
}

/// Defines a way to shrink rectangle. See Rectangle::shrinked for more.
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

/// Simple axis-aligned rectangle defined by its left top corner, width and height
impl Rect {
    pub fn new(x: f32, y: f32, width: f32, height: f32) -> Rect {
        Rect {
            x: x,
            y: y,
            width: width,
            height: height
        }
    }

    /// Returns true if `pos` is inside of rectangle
    pub fn point_is_inside(&self, pos: (f32, f32)) -> bool {
        let (x, y) = pos;
        return
            self.x <= x &&
            self.x + self.width >= x &&
            self.y <= y &&
            self.y + self.height >= y;
    }

    /// Returns size of rectangle in given `direction`
    pub fn dimension(&self, direction: Direction) -> f32 {
        match direction {
            Direction::Horizontal => self.width,
            Direction::Vertical => self.height,
        }
    }

    /// Returns new rectangle shifted by `dist` in given `direction`
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

    /// Returns new rectangle shrinked by `size` in `direction`. If `side` is `ShrinkSide::Lower`,
    /// rectangle will shrinked from top or left. If `side` is `ShrinkSide::Higher`, rectangle
    /// will be shrinked from right or bottom. If `side` is `ShrinkSide::Both`, new rectangle will
    /// be shrinked from both sides of given `direction`.
    /// For example, if `origin` is `Rect {x: 0.0, y: 0.0, width: 10.0, height: 20.0}`, then
    /// `origin.shrinked(Direction::Horizontal, 5.0, ShrinkSide::Lower)` will be
    /// `Rect {x: 5.0, y: 0.0, width: 5.0, height: 20.0}`.
    ///
    /// `origin.shrinked(Direction::Vertical, 5.0, ShrinkSide::Higher)` will be
    /// `Rect {x: 0.0, y: 0.0, width: 10.0, height: 15.0}`.
    ///
    /// `origin.shrinked(Direction::Vertical, 2.0, ShrinkSide::Both)` will be
    /// `Rect {x: 0.0, y: 2.0, width: 10.0, height: 16.0}`.
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

    /// Returns rectangles for each value in `ratios`. Each `ratio` is relative to width or height.
    /// `direction` shows direction of splits.
    /// For example, if `origin` is `Rect {x: 0.0, y: 0.0, width: 10.0, height: 20.0}`, then
    /// `origin.area_around_splits(Direction::Horizontal, &[0.4, 0.8], 2.0)` will be vector of
    /// `Rect {x: 0.0, y: 7.0, width: 10.0, height: 2.0}` and
    /// `Rect {x: 0.0, y: 15.0, width: 10.0, height: 2.0}`
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

    /// Splits rectangle into several smaller rectangles. `ratios` contains values relative to
    /// corresponding dimension (i.e. 0.5 is half of width or height).
    pub fn split_by_direction(&self, direction: Direction, ratios: &[f32]) -> Vec<Rect> {
        match direction {
            Direction::Horizontal => Rect::split_horizontally(self, ratios),
            Direction::Vertical => Rect::split_vertically(self, ratios),
        }
    }

    fn split_horizontally(&self, ratios: &[f32]) -> Vec<Rect> {
        let mut prev_height = 0.0;
        return ratios.iter().map(|ratio| {
            let next_height = self.height * ratio;
            let rect_height = next_height - prev_height;
            let res = Rect::new(self.x, self.y + prev_height, self.width, rect_height);
            prev_height = next_height;
            return res;
        }).collect();
    }

    fn split_vertically(&self, ratios: &[f32]) -> Vec<Rect> {
        let mut prev_width = 0.0;
        return ratios.iter().map(|ratio| {
            let next_width = self.width * ratio;
            let rect_width = next_width - prev_width;
            let res = Rect::new(self.x + prev_width, self.y, rect_width, self.height);
            prev_width = next_width;
            return res;
        }).collect();
    }
}

#[cfg(test)]
mod test {
    extern crate serde_json;
    use super::{Rect, Direction, ShrinkSide};
    use test_helper::rects_are_equal;

    #[test]
    fn test_point_is_inside() {
        let rect = Rect::new(0.0, 0.0, 10.0, 10.0);
        assert!(rect.point_is_inside((5.0, 5.0)));
        assert!(rect.point_is_inside((1.0, 5.0)));
        assert!(!rect.point_is_inside((-1.0, 5.0)));
        assert!(!rect.point_is_inside((11.0, 5.0)));
        assert!(!rect.point_is_inside((1.0, -5.0)));
        assert!(!rect.point_is_inside((1.0, 55.0)));
    }

    #[test]
    fn test_shifted() {
        let rect = Rect::new(0.0, 0.0, 10.0, 10.0);
        assert!(rects_are_equal(Rect::new(2.0, 0.0, 10.0, 10.0), rect.shifted(Direction::Horizontal, 2.0)));
        assert!(rects_are_equal(Rect::new(0.0, -5.0, 10.0, 10.0), rect.shifted(Direction::Vertical, -5.0)));
    }

    #[test]
    fn test_shrinked() {
        let rect = Rect::new(0.0, 0.0, 10.0, 20.0);
        assert!(rects_are_equal(
            Rect::new(5.0, 0.0, 5.0, 20.0),
            rect.shrinked(Direction::Horizontal, 5.0, ShrinkSide::Lower)
        ));
        assert!(rects_are_equal(
            Rect::new(0.0, 0.0, 10.0, 15.0),
            rect.shrinked(Direction::Vertical, 5.0, ShrinkSide::Higher)
        ));
        assert!(rects_are_equal(
            Rect::new(0.0, 2.0, 10.0, 16.0),
            rect.shrinked(Direction::Vertical, 2.0, ShrinkSide::Both)
        ));
    }

    #[test]
    fn test_area_around_splits() {
        let rect = Rect::new(0.0, 0.0, 10.0, 20.0);
        let res = rect.area_around_splits(Direction::Horizontal, &[0.4, 0.8], 2.0);
        assert_eq!(2, res.len());
        assert!(rects_are_equal(Rect::new(0.0, 7.0, 10.0, 2.0), res[0]));
        assert!(rects_are_equal(Rect::new(0.0, 15.0, 10.0, 2.0), res[1]));
        let res2 = rect.area_around_splits(Direction::Vertical, &[0.25, 0.5, 1.0], 2.0);
        assert_eq!(3, res2.len());
        assert!(rects_are_equal(Rect::new(1.5, 0.0, 2.0, 20.0), res2[0]));
        assert!(rects_are_equal(Rect::new(4.0, 0.0, 2.0, 20.0), res2[1]));
        assert!(rects_are_equal(Rect::new(9.0, 0.0, 2.0, 20.0), res2[2]));
    }

    #[test]
    fn split_by_direction() {
        let rect = Rect::new(0.0, 0.0, 1024.0, 1024.0);
        let res = rect.split_by_direction(Direction::Horizontal, &[0.25, 0.5, 1.0]);
        assert_eq!(3, res.len());
        assert!(rects_are_equal(Rect::new(0.0, 0.0, 1024.0, 256.0), res[0]));
        assert!(rects_are_equal(Rect::new(0.0, 256.0, 1024.0, 256.0), res[1]));
        assert!(rects_are_equal(Rect::new(0.0, 512.0, 1024.0, 512.0), res[2]));
        let res2 = rect.split_by_direction(Direction::Vertical, &[0.25, 0.5, 1.0]);
        assert_eq!(3, res.len());
        assert!(rects_are_equal(Rect::new(0.0, 0.0, 256.0, 1024.0), res2[0]));
        assert!(rects_are_equal(Rect::new(256.0, 0.0, 256.0, 1024.0), res2[1]));
        assert!(rects_are_equal(Rect::new(512.0, 0.0, 512.0, 1024.0), res2[2]));
    }

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
}