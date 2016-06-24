//! View dock is a generic windowing docking library written in [Rust](https://www.rust-lang.org). The purpose of this library
//! is to allow windows (views from now on) to connect to each other in flexible grid. The main
//! purpose of this is to be used inside graphical user interfaces (GUI) as it allows users to
//! arrange their views in an intuitive way.
//!
//! Here is an high-level overview of the data layout:
//!
//! ![](https://dl.dropboxusercontent.com/u/5205843/prodbg_doc/viewdock/overview.png)
//!
//! Viewdock in inspired by https://i3wm.org/docs/userguide.html and while the data structures
//! isn't identical the idea is the same
//!
//! Here is an example in i3wm:
//!
//! ![](https://i3wm.org/docs/tree-shot1.png)
//!
//! Tree layout
//!
//! ![](https://i3wm.org/docs/tree-layout1.png)
//!
//! Each split has percentage value that decides how much of each Container that is visible and can
//! have exact 2 other splits. If a new split is created in an Split the old container will be
//! moved down a level.
//!

extern crate serde_json;
mod error;
// mod serialize;
pub use self::error::Error;
// use std::io::{Write, Read};
// use std::fs::File;
//use std::io;

/// Handle to a dock
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct DockHandle(pub u64);

/// Handle to a split
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct SplitHandle(pub u64);

/// Data structure for rectangles
#[derive(Debug, Default, Clone, Copy)]
pub struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

