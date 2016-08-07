use serde;
use {DockHandle, ItemTarget};
use rect::{Direction, Rect};

/// `Container` shares its area (defined by `rect`) between its child `docks`. `active_dock` is an
/// index of dock that gets all the area, while others only draw their headers in tabs, defined by
/// `tab_borders`. Since drawing of tabs happen outside of Viewdock, `tab_borders` should be updated
/// each time they are changed.
#[derive(Debug, Clone)]
pub struct Container {
    /// Child docks
    pub docks: Vec<DockHandle>,
    /// Right border of tab in pixels for each dock in this container
    pub tab_borders: Vec<f32>,
    /// Area occupied by this `Container`
    pub rect: Rect,
    /// Index of active dock. It means header and all the area except tabs belong to this dock.
    pub active_dock: usize,
}

/// Height of header in pixels
const HEADER_HEIGHT: f32 = 26.0;
/// Height of tabs in pixels
const TAB_HEIGHT: f32 = 22.0;
/// Width of close button in header. Area of close button is excluded from header.
const CLOSE_BUTTON_WIDTH: f32 = 30.0;
/// Width of area between tabs available for dropping
const TAB_INSERT_WIDTH: f32 = 80.0;

impl Container {
    pub fn new(dock_handle: DockHandle, rect: Rect) -> Container {
        Container {
            docks: vec![dock_handle],
            tab_borders: vec![0.0],
            rect: rect,
            active_dock: 0,
        }
    }

    /// Appends new dock to this container. This dock becomes active.
    pub fn append_dock(&mut self, handle: DockHandle) {
        self.docks.push(handle);
        self.tab_borders.push(0.0);
        self.active_dock = self.docks.len() - 1;
    }

    /// Returns `true`, if this `handle` dock is in this container
    pub fn has_dock(&self, handle: DockHandle) -> bool {
        self.docks
            .iter()
            .any(|&dock_handle| dock_handle == handle)
    }

    /// Removes dock handle if it is in this container. If removed dock was active, then
    /// next dock in list becomes active. If removed dock was last, previous dock becomes active.
    pub fn remove_dock(&mut self, handle: DockHandle) {
        if let Some(index) = self.docks.iter().position(|&dock_handle| dock_handle == handle) {
            self.docks.remove(index);
            self.tab_borders.remove(index);
            if self.active_dock > 0 {
                self.active_dock = self.active_dock - 1;
            }
        }
    }

    /// Replaces dock handle with new one. Returns replaced dock handle. Does nothing and returns None if `handle`
    /// is not in this container.
    pub fn replace_dock(&mut self, target: DockHandle, handle: DockHandle) -> Option<DockHandle> {
        self.docks
            .iter_mut()
            .find(|&&mut dock_handle| dock_handle == target)
            .map(|dock_handle| {
                *dock_handle = handle;
                target
            })
    }

    /// Inserts dock in supplied position.
    pub fn insert_dock(&mut self, index: usize, handle: DockHandle) {
        self.docks.insert(index, handle);
        self.tab_borders.insert(index, 1.0);
        if self.active_dock >= index {
            self.active_dock += 1;
        }
    }

    /// Returns header area for docks in this container (without close button)
    fn get_header_rect(&self) -> Rect {
        Rect::new(self.rect.x,
                  self.rect.y,
                  self.rect.width - CLOSE_BUTTON_WIDTH,
                  HEADER_HEIGHT)
    }

    /// Returns tab areas for docks in this container
    fn get_tab_rects(&self) -> Vec<Rect> {
        let mut left_border = 0.0;
        return self.tab_borders
            .iter()
            .map(|&right_border| {
                let width = right_border - left_border;
                let res = Rect::new(self.rect.x + left_border,
                                    self.rect.y + HEADER_HEIGHT,
                                    width,
                                    TAB_HEIGHT);
                left_border = right_border;
                return res;
            })
            .collect();
    }

