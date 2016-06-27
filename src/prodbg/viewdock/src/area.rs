use rect::{Rect, Direction};
use dock::{DockHandle, Dock};

/// Handle to a split
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct SplitHandle(pub u64);

/// Given rectangle area is split in two parts.
#[derive(Debug, Clone)]
pub struct Split {
    /// left/top part of area
    pub first: Box<Area>,
    /// right/bottom part of area
    pub second: Box<Area>,
    /// ratio value of how much of each side that is visible. 1.0 = first fully visible
    pub ratio: f32,
    /// Direction of the split
    pub direction: Direction,
    /// Handle of the split
    pub handle: SplitHandle,
    /// Area occupied by this split
    pub rect: Rect,
}

impl Split {
    pub fn new(direction: Direction, ratio: f32, handle: SplitHandle, first: Area, second: Area, rect: Rect) -> Split {
        let mut res = Split {
            first: Box::new(first),
            second: Box::new(second),
            ratio: ratio,
            direction: direction,
            handle: handle,
            rect: rect,
        };
        res.update_rect(rect);
        return res;
    }

    fn update_child_sizes(&mut self) {
        let (first_rect, second_rect) = self.rect.split_by_direction(self.direction, self.ratio);
        self.first.update_rect(first_rect);
        self.second.update_rect(second_rect);
    }

    pub fn update_rect(&mut self, rect: Rect) {
        self.rect = rect;
        self.update_child_sizes();
    }

    fn get_child_at_pos(&self, pos: (f32, f32)) -> Option<&Box<Area>> {
        let (first_rect, second_rect) = self.rect.split_by_direction(self.direction, self.ratio);
        return if first_rect.point_is_inside(pos) {
            Some(&self.first)
        } else if second_rect.point_is_inside(pos) {
            Some(&self.second)
        } else {
            None
        }
    }

    pub fn get_drag_target_at_pos(&self, pos: (f32, f32)) -> Option<DragTarget> {
        let sizer_rect = self.rect.area_around_split(self.direction, self.ratio, 8.0);
        if sizer_rect.point_is_inside(pos) {
            return Some(DragTarget::SplitSizer(self.handle, self.direction));
        }
        return self.get_child_at_pos(pos)
            .and_then(|child| child.get_drag_target_at_pos(pos));
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

    pub fn change_ratio(&mut self, delta: (f32, f32)) {
        let scale = Self::map_rect_to_delta(self, delta);

        self.ratio += scale;

        if self.ratio < 0.01 {
            self.ratio = 0.01;
        }

        if self.ratio > 0.99 {
            self.ratio = 0.99;
        }
        self.update_child_sizes();
    }

//    pub fn dump_info(&self, level: i32) {
//        unimplemented!();
//        let l_count = self.left_docks.docks.len();
//        let r_count = self.right_docks.docks.len();
//
//        Self::pad(level);
//
//        println!("Split - Dir {:?} - handle {} - ratio {} count ({}, {}) - left ({:?}) right ({:?}) - rect {} {} - {} {}",
//        self.direction, self.handle.0, self.ratio, l_count, r_count,
//        self.left, self.right, self.rect.x, self.rect.y, self.rect.width, self.rect.height);
//
//        for d in &self.left_docks.docks {
//            Self::pad(level);
//            println!("left dock handle - {}", d.handle.0);
//        }
//
//        for d in &self.right_docks.docks {
//            Self::pad(level);
//            println!("right dock handle - {}", d.handle.0);
//        }
//    }

    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        let (first_rect, _) = self.rect.split_by_direction(self.direction, self.ratio);
        let res = if first_rect.point_is_inside(pos) {
            &self.first
        } else {
            &self.second
        };
        return res.get_dock_handle_at_pos(pos);
    }

    pub fn change_first_child(&mut self, child: Box<Area>) {
        self.first = child;
        self.update_child_sizes();
    }

