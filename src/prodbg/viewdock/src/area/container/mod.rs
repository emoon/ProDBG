mod serialize;

use dock::{Dock, DockHandle};
use rect::{Rect, Direction};
use super::super::ItemTarget;

#[derive(Debug, Clone)]
pub struct Container {
    pub docks: Vec<Dock>,
    pub tab_borders: Vec<f32>,
    pub rect: Rect,
    pub active_dock: usize,
}

const HEADER_HEIGHT: f32 = 26.0;
const TAB_HEIGHT: f32 = 22.0;
const CLOSE_BUTTON_WIDTH: f32 = 30.0;
const TAB_INSERT_WIDTH: f32 = 80.0;

impl Container {
    pub fn new(dock: Dock, rect: Rect) -> Container {
        Container {
            docks: vec!(dock),
            tab_borders: vec!(0.0),
            rect: rect,
            active_dock: 0,
        }
    }

    pub fn append_dock(&mut self, dock: Dock) {
        self.docks.push(dock);
        self.tab_borders.push(0.0);
        self.active_dock = self.docks.len() - 1;
    }

    pub fn has_dock(&self, handle: DockHandle) -> bool {
        self.docks.iter()
            .find(|&dock| dock.handle == handle)
            .is_some()
    }

    pub fn get_dock_mut(&mut self, handle: DockHandle) -> Option<&mut Dock> {
        self.docks.iter_mut().find(|dock| dock.handle == handle)
    }

    pub fn remove_dock(&mut self, handle: DockHandle) {
        if let Some(index) = self.docks.iter().position(|dock| dock.handle == handle) {
            self.docks.remove(index);
            self.tab_borders.remove(index);
            if self.active_dock > 0 {
                self.active_dock = self.active_dock - 1;
            }
        }
    }

    pub fn replace_dock(&mut self, handle: DockHandle, new_dock: Dock) -> Option<Dock> {
        self.docks.iter()
            .position(|dock| dock.handle == handle)
            .and_then(|index| {
                self.docks.push(new_dock);
                Some(self.docks.swap_remove(index))
            })
    }

    pub fn insert_dock(&mut self, index: usize, dock: Dock) {
        self.docks.insert(index, dock);
        self.tab_borders.insert(index, 1.0);
    }

    fn get_header_rect(&self) -> Rect {
        Rect::new(self.rect.x, self.rect.y, self.rect.width - CLOSE_BUTTON_WIDTH, HEADER_HEIGHT)
    }

    fn get_tab_rects(&self) -> Vec<Rect> {
        let mut left_border = 0.0;
        return self.tab_borders.iter()
            .map(|&right_border| {
                let width = right_border - left_border;
                let res = Rect::new(self.rect.x + left_border, self.rect.y + HEADER_HEIGHT, width, TAB_HEIGHT);
                left_border = right_border;
                return res;
            })
            .collect();
    }

    pub fn get_dock_handle_with_header_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        if self.get_header_rect().point_is_inside(pos) {
            return Some(self.docks[self.active_dock].handle);
        }
        if self.docks.len() > 1 {
            return self.get_tab_rects().iter().enumerate()
                .find(|&(_, rect)| rect.point_is_inside(pos))
                .and_then(|(i, _)| Some(self.docks[i].handle));
        }
        return None;
    }

    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        if self.rect.point_is_inside(pos) {
            return Some(self.docks[self.active_dock].handle);
        } else {
            return None;
        }
    }

    pub fn get_item_target_at_pos(&self, pos: (f32, f32)) -> Option<(ItemTarget, Rect)> {
        for (index, &border) in [0.0].iter().chain(self.tab_borders.iter()).enumerate() {
            let gap_rect = Rect::new(
                self.rect.x + border - TAB_INSERT_WIDTH / 2.0,
                self.rect.y + HEADER_HEIGHT,
                TAB_INSERT_WIDTH,
                TAB_HEIGHT);
            if gap_rect.point_is_inside(pos) {
                return Some((ItemTarget::AppendToContainer(self.docks[0].handle, index), gap_rect));
            }
        }
        let w3 = self.rect.width / 3.0;
        let h3 = self.rect.height / 3.0;
        let mid = Rect::new(self.rect.x + w3, self.rect.y + h3, w3, h3);
        if mid.point_is_inside(pos) {
            return Some((ItemTarget::AppendToContainer(self.docks[0].handle, self.docks.len()), mid));
        }
        for &(dist, over_dist, direction) in [(w3, self.rect.width/2.0, Direction::Horizontal), (h3, self.rect.height/2.0, Direction::Vertical)].iter() {
            for &(mult, place) in [(-1.0, 0), (1.0, 1)].iter() {
                let place_rect = mid.shifted(direction, dist * mult);
                let over_rect = self.rect.shifted_clip(direction, over_dist * mult);
                if place_rect.point_is_inside(pos) {
                    return Some((ItemTarget::SplitDock(self.docks[0].handle, direction.opposite(), place), over_rect));
                }
            }
        }
        return None;
    }

    pub fn update_tab_borders(&mut self, sizes: &[f32]) {
        for (mut size, new_size) in self.tab_borders.iter_mut().zip(sizes) {
            *size = *new_size;
        }
    }
}

#[cfg(test)]
mod test {
    extern crate serde_json;
    use super::Container;
    use dock::{Dock, DockHandle};
    use rect::Rect;

    #[test]
    fn test_container_serialize_0() {
        let container_in = Container {
            docks: Vec::new(),
            active_dock: 0,
            tab_borders: Vec::new(),
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
            }],
            tab_borders: vec!(0.0),
            active_dock: 0,
            rect: Rect::default(),
        };

        let serialized = serde_json::to_string(&container_in).unwrap();
        let container_out: Container = serde_json::from_str(&serialized).unwrap();

        assert_eq!(container_out.docks.len(), 1);
        assert_eq!(container_out.docks[0].plugin_name, "registers");
    }
}