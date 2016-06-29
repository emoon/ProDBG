mod container;
mod split;
mod serialize;

use rect::{Rect, Direction};
use dock::DockHandle;
pub use self::container::Container;
pub use self::split::{SplitHandle, Split};

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


#[derive(Debug)]
pub enum DragTarget {
    SplitSizer(SplitHandle, usize, Direction),
    Dock(DockHandle)
}

#[derive(Debug)]
pub enum DropTarget {
    Dock(DockHandle)
}


#[cfg(test)]
mod test {
    extern crate serde_json;

    use {Area};
    use super::container::Container;
    use super::split::{Split, SplitHandle};
    use dock::{Dock, DockHandle};
    use rect::{Rect, Direction};

    #[test]
    fn test_area_serialize() {
        let split = Area::Split(Split::from_two(
            Direction::Horizontal,
            0.7,
            SplitHandle(513),
            Rect::new(17.0, 15.0, 100.0, 159.0),
            Area::Container(Container::new(Dock::new(DockHandle(14), "test"), Rect::default())),
            Area::Container(Container::new(Dock::new(DockHandle(15), "test2"), Rect::default()))
        ));
        let container = Area::Container(Container::new(
            Dock::new(DockHandle(17), "test"), Rect::new(1.0, 2.0, 3.0, 4.0)
        ));
        let split_serialized = serde_json::to_string(&split).unwrap();
        let container_serialized = serde_json::to_string(&container).unwrap();
        let split_deserialized: Area = serde_json::from_str(&split_serialized).unwrap();
        let container_deserialized: Area = serde_json::from_str(&container_serialized).unwrap();
        assert!(match split_deserialized {
            Area::Split(_) => true,
            _ => false
        });
        assert!(match container_deserialized {
            Area::Container(_) => true,
            _ => false
        });
    }
}