mod error;
pub use self::error::Error;

pub type Result<T> = std::result::Result<T, Error>;

#[derive(Debug, Clone, Copy)]
pub struct DockHandle(pub u64);

#[derive(Debug, Clone, Copy)]
pub struct SplitHandle(pub u64);

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
}

#[derive(Debug, Clone)]
pub struct Dock {
    pub handle: DockHandle,
    pub rect: Rect
}

impl Dock {
    fn new(dock_handle: DockHandle) -> Dock {
        Dock {
            handle: dock_handle,
            rect: Rect::default(),
        }
    }
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

impl Container {
    pub fn new() -> Container {
        Container {
            docks: Vec::new(),
            rect: Rect::default(),
        }
    }

}

#[derive(Debug)]
pub struct Split {
    /// left/top slipit
    pub left: Option<Box<Split>>,
    /// right/bottom split
    pub right: Option<Box<Split>>,
    /// left/top docks
    pub left_docks: Container,
    /// right/top docks
    pub right_docks: Container,
    /// ratioage value of how much of each side that is visible. 1.0 = right/bottom fully visible
    pub ratio: f32,
    /// Direction of the split
    pub direction: Direction,
    /// Handle of the spliter
    pub handle: SplitHandle,

    rect: Rect,
}

impl Split {
    pub fn new(direction: Direction, handle: SplitHandle) -> Split {
        Split {
            left: None,
            right: None,
            left_docks: Container::new(),
            right_docks: Container::new(),
            ratio: 0.0,
            direction: direction,
            handle: handle,
            rect: Rect::default(),
        }
    }

    pub fn no_split(&mut self, direction: Direction, dock_handle: DockHandle) -> bool {
        // hack!
        if self.direction == Direction::Full {
            self.direction = direction;
        }

        if self.left_docks.docks.len() == 0 && self.left.is_none() {
            self.left_docks.docks.push(Dock::new(dock_handle));
            self.ratio = 0.5;
            return true;
        }

        if self.right_docks.docks.len() == 0 && self.right.is_none() {
            self.right_docks.docks.push(Dock::new(dock_handle));
            self.ratio = 0.5;
            return true;
        }

        false
    }

    pub fn split_left(&mut self, split_handle: SplitHandle, dock_handle: DockHandle, direction: Direction) {
        if Self::no_split(self, direction, dock_handle) {
            return;
        } else {
            let mut split = Box::new(Split::new(direction, split_handle));
            split.right_docks = self.left_docks.clone();
            split.left_docks.docks.push(Dock::new(dock_handle));
            split.ratio = 0.5;
            self.left = Some(split);
            self.left_docks.docks.clear();
        }
    }

