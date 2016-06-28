use rect::{Rect, Direction};
use dock::{DockHandle, Dock};

/// Handle to a split
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct SplitHandle(pub u64);

/// Given rectangle area is split in two parts.
#[derive(Debug, Clone)]
pub struct Split {
    /// Children
    pub children: Vec<Area>,
    /// Right (or bottom) border of each child. Last should always be 1.
    pub ratios: Vec<f32>,
    /// Direction of the split
    pub direction: Direction,
    /// Handle of the split
    pub handle: SplitHandle,
    /// Area occupied by this split
    pub rect: Rect,
}

impl Split {
    pub fn from_two(direction: Direction, ratio: f32, handle: SplitHandle, rect: Rect, first: Area, second: Area) -> Split {
        let mut res = Split {
            children: vec!(first, second),
            ratios: vec!(ratio, 1.0),
            direction: direction,
            handle: handle,
            rect: rect,
        };
        res.update_children_sizes();
        return res;
    }

    fn update_children_sizes(&mut self) {
        let rects = self.rect.split_by_direction(self.direction, &self.ratios);
        for (child, rect) in self.children.iter_mut().zip(rects.iter()) {
            child.update_rect(*rect);
        }
    }

    pub fn update_rect(&mut self, rect: Rect) {
        self.rect = rect;
        self.update_children_sizes();
    }

    fn get_child_at_pos(&self, pos: (f32, f32)) -> Option<&Area> {
        self.children.iter()
            .find(|child| child.get_rect().point_is_inside(pos))
    }

    pub fn get_drag_target_at_pos(&self, pos: (f32, f32)) -> Option<DragTarget> {
        let sizer_rects = self.rect.area_around_splits(self.direction, &self.ratios[0..self.ratios.len() - 1], 8.0);
        return sizer_rects.iter().enumerate()
            .find(|&(_, rect)| rect.point_is_inside(pos))
            .map(|(i, _)| DragTarget::SplitSizer(self.handle, i, self.direction))
            .or_else(|| {
                self.get_child_at_pos(pos)
                    .and_then(|child| child.get_drag_target_at_pos(pos))
            });
    }

    pub fn get_drop_target_at_pos(&self, pos: (f32, f32)) -> Option<DropTarget> {
        self.get_child_at_pos(pos)
            .and_then(|child| child.get_drop_target_at_pos(pos))
    }

    pub fn map_rect_to_delta(&self, delta: (f32, f32)) -> f32 {
        match self.direction {
            Direction::Vertical => -delta.0 / self.rect.width,
            Direction::Horizontal => -delta.1 / self.rect.height,
        }
    }

    pub fn change_ratio(&mut self, index: usize, delta: (f32, f32)) {
        let scale = Self::map_rect_to_delta(self, delta);
        let mut res = self.ratios[index] + scale;

        if res < 0.01 {
            res = 0.01;
        }

        if res > 0.99 {
            res = 0.99;
        }

        self.ratios[index] = res;
        self.update_children_sizes();
    }

    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        self.children.iter()
            .find(|child| child.get_rect().point_is_inside(pos))
            .and_then(|child| child.get_dock_handle_at_pos(pos))
    }

    pub fn replace_child(&mut self, index: usize, new_child: Area) -> Area {
        self.children.push(new_child);
        let res = self.children.swap_remove(index);
        self.update_children_sizes();
        return res;
    }

    pub fn append_child(&mut self, index: usize, child: Area) {
        let existing_ratio = self.ratios[index];
        let previous_ratio = match index {
            0 => 0.0,
            _ => self.ratios[index - 1]
        };
        let diff = existing_ratio - previous_ratio;
        self.children.insert(index, child);
        self.ratios.insert(index, existing_ratio - diff / 2.0);
        self.update_children_sizes();
    }

    pub fn remove_child(&mut self, index: usize) {
        self.children.remove(index);
        self.ratios.remove(index);
        if index == self.ratios.len() {
            self.ratios[index - 1] = 1.0;
        }
        self.update_children_sizes();
    }
}


/// Holds a list of available docks
#[derive(Debug, Clone)]
pub struct Container {
    /// Docks this container. The reason of supporting several docks here is that this can be used
    /// to implement tabs but only one dock should be visible at a time
    pub docks: Vec<Dock>,
    pub rect: Rect,
}

/// Area could be occupied either by Container or by Split
#[derive(Debug, Clone)]
pub enum Area {
    Container(Container),
    Split(Split),
}

impl Area {
    /// Finds Area::Split by its handle
    pub fn find_split_by_handle(&mut self, handle: SplitHandle) -> Option<&mut Split> {
        match self {
            &mut Area::Container(_) => None,
            &mut Area::Split(ref mut s) => if s.handle == handle {
                Some(s)
            } else {
                s.children.iter_mut()
                    .map(|child| child.find_split_by_handle(handle))
                    .find(|c| {c.is_some() })
                    .map(|res| res.unwrap())
            }
        }
    }