    pub fn change_second_child(&mut self, child: Box<Area>) {
        self.second = child;
        self.update_child_sizes();
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

pub enum SplitSearchResult<T> {
    FirstChild(T),
    SecondChild(T),
    None,
}

impl<T> SplitSearchResult<T> {
    pub fn is_some(&self) -> bool {
        match self {
            &SplitSearchResult::None => false,
            _ => true,
        }
    }
}

impl Area {
    /// Finds Area::Container with supplied DockHandle
    pub fn find_container_holder_by_dock_handle(&mut self, handle: DockHandle) -> Option<&mut Area> {
        let mut is_self = false;
        if let &mut Area::Container(ref c) = self {
            is_self = c.find_dock(handle).is_some();
        }
        if is_self {
            return Some(self);
        }
        if let &mut Area::Split(ref mut s) = self {
            let res = s.first.find_container_holder_by_dock_handle(handle);
            if res.is_some() {
                return res;
            }
            return s.second.find_container_holder_by_dock_handle(handle);
        }
        return None;
    }

    /// Finds Area::Split by its handle
    pub fn find_split_by_handle(&mut self, handle: SplitHandle) -> Option<&mut Split> {
        match self {
            &mut Area::Container(_) => None,
            &mut Area::Split(ref mut s) => if s.handle == handle {
                Some(s)
            } else {
                let first_res = s.first.find_split_by_handle(handle);
                if first_res.is_some() {
                    first_res
                } else {
                    s.second.find_split_by_handle(handle)
                }
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
            &Area::Split(ref s) => s.first
                .find_container_by_dock_handle(handle)
                .or_else(|| s.second.find_container_by_dock_handle(handle))
        }
    }

    pub fn find_dock_by_handle_mut(&mut self, handle: DockHandle) -> Option<&mut Dock> {
        match self {
            &mut Area::Container(ref mut c) => c.find_dock_mut(handle),
            &mut Area::Split(ref mut s) => {
                let res = s.first.find_dock_by_handle_mut(handle);
                if res.is_some() {
                    res
                } else {
                    s.second.find_dock_by_handle_mut(handle)
                }
            }
        }
    }

    /// Finds Area::Split which contains Container with supplied DockHandle
    pub fn find_split_by_dock_handle(&mut self, handle: DockHandle) -> SplitSearchResult<&mut Area> {
        let mut found_child = 0;
        if let &mut Area::Split(ref mut s) = self {
            if let Area::Container (ref c) = *s.first {
                if c.find_dock(handle).is_some() {
                    found_child = 1;
                };
            }
            if let Area::Container (ref c) = *s.second {
                if c.find_dock(handle).is_some() {
                    found_child = 2;
                }
            }
        }
        match found_child {
            1 => {return SplitSearchResult::FirstChild(self);},
            2 => {return SplitSearchResult::SecondChild(self);},
            _ => {
                if let &mut Area::Split(ref mut s) = self {
                    if let Area::Split(_) = *s.first {
                        let res = s.first.find_split_by_dock_handle(handle);
                        if res.is_some() {
                            return res;
                        }
                    }
                    if let Area::Split(_) = *s.second {
                        let res = s.second.find_split_by_dock_handle(handle);
                        if res.is_some() {
                            return res;
                        }
                    }
                }
                return SplitSearchResult::None;
            }
        }
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

    pub fn find_dock_mut(&mut self, handle: DockHandle) -> Option<&mut Dock> {
        self.docks.iter_mut().find(|dock| dock.handle == handle)
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

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        for dock in &self.docks {
            if dock.handle == handle {
                return Some(self.rect);
            }
        }

        None
    }

    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        if self.rect.point_is_inside(pos) {
            self.docks.first().map(|dock| dock.handle)
        } else {
            None
        }
    }

    /*
    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        self.docks.iter().find(|&dock| dock.handle == handle).map(|dock| dock.rect)
    }
    */

    pub fn is_inside(&self, pos: (f32, f32)) -> Option<(DockHandle, f32)> {
        for dock in &self.docks {
            if self.rect.point_is_inside(pos) {
                return Some((dock.handle, dock.rect.width * dock.rect.height));
            }
        }

        None
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
    SplitSizer(SplitHandle, Direction),
    Dock(DockHandle)
}

#[derive(Debug)]
pub enum DropTarget {
    Dock(DockHandle)
}