    pub fn split_right(&mut self, split_handle: SplitHandle, dock_handle: DockHandle, direction: Direction) {
        if Self::no_split(self, direction, dock_handle) {
            return;
        } else {
            let mut split = Box::new(Split::new(direction, split_handle));
            split.left_docks = self.right_docks.clone();
            split.right_docks.docks.push(Dock::new(dock_handle));
            split.ratio = 0.5;
            self.right = Some(split);
            self.right_docks.docks.clear();
        }
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

    fn recursive_update(&mut self, rect: Rect, level: usize) {
        let rects = Self::calc_rects(self.direction, rect, self.ratio);

        self.rect = rect;

        if let Some(ref mut split) = self.left {
            Self::recursive_update(split, rects.0, level + 1);
        }

        if let Some(ref mut split) = self.right {
            Self::recursive_update(split, rects.1, level + 1);
        }

        self.left_docks.rect = rects.0;
        self.right_docks.rect = rects.1;

        // TODO: Remove these loops, should be propagated to update call only

        for dock in &mut self.left_docks.docks {
            dock.rect = rects.0;
        }

        for dock in &mut self.right_docks.docks {
            dock.rect = rects.1;
        }
    }

    pub fn split_by_dock_handle(&mut self, direction: Direction, split_handle: SplitHandle, find_handle: DockHandle, handle: DockHandle) -> bool {
        // TODO: Fix me
        let left_docks = self.left_docks.docks.clone();
        let right_docks = self.right_docks.docks.clone();

        for dock in left_docks {
            if dock.handle.0 == find_handle.0 {
                self.split_left(split_handle, handle, direction);
                return true;
            }
        }

        for dock in right_docks {
            if dock.handle.0 == find_handle.0 {
                self.split_right(split_handle, handle, direction);
                return true;
            }
        }

        if let Some(ref mut split) = self.left {
            if Self::split_by_dock_handle(split, direction, split_handle, find_handle, handle) {
                return true;
            }
        }

        if let Some(ref mut split) = self.right {
            if Self::split_by_dock_handle(split, direction, split_handle, find_handle, handle) {
                return true;
            }
        }

        false
    }

    fn is_inside(v: (f32, f32), rect: Rect) -> bool {
        let x0 = rect.x;
        let y0 = rect.y;
        let x1 = x0 + rect.width;
        let y1 = y0 + rect.height;

        if (v.0 >= x0 && v.0 < x1) && (v.1 >= y0 && v.1 < y1) {
            true
        } else {
            false
        }
    }

    fn recrusive_collect_docks(&self, docks: &mut Vec<Dock>) {
        for h in &self.left_docks.docks {
            docks.push(h.clone());
        }

        for h in &self.right_docks.docks {
            docks.push(h.clone());
        }

        if let Some(ref split) = self.left {
            Self::recrusive_collect_docks(split, docks);
        }

        if let Some(ref split) = self.right {
            Self::recrusive_collect_docks(split, docks);
        }
    }

    fn pad(level: i32) {
        for _ in 0..level {
            print!(" ");
        }
    }

    fn dump(&self, level: i32) {
        Self::pad(level);

        let l_count = self.left_docks.docks.len();
        let r_count = self.right_docks.docks.len();

        println!("Split - Dir {:?} - handle {} - ratio {} count ({}, {}) - left ({}) right ({}) - rect {} {} - {} {}",
                self.direction, self.handle.0, self.ratio, l_count, r_count,
                self.left.is_some(), self.right.is_some(),
                self.rect.x, self.rect.y, self.rect.width, self.rect.height);

        for d in &self.left_docks.docks {
            Self::pad(level);
            println!("left dock - {} - rect {} {} - {} {}", d.handle.0, d.rect.x, d.rect.y, d.rect.width, d.rect.height);
        }

        for d in &self.right_docks.docks {
            Self::pad(level);
            println!("right dock - {} - rect {} {} - {} {}", d.handle.0, d.rect.x, d.rect.y, d.rect.width, d.rect.height);
        }

        if let Some(ref split) = self.left {
            Self::dump(split, level + 1);
        }

        if let Some(ref split) = self.right {
            Self::dump(split, level + 1);
        }
    }

    fn get_sizer_from_rect_horizontal(rect: Rect, size: f32) -> Rect {
        Rect::new(rect.x, rect.y + rect.height, rect.width, size)
    }

    fn get_sizer_from_rect_vertical(rect: Rect, size: f32) -> Rect {
        Rect::new(rect.x + rect.width, rect.y, size, rect.height)
    }

    fn is_hovering_rect(pos: (f32, f32), border_size: f32, rect: Rect, direction: Direction) -> bool {
        match direction {
            Direction::Horizontal => Self::is_inside(pos, Self::get_sizer_from_rect_horizontal(rect, border_size)),
            Direction::Vertical => Self::is_inside(pos, Self::get_sizer_from_rect_vertical(rect, border_size)),
            Direction::Full => false,
        }
    }

    pub fn is_hovering_sizer(&self, pos: (f32, f32)) -> Option<SplitHandle> {
        if Self::is_hovering_rect(pos, 8.0, self.left_docks.rect, self.direction) {
            return Some(self.handle)
        }

        if let Some(ref split) = self.left {
            if let Some(handle) = Self::is_hovering_sizer(split, pos) {
                return Some(handle);
            }
        }

        if let Some(ref split) = self.right {
            if let Some(handle) = Self::is_hovering_sizer(split, pos) {
                return Some(handle);
            }
        }

        None
    }

    fn map_rect_to_delta(&self, delta: (f32, f32)) -> f32 {
        match self.direction {
            Direction::Vertical => -delta.0 / self.rect.width,
            Direction::Horizontal => -delta.1 / self.rect.height,
            _ => 0.0,
        }
    }

    fn change_ratio(&mut self, delta: (f32, f32)) {
        let scale = Self::map_rect_to_delta(self, delta);

        self.ratio += scale;

        if self.ratio < 0.05 {
            self.ratio = 0.05;
        }

        if self.ratio > 0.95 {
            self.ratio = 0.95;
        }
    }

    pub fn delete_by_handle(&mut self, handle: DockHandle) -> bool {
        for i in (0..self.left_docks.docks.len()).rev() {
            println!("Matching left {} - {}", self.left_docks.docks[i].handle.0, handle.0);
            if self.left_docks.docks[i].handle.0 == handle.0 {
                self.ratio = 0.0;
                println!("deleted left dock!");
                self.left_docks.docks.swap_remove(i);
            }
        }

        for i in (0..self.right_docks.docks.len()).rev() {
            println!("Matching right {} - {}", self.right_docks.docks[i].handle.0, handle.0);
            if self.right_docks.docks[i].handle.0 == handle.0 {
                self.ratio = 1.0;
                println!("deleted right dock!");
                self.right_docks.docks.swap_remove(i);
            }
        }

        // if nothing left in the docks we return true to notify
        // the parent that the split should be removed

        if self.left_docks.docks.len() == 0 && self.left.is_none() &&
            self.right_docks.docks.len() == 0 && self.right.is_none() {
            println!("We can delete the split also! - {}", self.handle.0);
            return true;
        }

        let mut remove_node = false;

        if let Some(ref mut split) = self.left {
            if Self::delete_by_handle(split, handle) {
                remove_node = true;
            }
        }

        if remove_node {
            self.ratio = 0.0;
            self.left = None;
        }

        remove_node = false;

        if let Some(ref mut split) = self.right {
            if Self::delete_by_handle(split, handle) {
                remove_node = true;
            }
        }

        if remove_node {
            self.ratio = 1.0;
            self.right = None;
        }

        false
    }

    pub fn cleanup_delete(&mut self) -> bool {
        if self.left_docks.docks.len() == 0 && self.left.is_none() &&
            self.right_docks.docks.len() == 0 && self.right.is_none() {
            println!("cleanup delete {}", self.handle.0);
            return true;
        }

        if self.left.as_mut().map(|split| split.cleanup_delete()).unwrap_or(false) {
            self.left = None;
        }

        if self.right.as_mut().map(|split| split.cleanup_delete()).unwrap_or(false) {
            self.right = None;
        }

        false
    }

    pub fn adjust_percentage(&mut self) {
        if self.left_docks.docks.len() == 0 && self.left.is_none() {
            self.ratio = 0.0;
            println!("adjust left ratio");
        } else if self.right_docks.docks.len() == 0 && self.right.is_none() {
            self.ratio = 1.0;
            println!("adjust right ratio");
        }

        if let Some(ref mut split) = self.right {
            Self::adjust_percentage(split);
        }

        if let Some(ref mut split) = self.right {
            Self::adjust_percentage(split);
        }
    }


    pub fn drag_sizer(&mut self, handle: SplitHandle, delta: (f32, f32)) {
        if self.handle.0 == handle.0 {
            return Self::change_ratio(self, delta);
        }

        if let Some(ref mut split) = self.left {
            Self::drag_sizer(split, handle, delta);
        }

        if let Some(ref mut split) = self.right {
            Self::drag_sizer(split, handle, delta);
        }
    }

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        let mut docks = Vec::<Dock>::new();

        Self::recrusive_collect_docks(self, &mut docks);

        for dock in &docks {
            if dock.handle.0 == handle.0 {
                return Some(dock.rect);
            }
        }

        None
    }

