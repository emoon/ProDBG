extern crate serde_json;
mod error;
mod data;
pub use self::error::Error;
//use std::io::prelude::*;
//use std::fs::File;
//use std::io;

#[derive(Debug, PartialEq, Clone, Copy)]
pub struct DockHandle(pub u64);

#[derive(Debug, PartialEq, Clone, Copy)]
pub struct SplitHandle(pub u64);

#[derive(Debug, Default, Clone, Copy)]
pub struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

#[derive(Debug, Clone)]
pub struct Dock {
    pub handle: DockHandle,
    pub plugin_name: String,
    pub plugin_data: Option<Vec<String>>,
    pub rect: Rect
}

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Direction {
    Vertical,
    Horizontal,
    Full,
}

#[derive(Debug, Clone)]
pub struct Container {
    pub docks: Vec<Dock>,
    pub rect: Rect,
}

#[derive(Debug)]
pub struct Split {
    /// left/top slipit
    pub left: Option<SplitHandle>,
    /// right/bottom split
    pub right: Option<SplitHandle>,
    /// left/top docks
    pub left_docks: Container,
    /// right/top docks
    pub right_docks: Container,
    /// ratio value of how much of each side that is visible. 1.0 = right/bottom fully visible
    pub ratio: f32,
    /// Direction of the split
    pub direction: Direction,
    /// Handle of the spliter
    pub handle: SplitHandle,
    /// Rect
    rect: Rect,
}

#[derive(Debug)]
pub struct Workspace {
    pub splits: Vec<Split>,
    pub rect: Rect,
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

