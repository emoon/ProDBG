use serde;
use super::Area;
use rect::{Direction, Rect, ShrinkSide};

/// Handle to a split
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct SplitHandle(pub u64);

/// Handle to a sizer (area between two children). Identified by `SplitHandle` and index,
/// `Direction` is to simplify further use of `SizerPos`.
#[derive(Debug)]
pub struct SizerPos(pub SplitHandle, pub usize, pub Direction);

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
            .map(|(i, _)| SizerPos(self.handle, i, self.direction));
    }

    // recalculates absolute delta (in pixels) into relative ratio
    fn map_pos_to_ratio(&self, pos: (f32, f32)) -> f32 {
        let (start, dimension) = self.rect.segment(self.direction.opposite());
        let pos = match self.direction {
            Direction::Vertical => pos.0,
            Direction::Horizontal => pos.1,
        };
        (pos - start) / dimension
    }

    /// Sets ratio at `index`. Does not allow distance between neighbouring ratios less then
    /// `0.05` (ratio will be clamped).
    pub fn set_ratio(&mut self, index: usize, mut ratio: f32) {
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

        if ratio < min {
            ratio = min;
        }

        if ratio > max {
            ratio = max;
        }

        self.ratios[index] = ratio;
        self.update_children_sizes();
    }

    /// Changes sizer at `index` to be at `pos` (only one coordinate is used). Useful when
    /// sizer needs to follow mouse.
    pub fn set_sizer_at(&mut self, index: usize, pos: (f32, f32)) {
        let ratio = self.map_pos_to_ratio(pos);
        self.set_ratio(index, ratio);
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
gen_newtype_code!(SplitHandle);
gen_struct_code!(Split, children, ratios, direction, handle; rect => Rect::default());