/// Holds information about the plugin view, data and handle
#[derive(Debug, Clone)]
pub struct Dock {
    pub handle: DockHandle,
    pub plugin_name: String,
    pub plugin_data: Option<Vec<String>>,
    pub rect: Rect
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
            is_self = c.find_handle(handle).is_some();
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
            &Area::Container(ref c) => c.find_handle(handle).map(|_| c),
            &Area::Split(ref s) => s.first
                .find_container_by_dock_handle(handle)
                .or_else(|| s.second.find_container_by_dock_handle(handle))
        }
    }

    /// Finds Area::Split which contains Container with supplied DockHandle
    pub fn find_split_by_dock_handle(&mut self, handle: DockHandle) -> SplitSearchResult<&mut Area> {
        let mut found_child = 0;
        if let &mut Area::Split(ref mut s) = self {
            if let Area::Container (ref c) = *s.first {
                if c.find_handle(handle).is_some() {
                    found_child = 1;
                };
            }
            if let Area::Container (ref c) = *s.second {
                if c.find_handle(handle).is_some() {
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
}

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

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Direction {
    Vertical,
    Horizontal,
}

/// Top level structure that holds an array of all the splits and the rect size of of the full
/// layout. This size is then propagated downwards and recalculated depending on the tree
/// structure.
#[derive(Debug)]
pub struct Workspace {
    pub root_area: Option<Area>,
    rect: Rect,
    /// border size of the windows (in pixels)
    pub window_border: f32,
    handle_counter: SplitHandle,
}


pub type ResultView<T> = std::result::Result<T, Error>;

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

impl Dock {
    fn new(dock_handle: DockHandle) -> Dock {
        Dock {
            handle: dock_handle,
            plugin_name: "".to_owned(),
            plugin_data: None,
            rect: Rect::default(),
        }
    }
}

impl Container {
    pub fn new(dock: Dock, rect: Rect) -> Container {
        Container {
            docks: vec!(dock),
            rect: rect,
        }
    }

    pub fn find_handle(&self, handle: DockHandle) -> Option<&Dock> {
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

    pub fn update_rect(&mut self, rect: Rect) {
        self.rect = rect;
        let (first_rect, second_rect) = self.rect.split_by_direction(self.direction, self.ratio);
        self.first.update_rect(first_rect);
        self.second.update_rect(second_rect);
    }

    pub fn get_sizer_from_rect_horizontal(rect: Rect, size: f32) -> Rect {
        Rect::new(rect.x, rect.y + rect.height, rect.width, size)
    }

    pub fn get_sizer_from_rect_vertical(rect: Rect, size: f32) -> Rect {
        Rect::new(rect.x + rect.width, rect.y, size, rect.height)
    }

//    pub fn is_hovering_rect(&self, pos: (f32, f32), border_size: f32, rect: Rect) -> bool {
//        unimplemented!();
//        match self.direction {
//            Direction::Horizontal => Rect::is_inside(pos, Self::get_sizer_from_rect_horizontal(rect, border_size)),
//            Direction::Vertical => Rect::is_inside(pos, Self::get_sizer_from_rect_vertical(rect, border_size)),
//            Direction::Full => false,
//        }
//    }

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
        return match **res {
            Area::Container(ref c) => c.get_dock_handle_at_pos(pos),
            Area::Split(ref c) => c.get_dock_handle_at_pos(pos),
        };
    }
}

impl Workspace {
    /// Construct a new workspace. The rect has to be y >= 0, x >= 0, width > 0 and height > 0
    pub fn new(rect: Rect) -> std::io::Result<Workspace> {
        if rect.x < 0.0 || rect.y < 0.0 || rect.width <= 0.0 || rect.height <= 0.0 {
            // TODO: Remove commented code
            //return Err(Error::IllegalSize(write!("Illegal rect {} {} - {} {}",
            //                              rect.x, rect.y, rect.width, rect.height)));
            //return Err(Error::IllegalSize("Illegal rect size".to_owned()));
        }

        Ok(Workspace {
            root_area: None,
            rect: rect,
            window_border: 4.0,
            handle_counter: SplitHandle(0),
        })
    }

    #[inline]
    fn next_handle(&mut self) -> SplitHandle {
        self.handle_counter.0 += 1;
        self.handle_counter
    }

    pub fn initialize(&mut self, handle: DockHandle) {
        self.root_area = Some(Area::Container(
            Container::new(
                Dock::new(handle),
                self.rect.clone()
            )
        ));
    }

    pub fn split_by_dock_handle(&mut self, direction: Direction, find_handle: DockHandle, handle: DockHandle) {
        let next_handle = self.next_handle();
        let holder = self.root_area.as_mut().and_then(|root| {
            root.find_container_holder_by_dock_handle(find_handle)
        });
        if let Some(area) = holder {
            let rect = area.get_rect();
            let first = area.clone();
            let second = Area::Container(Container::new(Dock::new(handle), Rect::default()));
            let split = Split::new(direction, 0.5, next_handle, first, second, rect);
            *area = Area::Split(split);
        }
    }

//    fn delete_handle(splits: &mut Vec<Split>, handle: DockHandle) {
//        for i in 0..splits.len() {
//            if splits[i].left_docks.remove_handle(handle) {
//                return;
//            }
//
//            if splits[i].right_docks.remove_handle(handle) {
//                return;
//            }
//        }
//    }

//    pub fn new_split(&mut self, handle: DockHandle, direction: Direction) {
//        if self.splits.len() == 1 {
//            self.splits[0].split_left(SplitHandle(0), handle, direction);
//        }
//    }

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        self.root_area.as_ref().and_then(|area| {
            area.find_container_by_dock_handle(handle).and_then(|container| {
                Some(container.rect.clone())
            })
        })
    }

    pub fn get_hover_dock(&self, pos: (f32, f32)) -> Option<DockHandle> {
        self.root_area.as_ref().and_then(|root| {
            match *root {
                Area::Container(ref c) => c.get_dock_handle_at_pos(pos),
                Area::Split(ref s) => s.get_dock_handle_at_pos(pos),
            }
        })
    }

    pub fn update(&mut self, new_rect: Rect) {
        self.rect = new_rect;
        // TODO: perform actual update
    }

//    pub fn drag_sizer(&mut self, handle: SplitHandle, delta: (f32, f32)) {
//        unimplemented!();
//        for split in &mut self.splits {
//            if split.handle == handle {
//                return split.change_ratio(delta);
//            }
//        }
//    }

//    pub fn get_sizer_at(&self, pos: (f32, f32)) -> Option<(SplitHandle, Direction)> {
//        for split in &self.splits {
//            if split.is_hovering_rect(pos, 8.0, split.left_docks.rect) {
//                return Some((split.handle, split.direction));
//            }
//        }
//
//        None
//    }

//    fn recursive_dump(splits: &Vec<Split>, handle: SplitHandle, level: i32) {
//        unimplemented!();
//        let i = Self::find_split_by_handle(splits, handle);
//
//        splits[i].dump_info(level);
//
//        if let Some(split_handle) = splits[i].left {
//            Self::recursive_dump(splits, split_handle, level + 1);
//        }
//
//        if let Some(split_handle) = splits[i].right {
//            Self::recursive_dump(splits, split_handle, level + 1);
//        }
//    }

    // testing

//    pub fn dump_tree(&self) {
//        unimplemented!();
//        Self::recursive_dump(&self.splits, SplitHandle(0), 0);
//    }

//    pub fn dump_tree_linear(&self) {
//        unimplemented!();
//        for split in &self.splits {
//            split.dump_info(0);
//        }
//    }

    fn replace_area(area: &mut Area, mut subs: Area) {
        subs.update_rect(area.get_rect());
        *area = subs;
    }

    pub fn delete_by_handle(&mut self, handle: DockHandle) {
        let mut should_delete_root = false;
        if let Some(Area::Container(ref c)) = self.root_area {
            if c.find_handle(handle).is_some() {
                should_delete_root = true;
            }
        }
        if should_delete_root {
            self.root_area = None;
            return;
        }
        if let Some(ref mut root) = self.root_area {
            match root.find_split_by_dock_handle(handle) {
                SplitSearchResult::None => {},
                SplitSearchResult::FirstChild(area) => {
                    let subs;
                    if let &mut Area::Split(ref mut s) = area {
                        subs = *s.second.clone();
                    } else { unreachable!() }
                    Self::replace_area(area, subs);
                },
                SplitSearchResult::SecondChild(area) => {
                    let subs;
                    if let &mut Area::Split(ref mut s) = area {
                        subs = *s.first.clone();
                    } else { unreachable!() }
                    Self::replace_area(area, subs);
                }
            }
        }
    }

    pub fn get_docks(&self) -> Vec<Dock> {
        unimplemented!();
//        let mut docks = Vec::new();
//
//        for split in &self.splits {
//            for dock in &split.left_docks.docks {
//                docks.push(dock.clone());
//            }
//
//            for dock in &split.right_docks.docks {
//                docks.push(dock.clone());
//            }
//        }
//
//        docks
    }

//    pub fn save(&self, file_name: &str) -> io::Result<()> {
//        unimplemented!();
//        let data = serde_json::to_string_pretty(self).unwrap_or("".to_owned());
//        let mut f = try!(File::create(file_name));
//        let _ = f.write_all(data.as_bytes());
//        println!("saved file");
//        Ok(())
//    }

//    pub fn load(file_name: &str) -> Workspace {
//        unimplemented!();
//        println!("tring to open {}", file_name);
//        let mut f = File::open(file_name).unwrap();
//        let mut s = String::new();
//        println!("tring to read {}", file_name);
//        f.read_to_string(&mut s).unwrap();
//        println!("tring to serc");
//        let ws: Workspace = serde_json::from_str(&s).unwrap();
//        ws
//    }
}

#[cfg(test)]
mod test {
    extern crate serde_json;

    use {Container, Split, Workspace, Dock, Rect, DockHandle, SplitHandle, Direction};

    fn check_range(inv: f32, value: f32, delta: f32) -> bool {
        (inv - value).abs() < delta
    }

    #[test]
    fn test_calc_rect_horz_half() {
        let rects = Rect::split_horizontally(Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.5);

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
        let rects = Workspace::split_horizontally(Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.25);

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
        let rects = Workspace::split_horizontally(Rect::new(16.0, 32.0, 512.0, 1024.0), 0.25);

        assert_eq!(check_range(rects.0.x, 16.0, 0.001), true);
        assert_eq!(check_range(rects.0.y, 32.0, 0.001), true);
        assert_eq!(check_range(rects.0.width, 512.0, 0.001), true);
        assert_eq!(check_range(rects.0.height, 256.0, 0.001), true);

        assert_eq!(check_range(rects.1.x, 16.0, 0.001), true);
        assert_eq!(check_range(rects.1.y, 288.0, 0.001), true);
        assert_eq!(check_range(rects.1.width, 512.0, 0.001), true);
        assert_eq!(check_range(rects.1.height, 768.0, 0.001), true);
    }

    #[test]
    fn test_gen_horizontal_size() {
        let border_size = 4.0;
        let rect_in = Rect::new(10.0, 20.0, 30.0, 40.0);
        let rect = Split::get_sizer_from_rect_horizontal(rect_in, border_size);

        assert_eq!(check_range(rect.x, rect_in.x, 0.001), true);
        assert_eq!(check_range(rect.y, 60.0, 0.001), true);
        assert_eq!(check_range(rect.width, rect_in.width, 0.001), true);
        assert_eq!(check_range(rect.height, border_size, 0.001), true);
    }

    #[test]
    fn test_gen_vertical_size() {
        let border_size = 4.0;
        let rect_in = Rect::new(10.0, 20.0, 30.0, 40.0);
        let rect = Split::get_sizer_from_rect_vertical(rect_in, border_size);

        assert_eq!(check_range(rect.x, 40.0, 0.001), true);
        assert_eq!(check_range(rect.y, rect_in.y, 0.001), true);
        assert_eq!(check_range(rect.width, border_size, 0.001), true);
        assert_eq!(check_range(rect.height, rect_in.height, 0.001), true);
    }

//    #[test]
//    fn test_inside_horizontal() {
//        let border_size = 4.0;
//        let rect = Rect::new(10.0, 20.0, 30.0, 40.0);
//        let rect_horz = Split::get_sizer_from_rect_horizontal(rect, border_size);
//
//        assert_eq!(Rect::is_inside((9.0, 61.0), rect_horz), false);
//        assert_eq!(Rect::is_inside((11.0, 61.0), rect_horz), true);
//    }

    #[test]
    fn test_rect_serialize() {
        let rect_in = Rect { x: 1.0, y: 2.0, width: 1024.0, height: 768.0 };
        let serialized = serde_json::to_string(&rect_in).unwrap();
        let rect_out: Rect = serde_json::from_str(&serialized).unwrap();

        assert_eq!(rect_in.x as i32, rect_out.x as i32);
        assert_eq!(rect_in.y as i32, rect_out.y as i32);
        assert_eq!(rect_in.width as i32, rect_out.width as i32);
        assert_eq!(rect_in.height as i32, rect_out.height as i32);
    }

    #[test]
    fn test_dockhandle_serialize() {
        let handle_in = DockHandle(0x1337);
        let serialized = serde_json::to_string(&handle_in).unwrap();
        let handle_out: DockHandle = serde_json::from_str(&serialized).unwrap();

        assert_eq!(handle_in, handle_out);
    }

    #[test]
    fn test_splithandle_serialize() {
        let handle_in = SplitHandle(0x4422);
        let serialized = serde_json::to_string(&handle_in).unwrap();
        let handle_out: SplitHandle = serde_json::from_str(&serialized).unwrap();

        assert_eq!(handle_in, handle_out);
    }

    #[test]
    fn test_dock_serialize_0() {
        let dock_in = Dock {
            handle: DockHandle(1),
            plugin_name: "disassembly".to_owned(),
            plugin_data: None,
            rect: Rect::new(1.0, 2.0, 3.0, 4.0)
        };

        let serialized = serde_json::to_string(&dock_in).unwrap();
        let dock_out: Dock = serde_json::from_str(&serialized).unwrap();

        assert_eq!(dock_in.handle, dock_out.handle);
        assert_eq!(dock_in.plugin_name, dock_out.plugin_name);
        assert_eq!(dock_in.plugin_data, dock_out.plugin_data);
        // expect that rect is not serialized and set to zero
        assert_eq!(dock_out.rect.x as i32, 0);
        assert_eq!(dock_out.rect.y as i32, 0);
        assert_eq!(dock_out.rect.width as i32, 0);
        assert_eq!(dock_out.rect.height as i32, 0);
    }

    #[test]
    fn test_dock_serialize_1() {
        let dock_in = Dock {
            handle: DockHandle(1),
            plugin_name: "registers".to_owned(),
            plugin_data: Some(vec!["some_data".to_owned(), "more_data".to_owned()]),
            rect: Rect::new(4.0, 5.0, 2.0, 8.0)
        };

        let serialized = serde_json::to_string(&dock_in).unwrap();
        let dock_out: Dock = serde_json::from_str(&serialized).unwrap();

        assert_eq!(dock_in.handle, dock_out.handle);
        assert_eq!(dock_in.plugin_name, dock_out.plugin_name);
        assert_eq!(dock_in.plugin_data, dock_out.plugin_data);

        // expect that rect is not serialized and set to zero
        assert_eq!(dock_out.rect.x as i32, 0);
        assert_eq!(dock_out.rect.y as i32, 0);
        assert_eq!(dock_out.rect.width as i32, 0);
        assert_eq!(dock_out.rect.height as i32, 0);

        let plugin_data = dock_out.plugin_data.as_ref().unwrap();

        assert_eq!(plugin_data.len(), 2);
        assert_eq!(plugin_data[0], "some_data");
        assert_eq!(plugin_data[1], "more_data");
    }

    #[test]
    fn test_container_serialize_0() {
        let container_in = Container {
            docks: Vec::new(),
            rect: Rect::new(4.0, 5.0, 2.0, 8.0)
        };

        let serialized = serde_json::to_string(&container_in).unwrap();
        let container_out: Container = serde_json::from_str(&serialized).unwrap();

        assert_eq!(container_out.docks.len(), 0);
        // expect that rect is not serialized and set to zero
        assert_eq!(container_out.rect.x as i32, 0);
        assert_eq!(container_out.rect.y as i32, 0);
        assert_eq!(container_out.rect.width as i32, 0);
        assert_eq!(container_out.rect.height as i32, 0);
    }

    #[test]
    fn test_container_serialize_1() {
        let container_in = Container {
            docks: vec![Dock {
                handle: DockHandle(1),
                plugin_name: "registers".to_owned(),
                plugin_data: Some(vec!["some_data".to_owned(), "more_data".to_owned()]),
                rect: Rect::new(4.0, 5.0, 2.0, 8.0)
            }],
            rect: Rect::default(),
        };

        let serialized = serde_json::to_string(&container_in).unwrap();
        let container_out: Container = serde_json::from_str(&serialized).unwrap();

        assert_eq!(container_out.docks.len(), 1);
        assert_eq!(container_out.docks[0].plugin_name, "registers");
    }

    #[test]
    fn test_split_serialize_0() {
        let split_in = Split {
            left: None,
            right: None,
            left_docks: Container::new(),
            right_docks: Container::new(),
            ratio: 0.7,
            direction: Direction::Full,
            handle: SplitHandle(1),
            rect: Rect::new(4.0, 5.0, 2.0, 8.0)
        };

        let serialized = serde_json::to_string(&split_in).unwrap();
        let split_out: Split = serde_json::from_str(&serialized).unwrap();

        assert_eq!(split_in.left, split_out.left);
        assert_eq!(split_in.right, split_out.right);
        assert_eq!(split_in.direction, split_out.direction);
        assert_eq!(split_in.handle, split_out.handle);

        // expect that rect is not serialized and set to zero
        assert_eq!(split_out.rect.x as i32, 0);
        assert_eq!(split_out.rect.y as i32, 0);
        assert_eq!(split_out.rect.width as i32, 0);
        assert_eq!(split_out.rect.height as i32, 0);
    }

    #[test]
    fn test_direction_serialize() {
        let dir_in_0 = Direction::Horizontal;
        let dir_in_1 = Direction::Full;
        let dir_in_2 = Direction::Vertical;

        let s0 = serde_json::to_string(&dir_in_0).unwrap();
        let s1 = serde_json::to_string(&dir_in_1).unwrap();
        let s2 = serde_json::to_string(&dir_in_2).unwrap();

        let dir_out_0: Direction = serde_json::from_str(&s0).unwrap();
        let dir_out_1: Direction = serde_json::from_str(&s1).unwrap();
        let dir_out_2: Direction = serde_json::from_str(&s2).unwrap();

        assert_eq!(dir_in_0, dir_out_0);
        assert_eq!(dir_in_1, dir_out_1);
        assert_eq!(dir_in_2, dir_out_2);
    }

    #[test]
    fn test_workspace_serialize_0() {
        let ws_in = Workspace {
            splits: Vec::new(),
            rect: Rect::new(4.0, 5.0, 2.0, 8.0),
            window_border: 6.0,
            handle_counter: SplitHandle(2),
        };

        let serialized = serde_json::to_string(&ws_in).unwrap();
        let ws_out: Workspace = serde_json::from_str(&serialized).unwrap();

        assert_eq!(ws_out.splits.len(), 0);
        assert_eq!(ws_out.window_border as i32, 6);
        assert_eq!(ws_out.rect.x as i32, 4);
        assert_eq!(ws_out.rect.y as i32, 5);
        assert_eq!(ws_out.rect.width as i32, 2);
        assert_eq!(ws_out.rect.height as i32, 8);
    }

    #[test]
    fn test_workspace_serialize_1() {
        let ws_in = Workspace {
            splits: vec![Split {
                left: None,
                right: None,
                left_docks: Container::new(),
                right_docks: Container::new(),
                ratio: 0.7,
                direction: Direction::Full,
                handle: SplitHandle(1),
                rect: Rect::new(4.0, 5.0, 2.0, 8.0)
            }],
            rect: Rect::new(4.0, 5.0, 2.0, 8.0),
            window_border: 6.0,
            handle_counter: SplitHandle(2),
        };

        let serialized = serde_json::to_string(&ws_in).unwrap();
        let ws_out: Workspace = serde_json::from_str(&serialized).unwrap();

        assert_eq!(ws_out.splits.len(), 1);
        assert_eq!(ws_out.splits[0].handle.0, 1);
    }
}