    fn is_inside(v: (f32, f32), rect: Rect) -> bool {
        let x0 = rect.x;
        let y0 = rect.y;
        let x1 = x0 + rect.width;
        let y1 = y0 + rect.height;

        //println!("checking {:?} inside {:?}", v, rect);

        if (v.0 >= x0 && v.0 < x1) && (v.1 >= y0 && v.1 < y1) {
            true
        } else {
            false
        }
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
    pub fn new() -> Container {
        Container {
            docks: Vec::new(),
            rect: Rect::default(),
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

    /*
    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        self.docks.iter().find(|&dock| dock.handle == handle).map(|dock| dock.rect)
    }
    */

    pub fn is_inside(&self, pos: (f32, f32)) -> Option<(DockHandle, f32)> {
        for dock in &self.docks {
            if Rect::is_inside(pos, self.rect) {
                return Some((dock.handle, dock.rect.width * dock.rect.height));
            }
        }

        None
    }


}

impl Split {
    pub fn new(direction: Direction, ratio: f32, handle: SplitHandle) -> Split {
        Split {
            left: None,
            right: None,
            left_docks: Container::new(),
            right_docks: Container::new(),
            ratio: ratio,
            direction: direction,
            handle: handle,
            rect: Rect::default(),
        }
    }

    pub fn is_left_zero_and_none(&self) -> bool {
        self.left_docks.docks.len() == 0 && self.left.is_none()
    }

    pub fn is_right_zero_and_none(&self) -> bool {
        self.right_docks.docks.len() == 0 && self.right.is_none()
    }

    fn adjust_ratio_after_add(&mut self) {
        if self.right_docks.docks.len() == 0 {
            self.ratio = 1.0;
        } else if self.left_docks.docks.len() == 0 {
            self.ratio = 0.0;
        } else {
            self.ratio = 0.5;
        }
    }

    pub fn no_split(&mut self, direction: Direction, dock_handle: DockHandle) -> bool {
        // hack!
        if self.direction == Direction::Full {
            self.direction = direction;
        }

        if self.is_left_zero_and_none() {
            //println!("no_split: is_left_zero_and_none");
            self.left_docks.docks.push(Dock::new(dock_handle));
            self.adjust_ratio_after_add();
            return true;
        }

        if self.is_right_zero_and_none() {
            //println!("no_split: is_right_zero_and_none");
            self.right_docks.docks.push(Dock::new(dock_handle));
            self.adjust_ratio_after_add();
            return true;
        }

        false
    }

    pub fn split_left(&mut self, split_handle: SplitHandle, dock_handle: DockHandle, direction: Direction) -> Option<Split> {
        if Self::no_split(self, direction, dock_handle) {
            return None;
        }

        let mut split = Split::new(direction, 0.5, split_handle);
        split.right_docks = self.left_docks.clone();
        split.left_docks.docks.push(Dock::new(dock_handle));
        self.ratio = 0.5;
        self.left = Some(split_handle);
        self.left_docks.docks.clear();

        Some(split)
    }

    pub fn split_right(&mut self, split_handle: SplitHandle, dock_handle: DockHandle, direction: Direction) -> Option<Split> {
        if Self::no_split(self, direction, dock_handle) {
            return None;
        }

        let mut split = Split::new(direction, 0.5, split_handle);
        split.left_docks = self.right_docks.clone();
        split.right_docks.docks.push(Dock::new(dock_handle));
        self.ratio = 0.5;
        self.right = Some(split_handle);
        self.right_docks.docks.clear();

        Some(split)
    }


    pub fn get_sizer_from_rect_horizontal(rect: Rect, size: f32) -> Rect {
        Rect::new(rect.x, rect.y + rect.height, rect.width, size)
    }

    pub fn get_sizer_from_rect_vertical(rect: Rect, size: f32) -> Rect {
        Rect::new(rect.x + rect.width, rect.y, size, rect.height)
    }

    pub fn is_hovering_rect(&self, pos: (f32, f32), border_size: f32, rect: Rect) -> bool {
        match self.direction {
            Direction::Horizontal => Rect::is_inside(pos, Self::get_sizer_from_rect_horizontal(rect, border_size)),
            Direction::Vertical => Rect::is_inside(pos, Self::get_sizer_from_rect_vertical(rect, border_size)),
            Direction::Full => false,
        }
    }

    pub fn map_rect_to_delta(&self, delta: (f32, f32)) -> f32 {
        match self.direction {
            Direction::Vertical => -delta.0 / self.rect.width,
            Direction::Horizontal => -delta.1 / self.rect.height,
            _ => 0.0,
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

    fn pad(level: i32) {
        for _ in 0..level {
            print!(" ");
        }
    }

    pub fn dump_info(&self, level: i32) {
        let l_count = self.left_docks.docks.len();
        let r_count = self.right_docks.docks.len();

        Self::pad(level);

        println!("Split - Dir {:?} - handle {} - ratio {} count ({}, {}) - left ({:?}) right ({:?}) - rect {} {} - {} {}",
        self.direction, self.handle.0, self.ratio, l_count, r_count,
        self.left, self.right, self.rect.x, self.rect.y, self.rect.width, self.rect.height);

        for d in &self.left_docks.docks {
            Self::pad(level);
            println!("left dock handle - {}", d.handle.0);
        }

        for d in &self.right_docks.docks {
            Self::pad(level);
            println!("right dock handle - {}", d.handle.0);
        }
    }

}

impl Workspace {
    /// Construct a new workspace. The rect has to be y >= 0, x >= 0, width > 0 and height > 0
    pub fn new(rect: Rect) -> std::io::Result<Workspace> {
        if rect.x < 0.0 || rect.y < 0.0 || rect.width <= 0.0 || rect.height <= 0.0 {
            //return Err(Error::IllegalSize(write!("Illegal rect {} {} - {} {}",
            //                              rect.x, rect.y, rect.width, rect.height)));
            //return Err(Error::IllegalSize("Illegal rect size".to_owned()));
        }

        Ok(Workspace {
            splits: vec![Split::new(Direction::Full, 0.0, SplitHandle(0))],
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

    pub fn calc_horizontal_sizing(rect: Rect, ratio: f32) -> (Rect, Rect) {
        let h = rect.height * ratio;

        let rect_top = Rect::new(rect.x, rect.y, rect.width, h);
        let rect_bottom = Rect::new(rect.x, rect.y + h, rect.width, rect.height - h);

        (rect_top, rect_bottom)
    }

    pub fn calc_vertical_sizing(rect: Rect, ratio: f32) -> (Rect, Rect) {
        let w = rect.width * ratio;

        let rect_left = Rect::new(rect.x, rect.y, w, rect.height);
        let rect_right = Rect::new(rect.x + w, rect.y, rect.width - w, rect.height);

        (rect_left, rect_right)
    }

    fn calc_rects(direction: Direction, rect: Rect, ratio: f32) -> (Rect, Rect) {
        match direction {
            Direction::Vertical => Self::calc_vertical_sizing(rect, ratio),
            Direction::Horizontal => Self::calc_horizontal_sizing(rect, ratio),
            Direction::Full => (rect, rect),
        }
    }

    pub fn split(&mut self, dock_handle: DockHandle, direction: Direction) {
        // If we have only one split we right now split on the left side
        if self.splits.len() == 1 {
            let handle = self.next_handle();
            self.splits[0].split_left(handle, dock_handle, direction);
        }
    }

    fn find_split_by_handle(splits: &Vec<Split>, handle: SplitHandle) -> usize {
        for i in 0..splits.len() {
            if splits[i].handle == handle {
                return i;
            }
        }

        panic!("Should never happen!")
    }

    fn recursive_update(splits: &mut Vec<Split>, rect: Rect, handle: SplitHandle) {
        let i = Self::find_split_by_handle(splits, handle);

        let rects = Self::calc_rects(splits[i].direction, rect, splits[i].ratio);

        if let Some(split_handle) = splits[i].left {
            Self::recursive_update(splits, rects.0, split_handle);
        }

        if let Some(split_handle) = splits[i].right {
            Self::recursive_update(splits, rects.1, split_handle);
        }

        splits[i].rect = rect;
        splits[i].left_docks.rect = rects.0;
        splits[i].right_docks.rect = rects.1;
    }

    pub fn split_by_dock_handle(&mut self, direction: Direction, find_handle: DockHandle, handle: DockHandle) {
        let split_handle = self.next_handle();
        let mut temp_splits = Vec::new();

        //println!("split_by_dock_handle");

        for split in &mut self.splits {
            if split.left_docks.find_handle(find_handle).is_some() {
                if let Some(s) = split.split_left(split_handle, handle, direction) {
                    temp_splits.push(s);
                }
            }

            if split.right_docks.find_handle(find_handle).is_some() {
                if let Some(s) = split.split_right(split_handle, handle, direction) {
                    temp_splits.push(s);
                }
            }
        }

        for split in temp_splits {
            self.splits.push(split);
        }
    }

    fn delete_handle(splits: &mut Vec<Split>, handle: DockHandle) {
        for i in 0..splits.len() {
            if splits[i].left_docks.remove_handle(handle) {
                return;
            }

            if splits[i].right_docks.remove_handle(handle) {
                return;
            }
        }
    }

    pub fn new_split(&mut self, handle: DockHandle, direction: Direction) {
        if self.splits.len() == 1 {
            self.splits[0].split_left(SplitHandle(0), handle, direction);
        }
    }

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        for split in &self.splits {
            if let Some(rect) = split.left_docks.get_rect_by_handle(handle) {
                return Some(rect);
            }

            if let Some(rect) = split.right_docks.get_rect_by_handle(handle) {
                return Some(rect);
            }
        }

        None
    }

    pub fn get_hover_dock(&self, pos: (f32, f32)) -> Option<DockHandle> {
        let mut dock_handle = None;
        let mut last_size = 100000000.0;

        for split in &self.splits {
            if let Some(data) = split.left_docks.is_inside(pos) {
                if data.1 < last_size {
                    dock_handle = Some(data.0);
                    last_size = data.1;
                }
            }

            if let Some(data) = split.right_docks.is_inside(pos) {
                if data.1 < last_size {
                    dock_handle = Some(data.0);
                    last_size = data.1;
                }
            }

        }

        dock_handle
    }


    fn clean_splits(splits: &mut Vec<Split>) -> bool {
        if splits.len() == 1 {
            return false;
        }

        for i in (0..splits.len()).rev() {
            if splits[i].is_left_zero_and_none() && splits[i].is_right_zero_and_none() {
                let handle = Some(splits[i].handle);

                for split in splits.iter_mut() {
                    if split.left == handle {
                        split.ratio = 0.0;
                        split.left = None;
                    }

                    if split.right == handle {
                        split.ratio = 1.0;
                        split.right = None;
                    }
                }

                splits.swap_remove(i);
                return true;
            }
        }

        false
    }

    fn cleanup_after_delete(splits: &mut Vec<Split>) {
        loop {
            if !Self::clean_splits(splits) {
                break;
            }
        }

        // If there is nothing left (empty space) we switch the direction
        // to full for the first splitter

        if splits.len() == 1 {
            if splits[0].is_left_zero_and_none() && splits[0].is_right_zero_and_none() {
                splits[0].direction = Direction::Full;
            }
        }
    }

    fn adjust_percentages(splits: &mut Vec<Split>) {
        for split in splits {
            if split.is_left_zero_and_none() {
                split.ratio = 0.0;
            } else if split.is_right_zero_and_none() {
                split.ratio = 1.0;
            }
        }
    }

    pub fn update(&mut self) {
        Self::recursive_update(&mut self.splits, self.rect, SplitHandle(0));
    }

    pub fn drag_sizer(&mut self, handle: SplitHandle, delta: (f32, f32)) {
        for split in &mut self.splits {
            if split.handle == handle {
                return split.change_ratio(delta);
            }
        }
    }

    pub fn is_hovering_sizer(&self, pos: (f32, f32)) -> Option<(SplitHandle, Direction)> {
        for split in &self.splits {
            if split.is_hovering_rect(pos, 8.0, split.left_docks.rect) {
                return Some((split.handle, split.direction));
            }
        }

        None
    }

    fn recursive_dump(splits: &Vec<Split>, handle: SplitHandle, level: i32) {
        let i = Self::find_split_by_handle(splits, handle);

        splits[i].dump_info(level);

        if let Some(split_handle) = splits[i].left {
            Self::recursive_dump(splits, split_handle, level + 1);
        }

        if let Some(split_handle) = splits[i].right {
            Self::recursive_dump(splits, split_handle, level + 1);
        }
    }

    // testing

    pub fn dump_tree(&self) {
        Self::recursive_dump(&self.splits, SplitHandle(0), 0);
    }

    pub fn dump_tree_linear(&self) {
        for split in &self.splits {
            split.dump_info(0);
        }
    }

    pub fn delete_by_handle(&mut self, handle: DockHandle) {
        Self::delete_handle(&mut self.splits, handle);
        Self::cleanup_after_delete(&mut self.splits);
        Self::adjust_percentages(&mut self.splits);
    }

    pub fn get_docks(&self) -> Vec<Dock> {
        let mut docks = Vec::new();

        for split in &self.splits {
            for dock in &split.left_docks.docks {
                docks.push(dock.clone());
            }

            for dock in &split.right_docks.docks {
                docks.push(dock.clone());
            }
        }

        docks
    }

    /*
    pub fn save(&self, file_name: &str) -> io::Result<()> {
        let data = serde_json::to_string_pretty(self).unwrap_or("".to_owned());
        let mut f = try!(File::create(file_name));
        let _ = f.write_all(data.as_bytes());
        println!("saved file");
        Ok(())
    }

    pub fn load(file_name: &str) -> Workspace {
        println!("tring to open {}", file_name);
        let mut f = File::open(file_name).unwrap();
        let mut s = String::new();
        println!("tring to read {}", file_name);
        f.read_to_string(&mut s).unwrap();
        println!("tring to serc");
        let ws: Workspace = serde_json::from_str(&s).unwrap();
        ws
    }
    */
}

#[cfg(test)]
mod test {
    extern crate serde_json;

    use {Container, Split, Workspace, Dock, Rect, DockHandle, SplitHandle, Direction};

    fn check_range(inv: f32, value: f32, delta: f32) -> bool {
        (inv - value).abs() < delta
    }

    #[test]
    fn test_split_top() {
        let mut ws = Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).unwrap();
        ws.split(DockHandle(1), Direction::Vertical);

        assert_eq!(ws.splits.len(), 1);
        let split = &ws.splits[0];

        assert_eq!(split.left_docks.docks.len(), 1);
    }

    #[test]
    fn test_split_top_2() {
        let mut ws = Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).unwrap();
        ws.split(DockHandle(1), Direction::Vertical);
        ws.split(DockHandle(2), Direction::Vertical);

        assert_eq!(ws.splits.len(), 1);
        let split = &ws.splits[0];

        assert_eq!(split.right_docks.docks.len(), 1);
        assert_eq!(split.left_docks.docks.len(), 1);
        assert_eq!(check_range(split.ratio, 0.5, 0.01), true);
    }

    #[test]
    fn test_find_dock_handle() {
        let mut ws = Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).unwrap();
        ws.split(DockHandle(1), Direction::Vertical);
        ws.split(DockHandle(2), Direction::Vertical);
        ws.split_by_dock_handle(Direction::Horizontal, DockHandle(2), DockHandle(3));

        assert_eq!(ws.splits.len(), 2);

        assert_eq!(ws.get_rect_by_handle(DockHandle(1)).is_some(), true);
        assert_eq!(ws.get_rect_by_handle(DockHandle(2)).is_some(), true);
        assert_eq!(ws.get_rect_by_handle(DockHandle(2)).is_some(), true);
        assert_eq!(ws.get_rect_by_handle(DockHandle(4)).is_some(), false);
    }

    #[test]
    fn test_calc_rect_horz_half() {
        let rects = Workspace::calc_horizontal_sizing(Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.5);

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
        let rects = Workspace::calc_horizontal_sizing(Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.25);

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
        let rects = Workspace::calc_horizontal_sizing(Rect::new(16.0, 32.0, 512.0, 1024.0), 0.25);

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

    #[test]
    fn test_inside_horizontal() {
        let border_size = 4.0;
        let rect = Rect::new(10.0, 20.0, 30.0, 40.0);
        let rect_horz = Split::get_sizer_from_rect_horizontal(rect, border_size);

        assert_eq!(Rect::is_inside((9.0, 61.0), rect_horz), false);
        assert_eq!(Rect::is_inside((11.0, 61.0), rect_horz), true);
    }

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

        let s0 = serde_json::to_string(&dir_in_0).unwrap();
        let s1 = serde_json::to_string(&dir_in_1).unwrap();

        println!("s0 {}", s0);

        let dir_out_0: Direction = serde_json::from_str(&s0).unwrap();
        let dir_out_1: Direction = serde_json::from_str(&s1).unwrap();

        assert_eq!(dir_in_0, dir_out_0);
        assert_eq!(dir_in_1, dir_out_1);
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