    pub fn update_rect(&mut self, rect: Rect) {
        match self {
            &mut Area::Container(ref mut c) => c.rect = rect,
            &mut Area::Split(ref mut s) => s.update_rect(rect),
        }
    }

    pub fn get_rect(&self) -> Rect {
        match self {
            &Area::Container(ref c) => c.rect.clone(),
            &Area::Split(ref s) => s.rect.clone(),
        }
    }

    /// Finds Container with supplied DockHandle
    pub fn find_container_by_dock_handle(&self, handle: DockHandle) -> Option<&Container> {
        match self {
            &Area::Container(ref c) => c.find_dock(handle).map(|_| c),
            &Area::Split(ref s) => s.children.iter()
                .map(|child| child.find_container_by_dock_handle(handle))
                .find(|c| {c.is_some() })
                .map(|res| res.unwrap())
        }
    }

    /// Finds Area::Split which contains Container with supplied DockHandle
    pub fn find_split_by_dock_handle(&mut self, handle: DockHandle) -> Option<(&mut Split, usize)> {
        if let &mut Area::Split(ref mut s) = self {
            let found_child = s.children.iter_mut().position(|child| {
                match child {
                    &mut Area::Container(ref c) => c.find_dock(handle).is_some(),
                    _ => false,
                }
            });
            return match found_child {
                Some(position) => Some((s, position)),
                None => s.children.iter_mut()
                    .map(|child| child.find_split_by_dock_handle(handle))
                    .find(|res| res.is_some())
                    .map(|res| res.unwrap())
            }
        }
        return None;
    }

    pub fn find_parent_split_by_split_handle(&mut self, handle: SplitHandle) -> Option<(&mut Split, usize)> {
        if let &mut Area::Split(ref mut s) = self {
            let found_child = s.children.iter_mut().position(|child| {
                match child {
                    &mut Area::Split(ref s) => s.handle == handle,
                    _ => false,
                }
            });
            return match found_child {
                Some(position) => Some((s, position)),
                None => s.children.iter_mut()
                    .map(|child| child.find_parent_split_by_split_handle(handle))
                    .find(|res| res.is_some())
                    .map(|res| res.unwrap())
            }
        }
        return None;
    }

    pub fn get_drag_target_at_pos(&self, pos: (f32, f32)) -> Option<DragTarget> {
        match self {
            &Area::Split(ref s) => s.get_drag_target_at_pos(pos),
            &Area::Container(ref c) => c.get_drag_target_at_pos(pos),
        }
    }

    pub fn get_drop_target_at_pos(&self, pos: (f32, f32)) -> Option<DropTarget> {
        match self {
            &Area::Split(ref s) => s.get_drop_target_at_pos(pos),
            &Area::Container(ref c) => c.get_drop_target_at_pos(pos),
        }
    }

    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        return match self {
            &Area::Container(ref c) => c.get_dock_handle_at_pos(pos),
            &Area::Split(ref c) => c.get_dock_handle_at_pos(pos),
        };
    }
}


impl Container {
    pub fn new(dock: Dock, rect: Rect) -> Container {
        Container {
            docks: vec!(dock),
            rect: rect,
        }
    }

    pub fn find_dock(&self, handle: DockHandle) -> Option<&Dock> {
        self.docks.iter().find(|&dock| dock.handle == handle)
    }

    pub fn remove_handle(&mut self, handle: DockHandle) -> bool {
        for i in (0..self.docks.len()).rev() {
            if self.docks[i].handle == handle {
                //println!("Removed dock handle {:?}", handle);
                self.docks.swap_remove(i);
                return true;
            }
        }

        false
    }

    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        if self.rect.point_is_inside(pos) {
            self.docks.first().map(|dock| dock.handle)
        } else {
            None
        }
    }

    pub fn get_header_rect(&self) -> Rect {
        Rect::new(self.rect.x, self.rect.y, self.rect.width - 30.0, 30.0)
    }

    pub fn get_drag_target_at_pos(&self, pos: (f32, f32)) -> Option<DragTarget> {
        return if self.get_header_rect().point_is_inside(pos) {
            Some(DragTarget::Dock(self.docks.first().unwrap().handle))
        } else {
            None
        }
    }

    pub fn get_drop_target_at_pos(&self, pos: (f32, f32)) -> Option<DropTarget> {
        return if self.get_header_rect().point_is_inside(pos) {
            Some(DropTarget::Dock(self.docks.first().unwrap().handle))
        } else {
            None
        }
    }
}

#[derive(Debug)]
pub enum DragTarget {
    SplitSizer(SplitHandle, usize, Direction),
    Dock(DockHandle)
}

#[derive(Debug)]
pub enum DropTarget {
    Dock(DockHandle)
}