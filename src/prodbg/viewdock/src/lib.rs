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
mod rect;
mod area;
mod dock;
// mod serialize;
pub use self::error::Error;
// use std::io::{Write, Read};
// use std::fs::File;
//use std::io;
pub use rect::{Rect, Direction};
pub use area::{Area, Split, SplitHandle, Container, DragTarget, DropTarget};
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
        let is_root = match self.root_area {
            Some(Area::Container(ref c)) => c.find_dock(find_handle).is_some(),
            _ => false,
        };
        let new_dock = Area::Container(Container::new(Dock::new(handle), Rect::default()));
        if is_root {
            if let Some(ref mut root) = self.root_area {
                let old_root = root.clone();
                let new_child = Split::from_two(direction, 0.5, next_handle, self.rect.clone(), old_root, new_dock);
                *root = Area::Split(new_child);
                return;
            }
        }
        let parent_split = self.root_area.as_mut().and_then(|root| {
            root.find_split_by_dock_handle(find_handle)
        });
        if let Some((parent, pos)) = parent_split {
            if direction == parent.direction {
                parent.append_child(pos, new_dock);
            } else {
                let old_child = parent.children[pos].clone();
                let new_child = Split::from_two(direction, 0.5, next_handle, Rect::default(), old_child, new_dock);
                parent.replace_child(pos, Area::Split(new_child));
            }
        }
    }

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        self.root_area.as_ref().and_then(|area| {
            area.find_container_by_dock_handle(handle).and_then(|container| {
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
            if let Some(s) = root.find_split_by_handle(handle) {
                s.change_ratio(index, delta);
            }
        }
    }

    pub fn get_drag_target_at_pos(&self, pos: (f32, f32)) -> Option<DragTarget> {
        self.root_area.as_ref().and_then(|root| {
            root.get_drag_target_at_pos(pos)
        })
    }

    pub fn get_drop_target_at_pos(&self, pos: (f32, f32)) -> Option<DropTarget> {
        self.root_area.as_ref().and_then(|root| {
            root.get_drop_target_at_pos(pos)
        })
    }

    pub fn delete_by_handle(&mut self, handle: DockHandle) {
        let mut should_delete_root = false;
        if let Some(Area::Container(ref c)) = self.root_area {
            should_delete_root = c.find_dock(handle).is_some();
        }
        if should_delete_root {
            self.root_area = None;
            return;
        }
        if let Some(ref mut root) = self.root_area {
            let mut should_adopt = None;
            if let Some((split, index)) = root.find_split_by_dock_handle(handle) {
                split.remove_child(index);
                if split.children.len() == 1 {
                    should_adopt = Some((split.handle, split.children[0].clone()));
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
                } else if let Some((parent_split, index)) = root.find_parent_split_by_split_handle(split_handle) {
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

    pub fn swap_docks(&mut self, first: DockHandle, second: DockHandle) {
        if let Some(ref mut root) = self.root_area {
            if first == second {
                return;
            }
            let first_copy;
            let second_copy;
            let first_split_handle;
            let first_dock_index;
            match root.find_split_by_dock_handle(first) {
                None => return,
                Some((parent, index)) => {
                    first_copy = parent.children[index].clone();
                    first_dock_index = index;
                    first_split_handle = parent.handle;
                }
            }
            match root.find_split_by_dock_handle(second) {
                None => return,
                Some((parent, index)) => second_copy = parent.replace_child(index, first_copy),
            }
            match root.find_split_by_handle(first_split_handle) {
                None => panic!("Tried to swap docks {:?} and {:?} but lost {:?} in the middle", first, second, first),
                Some(s) => {s.replace_child(first_dock_index, second_copy);},
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
//    extern crate serde_json;
//
//    use {Container, Split, Workspace, Dock, Rect, DockHandle, SplitHandle, Direction};

//    #[test]
//    fn test_inside_horizontal() {
//        let border_size = 4.0;
//        let rect = Rect::new(10.0, 20.0, 30.0, 40.0);
//        let rect_horz = Split::get_sizer_from_rect_horizontal(rect, border_size);
//
//        assert_eq!(Rect::is_inside((9.0, 61.0), rect_horz), false);
//        assert_eq!(Rect::is_inside((11.0, 61.0), rect_horz), true);
//    }

//    #[test]
//    fn test_dockhandle_serialize() {
//        let handle_in = DockHandle(0x1337);
//        let serialized = serde_json::to_string(&handle_in).unwrap();
//        let handle_out: DockHandle = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(handle_in, handle_out);
//    }
//
//    #[test]
//    fn test_splithandle_serialize() {
//        let handle_in = SplitHandle(0x4422);
//        let serialized = serde_json::to_string(&handle_in).unwrap();
//        let handle_out: SplitHandle = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(handle_in, handle_out);
//    }
//
//    #[test]
//    fn test_dock_serialize_0() {
//        let dock_in = Dock {
//            handle: DockHandle(1),
//            plugin_name: "disassembly".to_owned(),
//            plugin_data: None,
//            rect: Rect::new(1.0, 2.0, 3.0, 4.0)
//        };
//
//        let serialized = serde_json::to_string(&dock_in).unwrap();
//        let dock_out: Dock = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(dock_in.handle, dock_out.handle);
//        assert_eq!(dock_in.plugin_name, dock_out.plugin_name);
//        assert_eq!(dock_in.plugin_data, dock_out.plugin_data);
//        // expect that rect is not serialized and set to zero
//        assert_eq!(dock_out.rect.x as i32, 0);
//        assert_eq!(dock_out.rect.y as i32, 0);
//        assert_eq!(dock_out.rect.width as i32, 0);
//        assert_eq!(dock_out.rect.height as i32, 0);
//    }
//
//    #[test]
//    fn test_dock_serialize_1() {
//        let dock_in = Dock {
//            handle: DockHandle(1),
//            plugin_name: "registers".to_owned(),
//            plugin_data: Some(vec!["some_data".to_owned(), "more_data".to_owned()]),
//            rect: Rect::new(4.0, 5.0, 2.0, 8.0)
//        };
//
//        let serialized = serde_json::to_string(&dock_in).unwrap();
//        let dock_out: Dock = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(dock_in.handle, dock_out.handle);
//        assert_eq!(dock_in.plugin_name, dock_out.plugin_name);
//        assert_eq!(dock_in.plugin_data, dock_out.plugin_data);
//
//        // expect that rect is not serialized and set to zero
//        assert_eq!(dock_out.rect.x as i32, 0);
//        assert_eq!(dock_out.rect.y as i32, 0);
//        assert_eq!(dock_out.rect.width as i32, 0);
//        assert_eq!(dock_out.rect.height as i32, 0);
//
//        let plugin_data = dock_out.plugin_data.as_ref().unwrap();
//
//        assert_eq!(plugin_data.len(), 2);
//        assert_eq!(plugin_data[0], "some_data");
//        assert_eq!(plugin_data[1], "more_data");
//    }
//
//    #[test]
//    fn test_container_serialize_0() {
//        let container_in = Container {
//            docks: Vec::new(),
//            rect: Rect::new(4.0, 5.0, 2.0, 8.0)
//        };
//
//        let serialized = serde_json::to_string(&container_in).unwrap();
//        let container_out: Container = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(container_out.docks.len(), 0);
//        // expect that rect is not serialized and set to zero
//        assert_eq!(container_out.rect.x as i32, 0);
//        assert_eq!(container_out.rect.y as i32, 0);
//        assert_eq!(container_out.rect.width as i32, 0);
//        assert_eq!(container_out.rect.height as i32, 0);
//    }
//
//    #[test]
//    fn test_container_serialize_1() {
//        let container_in = Container {
//            docks: vec![Dock {
//                handle: DockHandle(1),
//                plugin_name: "registers".to_owned(),
//                plugin_data: Some(vec!["some_data".to_owned(), "more_data".to_owned()]),
//                rect: Rect::new(4.0, 5.0, 2.0, 8.0)
//            }],
//            rect: Rect::default(),
//        };
//
//        let serialized = serde_json::to_string(&container_in).unwrap();
//        let container_out: Container = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(container_out.docks.len(), 1);
//        assert_eq!(container_out.docks[0].plugin_name, "registers");
//    }
//
//    #[test]
//    fn test_split_serialize_0() {
//        let split_in = Split {
//            left: None,
//            right: None,
//            left_docks: Container::new(),
//            right_docks: Container::new(),
//            ratio: 0.7,
//            direction: Direction::Full,
//            handle: SplitHandle(1),
//            rect: Rect::new(4.0, 5.0, 2.0, 8.0)
//        };
//
//        let serialized = serde_json::to_string(&split_in).unwrap();
//        let split_out: Split = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(split_in.left, split_out.left);
//        assert_eq!(split_in.right, split_out.right);
//        assert_eq!(split_in.direction, split_out.direction);
//        assert_eq!(split_in.handle, split_out.handle);
//
//        // expect that rect is not serialized and set to zero
//        assert_eq!(split_out.rect.x as i32, 0);
//        assert_eq!(split_out.rect.y as i32, 0);
//        assert_eq!(split_out.rect.width as i32, 0);
//        assert_eq!(split_out.rect.height as i32, 0);
//    }
//
//    #[test]
//    fn test_direction_serialize() {
//        let dir_in_0 = Direction::Horizontal;
//        let dir_in_1 = Direction::Full;
//        let dir_in_2 = Direction::Vertical;
//
//        let s0 = serde_json::to_string(&dir_in_0).unwrap();
//        let s1 = serde_json::to_string(&dir_in_1).unwrap();
//        let s2 = serde_json::to_string(&dir_in_2).unwrap();
//
//        let dir_out_0: Direction = serde_json::from_str(&s0).unwrap();
//        let dir_out_1: Direction = serde_json::from_str(&s1).unwrap();
//        let dir_out_2: Direction = serde_json::from_str(&s2).unwrap();
//
//        assert_eq!(dir_in_0, dir_out_0);
//        assert_eq!(dir_in_1, dir_out_1);
//        assert_eq!(dir_in_2, dir_out_2);
//    }
//
//    #[test]
//    fn test_workspace_serialize_0() {
//        let ws_in = Workspace {
//            splits: Vec::new(),
//            rect: Rect::new(4.0, 5.0, 2.0, 8.0),
//            window_border: 6.0,
//            handle_counter: SplitHandle(2),
//        };
//
//        let serialized = serde_json::to_string(&ws_in).unwrap();
//        let ws_out: Workspace = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(ws_out.splits.len(), 0);
//        assert_eq!(ws_out.window_border as i32, 6);
//        assert_eq!(ws_out.rect.x as i32, 4);
//        assert_eq!(ws_out.rect.y as i32, 5);
//        assert_eq!(ws_out.rect.width as i32, 2);
//        assert_eq!(ws_out.rect.height as i32, 8);
//    }
//
//    #[test]
//    fn test_workspace_serialize_1() {
//        let ws_in = Workspace {
//            splits: vec![Split {
//                left: None,
//                right: None,
//                left_docks: Container::new(),
//                right_docks: Container::new(),
//                ratio: 0.7,
//                direction: Direction::Full,
//                handle: SplitHandle(1),
//                rect: Rect::new(4.0, 5.0, 2.0, 8.0)
//            }],
//            rect: Rect::new(4.0, 5.0, 2.0, 8.0),
//            window_border: 6.0,
//            handle_counter: SplitHandle(2),
//        };
//
//        let serialized = serde_json::to_string(&ws_in).unwrap();
//        let ws_out: Workspace = serde_json::from_str(&serialized).unwrap();
//
//        assert_eq!(ws_out.splits.len(), 1);
//        assert_eq!(ws_out.splits[0].handle.0, 1);
//    }
}
