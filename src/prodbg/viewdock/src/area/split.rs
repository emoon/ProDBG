use serde;
use super::Area;
use rect::{Direction, Rect, ShrinkSide};

/// Handle to a split
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct SplitHandle(pub u64);

/// Handle to a sizer (area between two children). Identified by `SplitHandle` and index,
/// `Direction` is to simplify further use of `SizerPos`. Last member is current ratio.
#[derive(Debug)]
pub struct SizerPos(pub SplitHandle, pub usize, pub Direction, pub f32);

/// `Split` gives slice of its area to each child. It intentionally does not contain any tree
/// traversal methods. For tree traversal see `Area`.
#[derive(Debug, Clone)]
pub struct Split {
    /// Children
    pub children: Vec<Area>,
    /// Higher (right of bottom) border of each child. Last element should always be 1.0. For
    /// example, if `ratios` is `[0.25, 0.7, 1.0]`, then area `0 - 0.25` will be occupied by first
    /// child, `0.25 - 0.7` by second and so on.
    pub ratios: Vec<f32>,
    /// Direction of the split. For example, `Direction::Horizontal` means `rect.height` will be
    /// split by horizontal lines.
    pub direction: Direction,
    /// Handle of the split.
    pub handle: SplitHandle,
    /// Area occupied by this split.
    pub rect: Rect,
}

/// Width of area between splits. Children's rects will be shrinked a bit to fit sizers (empty area
/// currently) between them;
const SIZER_WIDTH: f32 = 4.0;

impl Split {
    /// Creates new `Split` from two children
    pub fn from_two(direction: Direction,
                    ratio: f32,
                    handle: SplitHandle,
                    rect: Rect,
                    first: Area,
                    second: Area)
                    -> Split {
        let mut res = Split {
            children: vec![first, second],
            ratios: vec![ratio, 1.0],
            direction: direction,
            handle: handle,
            rect: rect,
        };
        res.update_children_sizes();
        return res;
    }

    // Recalculates children sizes and pushes changes further
    fn update_children_sizes(&mut self) {
        let rects = self.rect.split_by_direction(self.direction, &self.ratios);
        let last_index = self.children.len() - 1;
        for (index, (child, rect)) in self.children.iter_mut().zip(rects.iter()).enumerate() {
            let side = match index {
                0 => ShrinkSide::Higher,
                x if x == last_index => ShrinkSide::Lower,
                _ => ShrinkSide::Both,
            };
            child.update_rect(rect.shrinked(self.direction.opposite(), SIZER_WIDTH / 2.0, side));
        }
    }

    /// Updates area of this split and all children
    pub fn update_rect(&mut self, rect: Rect) {
        self.rect = rect;
        self.update_children_sizes();
    }

    /// Returns reference to child which area contains `pos`
    pub fn get_child_at_pos(&self, pos: (f32, f32)) -> Option<&Area> {
        self.children
            .iter()
            .find(|child| child.get_rect().point_is_inside(pos))
    }

    /// Returns handle to a sizer
    pub fn get_sizer_at_pos(&self, pos: (f32, f32)) -> Option<SizerPos> {
        if !self.rect.point_is_inside(pos) {
            return None;
        }
        let sizer_rects = self.rect
            .area_around_splits(self.direction, &self.ratios[0..self.ratios.len() - 1], 8.0);
        return sizer_rects.iter()
            .enumerate()
            .find(|&(_, rect)| rect.point_is_inside(pos))
            .map(|(i, _)| SizerPos(self.handle, i, self.direction, self.ratios[i]));
    }

    // recalculates absolute delta (in pixels) into relative value (to increase/decrease `ratios`)
    fn map_rect_to_delta(&self, delta: (f32, f32)) -> f32 {
        match self.direction {
            Direction::Vertical => -delta.0 / self.rect.width,
            Direction::Horizontal => -delta.1 / self.rect.height,
        }
    }

    /// Changes ratio at `index`. Does not allow distance between neighbouring ratios less then
    /// `0.05`.
    pub fn change_ratio(&mut self, index: usize, origin: f32, delta: (f32, f32)) {
        let scale = Self::map_rect_to_delta(self, delta);
        let mut res = origin + scale;

        let min = if index == 0 {
            0.05
        } else {
            self.ratios[index - 1] + 0.05
        };
        let max = if index == self.ratios.len() - 1 {
            0.95
        } else {
            self.ratios[index + 1] - 0.05
        };

        if res < min {
            res = min;
        }

        if res > max {
            res = max;
        }

        self.ratios[index] = res;
        self.update_children_sizes();
    }