    /// Returns `Some(DockHandle)` which header or tab is at `pos`.
    pub fn get_dock_handle_with_header_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        if self.get_header_rect().point_is_inside(pos) {
            return Some(self.docks[self.active_dock]);
        }
        if self.docks.len() > 1 {
            return self.get_tab_rects()
                .iter()
                .enumerate()
                .find(|&(_, rect)| rect.point_is_inside(pos))
                .and_then(|(i, _)| Some(self.docks[i]));
        }
        return None;
    }

    /// Returns handle of active dock if `pos` is inside this container's rect.
    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        if self.rect.point_is_inside(pos) {
            return Some(self.docks[self.active_dock]);
        } else {
            return None;
        }
    }

    /// Returns `ItemTarget` corresponding to `pos`. See `ItemTarget` documentation for more.
    pub fn get_item_target_at_pos(&self, pos: (f32, f32)) -> Option<(ItemTarget, Rect)> {
        // check special place inside tabs to change tab order
        for (index, &border) in [0.0].iter().chain(self.tab_borders.iter()).enumerate() {
            let gap_rect = Rect::new(self.rect.x + border - TAB_INSERT_WIDTH / 2.0,
                                     self.rect.y + HEADER_HEIGHT,
                                     TAB_INSERT_WIDTH,
                                     TAB_HEIGHT);
            if gap_rect.point_is_inside(pos) {
                return Some((ItemTarget::AppendToContainer(self.docks[0], index), gap_rect));
            }
        }

        // check middle box area to inject new tab
        let w3 = self.rect.width / 3.0;
        let h3 = self.rect.height / 3.0;
        let mid = Rect::new(self.rect.x + w3, self.rect.y + h3, w3, h3);
        if mid.point_is_inside(pos) {
            return Some((ItemTarget::AppendToContainer(self.docks[0], self.docks.len()),
                         self.rect));
        }

        if !self.rect.point_is_inside(pos) {
            return None;
        }

        // calc closest distance to border
        let mut res = None;
        let x = self.rect.x;
        let y = self.rect.y;
        let w = self.rect.width;
        let h = self.rect.height;
        let mut min = 0.0;
        for &(add_x, add_y, over_dist, direction, place) in [(0.0,
                                                              0.0,
                                                              -w / 2.0,
                                                              Direction::Horizontal,
                                                              0),
                                                             (w,
                                                              0.0,
                                                              w / 2.0,
                                                              Direction::Horizontal,
                                                              1),
                                                             (0.0,
                                                              0.0,
                                                              -h / 2.0,
                                                              Direction::Vertical,
                                                              0),
                                                             (0.0,
                                                              h,
                                                              h / 2.0,
                                                              Direction::Vertical,
                                                              1)]
            .iter() {
            let dist = match direction {
                Direction::Horizontal => (pos.0 - (x + add_x)).abs(),
                Direction::Vertical => (pos.1 - (y + add_y)).abs(),
            };
            if min == 0.0 || dist < min {
                let over_rect = self.rect.shifted_clip(direction, over_dist);
                res = Some((ItemTarget::SplitDock(self.docks[0], direction.opposite(), place),
                            over_rect));
                min = dist;
            }
        }
        res
    }

    /// Updates tab borders for this container
    pub fn update_tab_borders(&mut self, sizes: &[f32]) {
        for (mut size, new_size) in self.tab_borders.iter_mut().zip(sizes) {
            *size = *new_size;
        }
    }
}

// Serialization
gen_struct_code!(Container, docks, active_dock;
    tab_borders => vec!(0.0),
    rect => Rect::default()
);

#[cfg(test)]
mod test {
    extern crate serde_json;
    use Container;
    use DockHandle;
    use rect::Rect;

    fn get_test_container() -> Container {
        Container::new(DockHandle(5), Rect::new(20.0, 20.0, 100.0, 200.0))
    }

    #[test]
    fn test_append_dock() {
        let mut container = get_test_container();
        container.append_dock(DockHandle(6));
        assert_eq!(2, container.docks.len());
        assert_eq!(2, container.tab_borders.len());
    }

    #[test]
    fn test_has_dock() {
        let mut container = get_test_container();
        container.append_dock(DockHandle(8));
        assert!(container.has_dock(DockHandle(8)));
        assert!(!container.has_dock(DockHandle(9)));
    }

    #[test]
    fn test_remove_dock() {
        let mut container = get_test_container();
        container.append_dock(DockHandle(8));
        container.remove_dock(DockHandle(8));
        assert!(!container.has_dock(DockHandle(8)));
        assert!(container.active_dock < container.docks.len());
    }

    #[test]
    fn test_replace_dock_existing() {
        let mut container = get_test_container();
        container.append_dock(DockHandle(8));
        let res = container.replace_dock(DockHandle(8), DockHandle(9));
        assert!(!container.has_dock(DockHandle(8)));
        assert!(container.has_dock(DockHandle(9)));
        assert_eq!(res, Some(DockHandle(8)));
    }

    #[test]
    fn test_replace_dock_non_existing() {
        let mut container = get_test_container();
        let res = container.replace_dock(DockHandle(9), DockHandle(10));
        assert!(!container.has_dock(DockHandle(10)));
        assert!(res.is_none());
    }

    #[test]
    fn test_insert_dock() {
        let mut container = get_test_container();
        container.append_dock(DockHandle(8));
        container.active_dock = 1;
        container.insert_dock(1, DockHandle(9));
        assert_eq!(container.docks[1], DockHandle(9));
        assert_eq!(container.active_dock, 2);
        container.active_dock = 1;
        container.insert_dock(2, DockHandle(10));
        assert_eq!(container.docks[2], DockHandle(10));
        assert_eq!(container.active_dock, 1);
    }

    #[test]
    fn test_get_tab_rects() {
        let mut container = get_test_container();
        container.append_dock(DockHandle(8));
        assert_eq!(2, container.get_tab_rects().len());
    }
}
