mod serialize;

use dock::{Dock, DockHandle};
use rect::Rect;
use super::{DragTarget, DropTarget};

/// Holds a list of available docks
#[derive(Debug, Clone)]
pub struct Container {
    /// Docks this container. The reason of supporting several docks here is that this can be used
    /// to implement tabs but only one dock should be visible at a time
    pub docks: Vec<Dock>,
    pub rect: Rect,
    pub active_dock: usize,
}

impl Container {
    pub fn new(dock: Dock, rect: Rect) -> Container {
        Container {
            docks: vec!(dock),
            rect: rect,
            active_dock: 0,
        }
    }

    pub fn find_dock(&self, handle: DockHandle) -> Option<&Dock> {
        self.docks.iter().find(|&dock| dock.handle == handle)
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
            Some(DragTarget::Dock(self.docks.get(self.active_dock).unwrap().handle))
        } else {
            None
        }
    }

    pub fn get_drop_target_at_pos(&self, pos: (f32, f32)) -> Option<DropTarget> {
        return if self.get_header_rect().point_is_inside(pos) {
            Some(DropTarget::Dock(self.docks.get(self.active_dock).unwrap().handle))
        } else {
            None
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