    /// Replace child at `index` by `new_child` and replaced child.
    pub fn replace_child(&mut self, index: usize, new_child: Area) -> Area {
        self.children.push(new_child);
        let res = self.children.swap_remove(index);
        self.update_children_sizes();
        return res;
    }

    /// Insert `child` at given `index`. If `index` is more then `children.len()`, pushes child
    /// to the end.
    pub fn insert_child(&mut self, index: usize, child: Area) {
        if index > self.children.len() - 1 {
            return self.push_child(child);
        }
        let existing_ratio = self.ratios[index];
        let previous_ratio = match index {
            0 => 0.0,
            _ => self.ratios[index - 1],
        };
        let diff = existing_ratio - previous_ratio;
        self.children.insert(index, child);
        self.ratios.insert(index, existing_ratio - diff / 2.0);
        self.update_children_sizes();
    }

    /// Pushes `child` to the end of `children`.
    pub fn push_child(&mut self, child: Area) {
        let last_index = self.children.len() - 1;
        self.children.push(child);
        let diff = 1.0 - self.ratios[last_index - 1];
        self.ratios[last_index] = 1.0 - diff / 2.0;
        self.ratios.push(1.0);
    }

    /// Removes child at given `index`
    pub fn remove_child(&mut self, index: usize) {
        self.children.remove(index);
        self.ratios.remove(index);
        if index == self.ratios.len() {
            self.ratios[index - 1] = 1.0;
        }
        self.update_children_sizes();
    }

    /// Replaces child at `index` with `children`. Area occupied by child at `index` is split
    /// between `children` according to their rects.
    pub fn replace_child_with_children(&mut self, index: usize, children: &[Area]) {
        self.children.remove(index);
        let mut dimensions: Vec<f32> = children.iter()
            .map(|child| match self.direction {
                Direction::Horizontal => child.get_rect().height,
                Direction::Vertical => child.get_rect().width,
            })
            .collect();
        let dimension_sum = dimensions.iter().fold(0.0, |sum, dimension| sum + dimension);
        let mut prev = 0.0;
        for dimension in dimensions.iter_mut() {
            prev += *dimension / dimension_sum;
            *dimension = prev;
        }
        for child in children.iter().rev() {
            self.children.insert(index, child.clone());
        }

        let old_ratio = self.ratios.remove(index);
        let previous_ratio = match index {
            0 => 0.0,
            _ => self.ratios[index - 1],
        };
        let diff = old_ratio - previous_ratio;
        for pos in dimensions.iter().rev() {
            self.ratios.insert(index, previous_ratio + pos * diff);
        }
        self.update_children_sizes();
    }
}

// Serialization
gen_handle!("SplitHandle", SplitHandle, SplitHandleVisitor);
gen_struct_code!(Split, children, ratios, direction, handle; rect => Rect::default());

#[cfg(test)]
mod test {
    extern crate serde_json;

    use super::{Split, SplitHandle};
    use area::Area;
    use area::container::Container;
    use DockHandle;
    use rect::{Direction, Rect};

    #[test]
    fn test_splithandle_serialize() {
        let handle_in = SplitHandle(0x4422);
        let serialized = serde_json::to_string(&handle_in).unwrap();
        let handle_out: SplitHandle = serde_json::from_str(&serialized).unwrap();

        assert_eq!(handle_in, handle_out);
    }

    #[test]
    fn test_split_serialize() {
        let split_in =
            Split::from_two(Direction::Horizontal,
                            0.7,
                            SplitHandle(513),
                            Rect::new(17.0, 15.0, 100.0, 159.0),
                            Area::Container(Container::new(DockHandle(14), Rect::default())),
                            Area::Container(Container::new(DockHandle(15), Rect::default())));

        let serialized = serde_json::to_string(&split_in).unwrap();
        let split_out: Split = serde_json::from_str(&serialized).unwrap();

        assert_eq!(split_in.children.len(), split_out.children.len());
        assert_eq!(split_in.ratios.len(), split_out.ratios.len());
        assert_eq!(split_in.direction, split_out.direction);
        assert_eq!(split_in.handle, split_out.handle);

        // expect that rect is not serialized and set to zero
        assert_eq!(split_out.rect.x as i32, 0);
        assert_eq!(split_out.rect.y as i32, 0);
        assert_eq!(split_out.rect.width as i32, 0);
        assert_eq!(split_out.rect.height as i32, 0);
    }
}