    pub fn get_hover_dock(&self, pos: (f32, f32)) -> Option<DockHandle> {
        let mut docks = Vec::<Dock>::new();
        let mut last_size = 100000000.0;
        let mut dock_handle: Option<DockHandle> = None;

        Self::recrusive_collect_docks(self, &mut docks);

        for dock in &docks {
            if Self::is_inside(pos, dock.rect) {
                let size = dock.rect.width * dock.rect.height;
                if size < last_size {
                    last_size = size;
                    dock_handle = Some(dock.handle);
                }
            }
        }

        dock_handle
    }
}

pub struct Workspace {
    pub split: Option<Box<Split>>,
    pub rect: Rect,
    /// border size of the windows (in pixels)
    pub window_border: f32,
    handle_counter: SplitHandle,
}

impl Workspace {
    /// Construct a new workspace. The rect has to be y >= 0, x >= 0, width > 0 and height > 0
    pub fn new(rect: Rect) -> Result<Workspace> {
        if rect.x < 0.0 {
            return Err(Error::IllegalSize("x has to be non-negative".to_owned()));
        }

        if rect.y < 0.0 {
            return Err(Error::IllegalSize("y has to be non-negative".to_owned()));
        }

        if rect.width <= 0.0 {
            return Err(Error::IllegalSize("width has to be larger than 0.0".to_owned()));
        }

        if rect.height <= 0.0 {
            return Err(Error::IllegalSize("height has to be larger than 0.0".to_owned()));
        }

        Ok(Workspace {
            split: None,
            rect: rect,
            window_border: 4.0,
            handle_counter: SplitHandle(1),
        })
    }

