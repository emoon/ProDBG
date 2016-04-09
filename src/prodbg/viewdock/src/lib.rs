mod error;
pub use self::error::Error;

pub type Result<T> = std::result::Result<T, Error>;

#[derive(Clone, Copy)]
pub struct ViewHandle(pub u64);

#[derive(Clone, Copy)]
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

#[derive(Clone)]
pub struct View {
    pub handle: ViewHandle,
    pub rect: Rect
}

impl View {
    fn new(view_handle: ViewHandle) -> View {
        View {
            handle: view_handle,
            rect: Rect::default(),
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub enum Direction {
    Vertical,
    Horizontal,
    Full,
}

#[derive(Clone)]
pub struct Container {
    pub views: Vec<View>,
    pub rect: Rect,
}

impl Container {
    pub fn new() -> Container {
        Container {
            views: Vec::new(),
            rect: Rect::default(),
        }
    }

}

pub struct Split {
    /// left/top slipit 
    pub left: Option<Box<Split>>,
    /// right/bottom split
    pub right: Option<Box<Split>>,
    /// left/top views
    pub left_views: Container,
    /// right/top views
    pub right_views: Container,
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
            left_views: Container::new(),
            right_views: Container::new(),
            ratio: 0.0,
            direction: direction,
            handle: handle,
            rect: Rect::default(),
        }
    }

    pub fn no_split(&mut self, direction: Direction, view_handle: ViewHandle) -> bool {
        self.direction = direction;

        if self.left_views.views.len() == 0 {
            self.left_views.views.push(View::new(view_handle));
            self.ratio = 0.5;
            return true;
        }

        if self.right_views.views.len() == 0 {
            self.right_views.views.push(View::new(view_handle));
            self.ratio = 0.5;
            return true;
        }

        false
    }

    pub fn split_left(&mut self, split_handle: SplitHandle, view_handle: ViewHandle, direction: Direction) {
        if Self::no_split(self, direction, view_handle) { 
            return; 
        } else {
            let mut split = Box::new(Split::new(direction, split_handle));
            split.right_views = self.left_views.clone();
            split.left_views.views.push(View::new(view_handle));
            split.ratio = 0.5;
            self.left = Some(split);
            self.left_views.views.clear();
        }
    }

    pub fn split_right(&mut self, split_handle: SplitHandle, view_handle: ViewHandle, direction: Direction) {
        if Self::no_split(self, direction, view_handle) { 
            return; 
        } else {
            let mut split = Box::new(Split::new(direction, split_handle));
            split.left_views = self.right_views.clone();
            split.right_views.views.push(View::new(view_handle));
            split.ratio = 0.5;
            self.right = Some(split);
            self.right_views.views.clear();
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

        self.left_views.rect = rects.0;
        self.right_views.rect = rects.1;

        //println!("level {} left  - x: {} y: {} w: {} h: {}", level, rects.0.x, rects.0.y, rects.0.width, rects.0.height);
        //println!("level {} right - x: {} y: {} w: {} h: {}", level, rects.1.x, rects.1.y, rects.1.width, rects.1.height);

        // TODO: Remove these loops, should be propagated to update call only

        for view in &mut self.left_views.views {
            //println!("level {} left  - x: {} y: {} w: {} h: {}", level, rects.0.x, rects.0.y, rects.0.width, rects.0.height);
            view.rect = rects.0;
        }

        for view in &mut self.right_views.views {
            //println!("level {} right - x: {} y: {} w: {} h: {}", level, rects.1.x, rects.1.y, rects.1.width, rects.1.height);
            view.rect = rects.1;
        }
    }

    pub fn split_by_view_handle(&mut self, direction: Direction, split_handle: SplitHandle, find_handle: ViewHandle, handle: ViewHandle) {
        // TODO: Fix me
        let left_views = self.left_views.views.clone();
        let right_views = self.right_views.views.clone();

        for view in left_views {
            if view.handle.0 == find_handle.0 {
                self.split_left(split_handle, handle, direction);
                return;
            }
        }

        for view in right_views {
            if view.handle.0 == find_handle.0 {
                self.split_right(split_handle, handle, direction);
                return;
            }
        }

        if let Some(ref mut split) = self.left {
            Self::split_by_view_handle(split, direction, split_handle, find_handle, handle); 
        }

        if let Some(ref mut split) = self.right {
            Self::split_by_view_handle(split, direction, split_handle, find_handle, handle); 
        }
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
        if Self::is_hovering_rect(pos, 8.0, self.left_views.rect, self.direction) {
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

    pub fn get_rect_by_handle(&self, handle: ViewHandle) -> Option<Rect> {
        for h in &self.left_views.views {
            if h.0 == handle.0 {
                return Some(h.rect);
            }
        }

        for h in &self.right_views.views {
            if h.0 == handle.0 {
                return Some(h.rect);
            }
        }

        if let Some(ref split) = self.left {
            if let Some(handle) = Self::get_rect_by_handle(handle) {
                return Some(handle);
            }
        }

        if let Some(ref split) = self.right {
            if let Some(handle) = Self::get_rect_by_handle(handle) {
                return Some(handle);
            }
        }
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

    /// This code gets called when the top split is None. This mean that the view will be
    /// set to fullscreen as there are no other splits to be done
    fn split_new(&mut self, split_handle: SplitHandle, view_handle: ViewHandle) {
        let mut split = Box::new(Split::new(Direction::Full, split_handle));
        split.ratio = 1.0;
        split.left_views.views.push(View::new(view_handle));
        self.split = Some(split);
    }

    pub fn split_top(&mut self, view_handle: ViewHandle, direction: Direction) {
        self.handle_counter.0 += 1;
        let split_handle = self.handle_counter;
        if let Some(ref mut split) = self.split {
            split.split_left(split_handle, view_handle, direction);
        } else {
            Self::split_new(self, split_handle, view_handle);
        }
    }

    pub fn update(&mut self) {
        let rect = self.rect.clone();
        if let Some(ref mut split) = self.split {
            split.recursive_update(rect, 0);
        }
    }

    pub fn split_by_view_handle(&mut self, direction: Direction, find_handle: ViewHandle, handle: ViewHandle) {
        if let Some(ref mut split) = self.split {
            self.handle_counter.0 += 1;
            split.split_by_view_handle(direction, self.handle_counter, find_handle, handle);
        }
    }

    pub fn is_hovering_sizer(&self, pos: (f32, f32)) -> Option<SplitHandle> {
        if let Some(ref split) = self.split {
            split.is_hovering_sizer(pos)
        } else {
            None
        }
    }

    pub fn drag_sizer(&mut self, handle: SplitHandle, delta: (f32, f32)) {
        if let Some(ref mut split) = self.split {
            split.drag_sizer(handle, delta)
        }
    }

    pub fn get_rect_by_handle(&self, handle: ViewHandle) -> Option<Rect> {
        if let Some(ref split) = self.split {
            split.get_rect_by_handle(handle)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod test {
    use {Split, Workspace, Rect, ViewHandle, Direction};

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
        ws.split_top(ViewHandle(1), Direction::Vertical);

        assert_eq!(ws.split.is_some(), true);
        let split = ws.split.unwrap();

        assert_eq!(split.left_views.views.len(), 1);
    }

    #[test]
    fn test_split_top_2() {
        let mut ws = Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).unwrap();
        ws.split_top(ViewHandle(1), Direction::Vertical);
        ws.split_top(ViewHandle(2), Direction::Vertical);

        assert_eq!(ws.split.is_some(), true);
        let split = ws.split.unwrap();

        assert_eq!(split.right_views.views.len(), 1);
        assert_eq!(split.left_views.views.len(), 1);
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
