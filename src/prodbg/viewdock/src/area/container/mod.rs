mod serialize;

use dock::{Dock, DockHandle};
use rect::Rect;
use super::super::ItemTarget;

#[derive(Debug, Clone)]
pub struct Container {
    pub docks: Vec<Dock>,
    pub tab_sizes: Vec<f32>,
    pub rect: Rect,
    pub active_dock: usize,
}

impl Container {
    pub fn new(dock: Dock, rect: Rect) -> Container {
        Container {
            docks: vec!(dock),
            tab_sizes: vec!(0.0),
            rect: rect,
            active_dock: 0,
        }
    }

    pub fn append_dock(&mut self, dock: Dock) {
        self.docks.push(dock);
        self.tab_sizes.push(0.0);
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
            self.tab_sizes.remove(index);
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
        self.tab_sizes.insert(index, 1.0);
    }

    fn get_header_rect(&self) -> Rect {
        Rect::new(self.rect.x, self.rect.y, self.rect.width - 30.0, 30.0)
    }

    fn get_tab_rects(&self) -> Vec<Rect> {
        let mut total_width = 0.0;
        return self.tab_sizes.iter()
            .map(|size| {
                let res = Rect::new(self.rect.x + total_width, self.rect.y + 30.0, *size, 30.0);
                total_width += *size;
                return res;
            })
            .collect();
    }

    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
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

    pub fn get_item_target_at_pos(&self, pos: (f32, f32)) -> Option<ItemTarget> {
        if self.docks.len() == 1 {
            if self.get_header_rect().point_is_inside(pos) {
                return Some(ItemTarget::AppendToContainer(self.docks[0].handle, 1));
            }
        }
        let mut total_width = 0.0;
        for (index, size) in self.tab_sizes.iter().enumerate() {
            let item_pos = Rect::new(self.rect.x + total_width - 40.0, self.rect.y + 30.0, 80.0, 30.0);
            if item_pos.point_is_inside(pos) {
                return Some(ItemTarget::AppendToContainer(self.docks[0].handle, index));
            }
            total_width += *size;
        }
        let item_pos = Rect::new(self.rect.x + total_width - 40.0, self.rect.y + 30.0, 80.0, 30.0);
        if item_pos.point_is_inside(pos) {
            return Some(ItemTarget::AppendToContainer(self.docks[0].handle, self.tab_sizes.len()));
        }
        return None;
    }

    pub fn update_tab_sizes(&mut self, sizes: &[f32]) {
        // TODO: store tab positions instead of tab sizes
        if sizes.len() == self.docks.len() {
            for (mut size, new_size) in self.tab_sizes.iter_mut().zip(sizes) {
                *size = *new_size;
            }
        } else {
            // TODO: do we really need to panic here?
            panic!("Wrong tab sizes! Expected {}, but got {}", self.docks.len(), sizes.len());
        }
    }
}

#[cfg(test)]
mod test {
    extern crate serde_json;
    use {Container, Dock, DockHandle, Rect};

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
}