    /// This code gets called when the top split is None. This mean that the dock will be
    /// set to fullscreen as there are no other splits to be done
    fn split_new(&mut self, split_handle: SplitHandle, dock_handle: DockHandle) {
        let mut split = Box::new(Split::new(Direction::Full, split_handle));
        split.ratio = 1.0;
        split.left_docks.docks.push(Dock::new(dock_handle));
        split.rect = self.rect;
        self.split = Some(split);
    }

    pub fn split_top(&mut self, dock_handle: DockHandle, direction: Direction) {
        self.handle_counter.0 += 1;
        let split_handle = self.handle_counter;
        if let Some(ref mut split) = self.split {
            split.split_left(split_handle, dock_handle, direction);
        } else {
            Self::split_new(self, split_handle, dock_handle);
        }
    }

    pub fn update(&mut self) {
        let rect = self.rect.clone();
        if let Some(ref mut split) = self.split {
            split.recursive_update(rect, 0);
        }
    }

    pub fn split_by_dock_handle(&mut self, direction: Direction, find_handle: DockHandle, handle: DockHandle) {
        if let Some(ref mut split) = self.split {
            self.handle_counter.0 += 1;
            split.split_by_dock_handle(direction, self.handle_counter, find_handle, handle);
        }
    }

    pub fn is_hovering_sizer(&self, pos: (f32, f32)) -> Option<SplitHandle> {
        if let Some(ref split) = self.split {
            split.is_hovering_sizer(pos)
        } else {
            None
        }
    }

    pub fn is_hovering_dock(&self, pos: (f32, f32)) -> Option<DockHandle> {
        if let Some(ref split) = self.split {
            split.get_hover_dock(pos)
        } else {
            None
        }
    }

    pub fn drag_sizer(&mut self, handle: SplitHandle, delta: (f32, f32)) {
        if let Some(ref mut split) = self.split {
            split.drag_sizer(handle, delta)
        }
    }

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        if let Some(ref split) = self.split {
            split.get_rect_by_handle(handle)
        } else {
            None
        }
    }

    pub fn dump_tree(&self) {
        if let Some(ref split) = self.split {
            split.dump(0)
        }
    }

    pub fn delete_by_handle(&mut self, handle: DockHandle) {
        if let Some(ref mut split) = self.split {
            println!("About to delete {}", handle.0);
            let _ = split.delete_by_handle(handle);
            let _ = split.cleanup_delete();
            split.adjust_percentage();
        }
    }
}

#[cfg(test)]
mod test {
    use {Split, Workspace, Rect, DockHandle, Direction};

    fn check_range(inv: f32, value: f32, delta: f32) -> bool {
        (inv - value).abs() < delta
    }

    #[test]
    fn test_validate_x_less_than_zero() {
        assert_eq!(Workspace::new(Rect::new(-0.1, 0.0, 1.0, 1.0)).is_err(), true);
    }

    #[test]
    fn test_validate_y_less_than_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, -0.1, 1.0, 1.0)).is_err(), true);
    }

    #[test]
    fn test_validate_width_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, 0.0, 1.0)).is_err(), true);
    }

    #[test]
    fn test_validate_height_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, 1.0, 0.0)).is_err(), true);
    }

    #[test]
    fn test_validate_width_less_than_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, -1.0, 0.0)).is_err(), true);
    }

    #[test]
    fn test_validate_height_less_than_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, 0.0, -1.0)).is_err(), true);
    }

    #[test]
    fn test_validate_workspace_ok() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).is_ok(), true);
    }

    #[test]
    fn test_split_top() {
        let mut ws = Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).unwrap();
        ws.split_top(DockHandle(1), Direction::Vertical);

        assert_eq!(ws.split.is_some(), true);
        let split = ws.split.unwrap();

        assert_eq!(split.left_docks.docks.len(), 1);
    }

    #[test]
    fn test_split_top_2() {
        let mut ws = Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).unwrap();
        ws.split_top(DockHandle(1), Direction::Vertical);
        ws.split_top(DockHandle(2), Direction::Vertical);

        assert_eq!(ws.split.is_some(), true);
        let split = ws.split.unwrap();

        assert_eq!(split.right_docks.docks.len(), 1);
        assert_eq!(split.left_docks.docks.len(), 1);
        assert_eq!(check_range(split.ratio, 0.5, 0.01), true);
    }

    #[test]
    fn test_calc_rect_horz_half() {
        let rects = Split::calc_horizontal_sizing(Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.5);

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
        let rects = Split::calc_horizontal_sizing(Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.25);

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
        let rects = Split::calc_horizontal_sizing(Rect::new(16.0, 32.0, 512.0, 1024.0), 0.25);

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

        assert_eq!(Split::is_inside((9.0, 61.0), rect_horz), false);
        assert_eq!(Split::is_inside((11.0, 61.0), rect_horz), true);
    }
}
