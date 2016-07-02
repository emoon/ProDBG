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
#[macro_use]
mod serialize_helper;
mod error;
mod rect;
mod area;
mod dock;
mod serialize;

pub use self::error::Error;
// use std::io::{Write, Read};
// use std::fs::File;
//use std::io;
pub use rect::{Rect, Direction};
pub use area::{Area, SplitHandle, Container, SizerPos};
use area::Split;
pub use dock::{DockHandle, Dock};

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

#[derive(Debug)]
pub enum ItemTarget {
    // TODO: use (Direction, bool) instead
    /// Direction of split and position (0 for first child, 1 for second child)
    SplitRoot(Direction, usize),
    /// Handle of split to change and index of new Dock
    AppendToSplit(SplitHandle, usize),
    /// SplitHandle, usize define container. usize defines position in split (0 for first child,
    /// 1 for second child). Direction is opposite to direction of parent split
    SplitContainer(SplitHandle, usize, usize),
    /// DockHandle defines container. usize defines index of new Dock
    AppendToContainer(DockHandle, usize)
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

    fn next_handle(&mut self) -> SplitHandle {
        self.handle_counter.0 += 1;
        self.handle_counter
    }

    pub fn initialize(&mut self, dock: Dock) {
        self.root_area = Some(Area::Container(
            Container::new(
                dock,
                self.rect.clone()
            )
        ));
    }

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        self.root_area.as_ref().and_then(|area| {
            area.get_container_by_dock_handle(handle).and_then(|container| {
                Some(container.rect.clone())
            })
        })
    }

    pub fn get_hover_dock(&self, pos: (f32, f32)) -> Option<DockHandle> {
        self.root_area.as_ref().and_then(|root| {
            root.get_dock_handle_at_pos(pos)
        })
    }

    pub fn update(&mut self, new_rect: Rect) {
        self.rect = new_rect;
        if let Some(ref mut a) = self.root_area {
            a.update_rect(new_rect);
        }
    }

    pub fn drag_sizer(&mut self, handle: SplitHandle, index: usize, delta: (f32, f32)) {
        if let Some(ref mut root) = self.root_area {
            if let Some(s) = root.get_split_by_handle(handle) {
                s.change_ratio(index, delta);
            }
        }
    }

    pub fn get_sizer_at_pos(&self, pos: (f32, f32)) -> Option<SizerPos> {
        self.root_area.as_ref().and_then(|root| {
            root.get_sizer_at_pos(pos)
        })
    }

    fn get_horizontal_root_split_rects(&self) -> Vec<(Rect, ItemTarget)> {
        vec!(
            (
                Rect::new(self.rect.x, self.rect.y, self.rect.width, 100.0),
                ItemTarget::SplitRoot(Direction::Horizontal, 0)
            ),
            (
                Rect::new(self.rect.x, self.rect.y + self.rect.height - 100.0, self.rect.width, 100.0),
                ItemTarget::SplitRoot(Direction::Horizontal, 1)
            ),
        )
    }

    fn get_vertical_root_split_rects(&self) -> Vec<(Rect, ItemTarget)> {
        vec!(
            (
                Rect::new(self.rect.x, self.rect.y, 100.0, self.rect.height),
                ItemTarget::SplitRoot(Direction::Vertical, 0)
            ),
            (
                Rect::new(self.rect.x + self.rect.width - 100.0, self.rect.y, 100.0, self.rect.height),
                ItemTarget::SplitRoot(Direction::Vertical, 1)
            )
        )
    }

    pub fn get_item_target_at_pos(&self, pos: (f32, f32)) -> Option<(ItemTarget, Rect)> {
        self.root_area.as_ref().and_then(|root| {
            root.get_item_target_at_pos(pos)
                .or_else(||
                    match *root {
                        Area::Container(_) => {
                            for (rect, target) in self.get_horizontal_root_split_rects() {
                                if rect.point_is_inside(pos) {
                                    return Some((target, rect));
                                }
                            }
                            for (rect, target) in self.get_vertical_root_split_rects() {
                                if rect.point_is_inside(pos) {
                                    return Some((target, rect));
                                }
                            }
                            return None;
                        },
                        Area::Split(ref s) => {
                            match s.direction {
                                Direction::Horizontal => {
                                    for (rect, target) in self.get_vertical_root_split_rects() {
                                        if rect.point_is_inside(pos) {
                                            return Some((target, rect));
                                        }
                                    }
                                },
                                Direction::Vertical => {
                                    for (rect, target) in self.get_horizontal_root_split_rects() {
                                        if rect.point_is_inside(pos) {
                                            return Some((target, rect));
                                        }
                                    }
                                }
                            }
                            return None;
                        }
                    }
                )
        })
    }

    pub fn get_dock_handle_at_pos(&self, pos:(f32, f32)) -> Option<DockHandle> {
        self.root_area.as_ref().and_then(|root| {
            root.get_dock_handle_at_pos(pos)
        })
    }

    pub fn create_dock_at(&mut self, target: ItemTarget, dock: Dock) {
        match target {
            ItemTarget::SplitRoot(direction, index) => {
                let next_handle = self.next_handle();
                if let Some(ref mut root) = self.root_area {
                    let old_root = root.clone();
                    let new_dock = Area::container_from_dock(dock);
                    let new_child = if index == 0 {
                        Split::from_two(direction, 0.5, next_handle, self.rect.clone(), new_dock, old_root)
                    } else {
                        Split::from_two(direction, 0.5, next_handle, self.rect.clone(), old_root, new_dock)
                    };
                    *root = Area::Split(new_child);
                }
            },
            ItemTarget::AppendToSplit(handle, index) => {
                if let Some(ref mut root) = self.root_area {
                    if let Some(s) = root.get_split_by_handle(handle) {
                        s.append_child(index, Area::container_from_dock(dock));
                    }
                }
            },
            ItemTarget::SplitContainer(handle, index, new_index) => {
                let next_handle = self.next_handle();
                if let Some(ref mut root) = self.root_area {
                    if let Some(s) = root.get_split_by_handle(handle) {
                        let old_copy = s.children[index].clone();
                        let new_dock = Area::container_from_dock(dock);
                        let new_child = Area::Split(if new_index == 0 {
                            Split::from_two(s.direction.opposite(), 0.5, next_handle, self.rect.clone(), new_dock, old_copy)
                        } else {
                            Split::from_two(s.direction.opposite(), 0.5, next_handle, self.rect.clone(), old_copy, new_dock)
                        });
                        s.replace_child(index, new_child);
                    }
                }
            },
            ItemTarget::AppendToContainer(handle, new_index) => {
                if let Some(ref mut root) = self.root_area {
                    if let Some(c) = root.get_container_by_dock_handle_mut(handle) {
                        c.insert_dock(new_index, dock);
                    }
                }
            }
        }
    }

    pub fn delete_dock_by_handle(&mut self, handle: DockHandle) {
        let mut should_delete_root = false;
        let mut should_stop = false;
        if let Some(Area::Container(ref mut c)) = self.root_area {
            if c.has_dock(handle) {
                c.remove_dock(handle);
                should_delete_root = c.docks.is_empty();
                should_stop = true;
            }
        }
        if should_delete_root {
            self.root_area = None;
        }
        if should_stop {
            return;
        }
        if let Some(ref mut root) = self.root_area {
            let mut should_adopt = None;
            if let Some((split, index)) = root.get_split_by_dock_handle(handle) {
                let mut should_remove_child = false;
                if let Area::Container(ref mut container) = split.children[index] {
                    container.remove_dock(handle);
                    should_remove_child = container.docks.is_empty();
                }
                if should_remove_child {
                    split.remove_child(index);
                    if split.children.len() == 1 {
                        should_adopt = Some((split.handle, split.children[0].clone()));
                    }
                }
            }
            if let Some((split_handle, mut contents)) = should_adopt {
                let mut should_replace_root = false;
                if let &mut Area::Split(ref s) = root {
                    if s.handle == split_handle {
                        should_replace_root = true;
                    }
                }
                if should_replace_root {
                    contents.update_rect(root.get_rect().clone());
                    *root = contents;
                } else if let Some((parent_split, index)) = root.get_parent_split_by_split_handle(split_handle) {
                    match contents {
                        Area::Split(ref mut s) if s.direction == parent_split.direction => {
                            parent_split.replace_child_with_children(index, &s.children)
                        },
                        _ => {parent_split.replace_child(index, contents);},
                    }
                }
            }
        }
    }

    pub fn move_dock(&mut self, handle: DockHandle, target: ItemTarget) {
        // TODO: use special constant here
        let marker = DockHandle(u64::max_value());
        let copy = self.root_area.as_mut()
            .and_then(|root| root.get_container_by_dock_handle_mut(handle))
            .and_then(|c| c.get_dock_mut(handle))
            .and_then(|dock| {
                let res = Some(dock.clone());
                dock.handle = marker;
                return res;
            });
        if let Some(dock) = copy {
            self.create_dock_at(target, dock);
            self.delete_dock_by_handle(marker);
        }
    }

    pub fn swap_docks(&mut self, first: DockHandle, second: DockHandle) {
        if let Some(ref mut root) = self.root_area {
            if first == second {
                return;
            }
            let first_copy;
            let second_copy;
            // TODO: use special constant here
            let marker = DockHandle(u64::max_value());
            match root.get_container_by_dock_handle_mut(first) {
                None => return,
                Some(c) => {
                    if let Some(ref mut dock) = c.get_dock_mut(first) {
                        first_copy = dock.clone();
                        dock.handle = marker;
                    } else {
                        return;
                    }
                }
            }
            match root.get_container_by_dock_handle_mut(second) {
                None => return,
                Some(c) => {
                    if let Some(copy) = c.replace_dock(second, first_copy) {
                        second_copy = copy;
                    } else {
                        // TODO: can we cancel previous operations here?
                        panic!();
                    }
                }
            }
            match root.get_container_by_dock_handle_mut(marker) {
                None => panic!("Tried to swap docks {:?} and {:?} but lost {:?} in the middle", first, second, first),
                Some(c) => {
                    if c.replace_dock(marker, second_copy).is_none() {
                        // TODO: can we cancel previous operations here?
                        panic!();
                    }
                },
            }
        }
    }

    pub fn save_state(&self) -> String {
        serde_json::to_string(self).unwrap()
    }

    pub fn from_state(state: &str) -> Workspace {
        serde_json::from_str(state).unwrap()
    }

    pub fn get_docks(&self) -> Vec<Dock> {
        let mut docks = Vec::new();
        match self.root_area {
            Some(ref root) => Workspace::collect_docks(&mut docks, root),
            None => {},
        };
        return docks;
    }

    fn collect_docks(target: &mut Vec<Dock>, source: &Area) {
        match *source {
            Area::Container(ref c) => {
                for dock in &c.docks {
                    target.push(dock.clone());
                }
            },
            Area::Split(ref s) => {
                for child in &s.children {
                    Workspace::collect_docks(target, child);
                }
            }
        }
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

    use {Area, Container, Workspace, Dock, Rect, DockHandle, SplitHandle};

    #[test]
    fn test_workspace_serialize_0() {
        let ws_in = Workspace {
            root_area: None,
            rect: Rect::new(4.0, 5.0, 2.0, 8.0),
            window_border: 6.0,
            handle_counter: SplitHandle(2),
        };

        let serialized = serde_json::to_string(&ws_in).unwrap();
        let ws_out: Workspace = serde_json::from_str(&serialized).unwrap();

        assert!(ws_out.root_area.is_none());
        assert_eq!(ws_out.window_border as i32, 6);
        assert_eq!(ws_out.rect.x as i32, 4);
        assert_eq!(ws_out.rect.y as i32, 5);
        assert_eq!(ws_out.rect.width as i32, 2);
        assert_eq!(ws_out.rect.height as i32, 8);
        assert_eq!(ws_out.handle_counter, SplitHandle(2));
    }

    #[test]
    fn test_workspace_serialize_1() {
        let ws_in = Workspace {
            root_area: Some(Area::Container(
                Container::new(Dock::new(DockHandle(5), "test"), Rect::default())
            )),
            rect: Rect::new(4.0, 5.0, 2.0, 8.0),
            window_border: 6.0,
            handle_counter: SplitHandle(2),
        };

        let serialized = serde_json::to_string(&ws_in).unwrap();
        let ws_out: Workspace = serde_json::from_str(&serialized).unwrap();

        assert!(match ws_out.root_area {
            Some(Area::Container(_)) => true,
            _ => false,
        });
    }
}
