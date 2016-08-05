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

extern crate serde_json;
#[macro_use]
mod serialize_helper;
mod rect;
mod area;
mod serialize;
#[cfg(test)]
mod test_helper;

pub use rect::{Direction, Rect};
pub use area::{Area, Container, SizerPos, Split, SplitHandle};

// TODO: remove this struct. Let client code use its own handlers instead. Workspace shoud be
// Workspace<T> where T is a handle.
/// Handle to a dock
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct DockHandle(pub u64);

/// Top level structure.
#[derive(Debug, Clone)]
pub struct Workspace {
    pub root_area: Option<Area>,
    rect: Rect,
    handle_counter: SplitHandle,
}


/// This enum identifies some now, not yet existing, place in the structure. Used to point where
/// new unit in structure should be created.
#[derive(Debug, Clone)]
pub enum ItemTarget {
    /// Root area should be split. This is usable if root area is `Container` or `Split` that
    /// should become child of new `Split` with opposite direction. Applying this will result in
    /// creating new `Split` with supplied direction. Second arguments points at position of child
    /// to be created (0 for first child, 1 for second child).
    SplitRoot(Direction, usize),
    /// New child should be added to existing `Split`. All the children starting with second
    /// argument will be shifted.
    AppendToSplit(SplitHandle, usize),
    /// Container identified by (`SplitHandle`, `usize` (index of child in split)) should be split
    /// in direction opposite to parent `Split`. After splitting, it should be placed at index equal
    /// to third parameter of this enum.
    SplitContainer(SplitHandle, usize, usize),
    /// New dock should be added to container. Container is identified by dock handle (which means
    /// it points at container with child `DockHandle`). New dock should be placed at index equal to
    /// second parameter of this enum, exising children starting with this index should be shifted.
    AppendToContainer(DockHandle, usize),
    /// ItemTarget added for convenience' sake. Container with `DockHandle` should be split in
    /// `Direction` and placed as `usize` child of new split. This is normalized to either
    /// `SplitContainer` or `AppendToSplit` when doing actual insertion. See
    /// `WorkSpace::normalize_target` for more.
    SplitDock(DockHandle, Direction, usize),
}

impl Workspace {
    /// Construct a new workspace.
    pub fn new(rect: Rect) -> Workspace {
        Workspace {
            root_area: None,
            rect: rect,
            handle_counter: SplitHandle(0),
        }
    }

    /// Generates new handle for Split
    fn next_handle(&mut self) -> SplitHandle {
        self.handle_counter.0 += 1;
        self.handle_counter
    }

    /// Used to set or change root area
    pub fn initialize(&mut self, handle: DockHandle) {
        self.root_area = Some(Area::Container(Container::new(handle, self.rect.clone())));
    }

    /// Returns area occupied by supplied `DockHandle`
    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        self.root_area.as_ref().and_then(|area| {
            area.get_container_by_dock_handle(handle)
                .and_then(|container| Some(container.rect.clone()))
        })
    }

    /// Updates area and propagates change to the structure
    pub fn update_rect(&mut self, new_rect: Rect) {
        self.rect = new_rect;
        if let Some(ref mut a) = self.root_area {
            a.update_rect(new_rect);
        }
    }

    /// Changes ratio of sizer identified by (`handle`, `index`).
    pub fn change_ratio(&mut self,
                        handle: SplitHandle,
                        index: usize,
                        origin: f32,
                        delta: (f32, f32)) {
        if let Some(ref mut root) = self.root_area {
            if let Some(s) = root.get_split_by_handle(handle) {
                s.change_ratio(index, origin, delta);
            }
        }
    }

    /// Returns ratio of sizer identified by (`handle`, `index`)
    pub fn get_ratio(&mut self, handle: SplitHandle, index: usize) -> f32 {
        if let Some(ref mut root) = self.root_area {
            if let Some(s) = root.get_split_by_handle(handle) {
                return s.ratios[index];
            }
        }
        0.0
    }

    /// Returns sizer identifier at `pos`
    pub fn get_sizer_at_pos(&self, pos: (f32, f32)) -> Option<SizerPos> {
        self.root_area.as_ref().and_then(|root| root.get_sizer_at_pos(pos))
    }

    /// Returns `ItemTarget` and area that will be occupied by this `ItemTarget` for corresponding
    /// point at `pos`
    pub fn get_item_target_at_pos(&self, pos: (f32, f32)) -> Option<(ItemTarget, Rect)> {
        self.root_area.as_ref().and_then(|root| root.get_item_target_at_pos(pos))
    }

    /// Returns `DockHandle` which header or tab is at `pos`.
    pub fn get_dock_handle_with_header_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        self.root_area.as_ref().and_then(|root| root.get_dock_handle_with_header_at_pos(pos))
    }

    /// Returns `DockHandle` that is currently under `pos`
    pub fn get_dock_handle_at_pos(&self, pos: (f32, f32)) -> Option<DockHandle> {
        self.root_area.as_ref().and_then(|root| root.get_dock_handle_at_pos(pos))
    }

    /// Normalizes `ItemTarget`. Replaces `SplitDock` with `SplitContainer`, `AppendToSplit` or
    /// `SplitRoot` depending on current structure and `direction` of `SplitDock`.
    fn normalize_target(&self, target: &ItemTarget) -> Option<ItemTarget> {
        self.root_area.as_ref().map(|root| {
            match *target {
                ItemTarget::SplitDock(target_handle, direction, pos) => {
                    let res = if let Some((sh, position)) =
                                     root.get_split_by_dock_handle(target_handle) {
                        if sh.direction != direction {
                            // split with different direction
                            ItemTarget::SplitContainer(sh.handle, position, pos)
                        } else {
                            ItemTarget::AppendToSplit(sh.handle, position + pos)
                        }
                    } else {
                        ItemTarget::SplitRoot(direction, pos)
                    };
                    return res;
                }
                _ => (*target).clone(),
            }
        })
    }

    fn index_is_neighbour(cur: usize, target: usize) -> bool {
        target == cur || target == cur + 1
    }

    /// Returns true if moving `DockHandle` identified by `handle` to `target` will change nothing.
    pub fn already_at_place(&self, target: &ItemTarget, handle: DockHandle) -> bool {
        let mut res = false;
        if let Some(target) = self.normalize_target(target) {
            if let Some(ref root) = self.root_area {
                let mut single_dock = true;
                if let Some(container) = root.get_container_by_dock_handle(handle) {
                    single_dock = container.docks.len() == 1;
                }
                match target {
                    ItemTarget::AppendToSplit(target_handle, target_index) if single_dock => {
                        if let Some((split, cur_index)) = root.get_split_by_dock_handle(handle) {
                            res = split.handle == target_handle &&
                                  Self::index_is_neighbour(cur_index, target_index)
                        }
                    }
                    ItemTarget::AppendToContainer(target_handle, target_index) => {
                        if let Some(container) = root.get_container_by_dock_handle(target_handle) {
                            if let Some(cur_index) = container.docks
                                .iter()
                                .position(|dock_handle| *dock_handle == handle) {

                                res = Self::index_is_neighbour(cur_index, target_index);
                            }
                        }
                    }
                    ItemTarget::SplitContainer(target_handle, target_index, _) if single_dock => {
                        if let Some((split, cur_index)) = root.get_split_by_dock_handle(handle) {
                            res = split.handle == target_handle && cur_index == target_index;
                        }
                    }
                    _ => {}
                }
            }
        }
        return res;
    }

    /// Inserts `handle` into a place identified by `target`
    pub fn create_dock_at(&mut self, target: ItemTarget, handle: DockHandle) {
        if let Some(target) = self.normalize_target(&target) {
            match target {
                ItemTarget::SplitRoot(direction, index) => {
                    let next_handle = self.next_handle();
                    if let Some(ref mut root) = self.root_area {
                        let old_root = root.clone();
                        let new_dock = Area::container_from_dock(handle);
                        let new_child = if index == 0 {
                            Split::from_two(direction,
                                            0.5,
                                            next_handle,
                                            self.rect.clone(),
                                            new_dock,
                                            old_root)
                        } else {
                            Split::from_two(direction,
                                            0.5,
                                            next_handle,
                                            self.rect.clone(),
                                            old_root,
                                            new_dock)
                        };
                        *root = Area::Split(new_child);
                    }
                }
                ItemTarget::AppendToSplit(split_handle, index) => {
                    if let Some(ref mut root) = self.root_area {
                        if let Some(s) = root.get_split_by_handle(split_handle) {
                            s.insert_child(index, Area::container_from_dock(handle));
                        }
                    }
                }
                ItemTarget::SplitContainer(split_handle, index, new_index) => {
                    let next_handle = self.next_handle();
                    if let Some(ref mut root) = self.root_area {
                        if let Some(s) = root.get_split_by_handle(split_handle) {
                            let old_copy = s.children[index].clone();
                            let new_dock = Area::container_from_dock(handle);
                            let new_child = Area::Split(if new_index == 0 {
                                Split::from_two(s.direction.opposite(),
                                                0.5,
                                                next_handle,
                                                self.rect.clone(),
                                                new_dock,
                                                old_copy)
                            } else {
                                Split::from_two(s.direction.opposite(),
                                                0.5,
                                                next_handle,
                                                self.rect.clone(),
                                                old_copy,
                                                new_dock)
                            });
                            s.replace_child(index, new_child);
                        }
                    }
                }
                ItemTarget::AppendToContainer(handle, new_index) => {
                    if let Some(ref mut root) = self.root_area {
                        if let Some(c) = root.get_container_by_dock_handle_mut(handle) {
                            c.insert_dock(new_index, handle);
                        }
                    }
                }
                _ => {
                    // Should never happen due to normalization
                }
            }
        }
    }

    /// Deletes dock identified by `handle`. Note that due to structural change stored `ItemTarget`
    /// object can become invalid.
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
            if let Some((split, index)) = root.get_split_by_dock_handle_mut(handle) {
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
                } else if let Some((parent_split, index)) =
                       root.get_parent_split_by_split_handle(split_handle) {
                    match contents {
                        Area::Split(ref mut s) if s.direction == parent_split.direction => {
                            parent_split.replace_child_with_children(index, &s.children)
                        }
                        _ => {
                            parent_split.replace_child(index, contents);
                        }
                    }
                }
            }
        }
    }

    /// Move dock identified by `handle` to a `target`. Strategy used is simple:
    /// * find source dock, copy it, mark with special DockHandle;
    /// * insert copy of source dock using `create_dock` method;
    /// * delete marked dock using `delete_dock_by_handle` method;
    ///
    /// This strategy ensures that ItemTarget will always be valid (since we perform no structural
    /// changes before using it) and we will always delete source dock (since we marked it and will
    /// not mess it with newly created).
    pub fn move_dock(&mut self, handle: DockHandle, mut target: ItemTarget) {
        let marker = DockHandle(u64::max_value());
        let copy = self.root_area
            .as_mut()
            .and_then(|root| root.get_container_by_dock_handle_mut(handle))
            .and_then(|c| c.replace_dock(handle, marker));
        match target {
            ItemTarget::SplitDock(ref mut target_handle, _, _) => {
                if *target_handle == handle {
                    *target_handle = marker;
                }
            }
            ItemTarget::AppendToContainer(ref mut target_handle, _) => {
                if *target_handle == handle {
                    *target_handle = marker;
                }
            }
            _ => {}
        }
        if let Some(dock) = copy {
            self.create_dock_at(target, dock);
            self.delete_dock_by_handle(marker);
        }
    }

    /// Returns serialized contents of Workspace. `Rect` fields are not serialized.
    pub fn save_state(&self) -> String {
        serde_json::to_string(self).unwrap()
    }

    /// Creates workspace from serialized string. Since `Rect` fields are not serialized, call
    /// `update_rect` to set new one.
    pub fn from_state(state: &str) -> Result<Workspace, serde_json::Error> {
        serde_json::from_str(state)
    }

    // TODO: return iterator over workspace
    /// Returns all the docks in this structure
    pub fn get_docks(&self) -> Vec<DockHandle> {
        let mut docks = Vec::new();
        match self.root_area {
            Some(ref root) => Workspace::collect_docks(&mut docks, root),
            None => {}
        };
        return docks;
    }

    fn collect_docks(target: &mut Vec<DockHandle>, source: &Area) {
        match *source {
            Area::Container(ref c) => {
                for dock in &c.docks {
                    target.push(*dock);
                }
            }
            Area::Split(ref s) => {
                for child in &s.children {
                    Workspace::collect_docks(target, child);
                }
            }
        }
    }
}

#[cfg(test)]
mod test {
    extern crate serde_json;

    use {Area, Container, Direction, DockHandle, ItemTarget, Rect, Split, SplitHandle, Workspace};
    use test_helper::{is_container_with_single_dock, rects_are_equal};

    fn test_area_container(id: u64) -> Area {
        Area::Container(Container::new(DockHandle(id), Rect::default()))
    }

    fn test_area_split(dir: Direction, id: u64, first: Area, second: Area) -> Area {
        Area::Split(Split::from_two(dir, 0.5, SplitHandle(id), Rect::default(), first, second))
    }

    #[test]
    fn test_delete_dock_by_handle_deletes_empty_container_from_root() {
        let mut ws = Workspace {
            root_area: Some(test_area_container(5)),
            rect: Rect::default(),
            handle_counter: SplitHandle(0),
        };
        ws.delete_dock_by_handle(DockHandle(5));
        assert!(ws.root_area.is_none());
    }

    #[test]
    fn test_delete_dock_by_handle_deletes_empty_split_from_root() {
        let first = test_area_container(0);
        let second = test_area_container(1);
        let split = test_area_split(Direction::Horizontal, 0, first, second);
        let mut ws = Workspace {
            root_area: Some(split),
            rect: Rect::default(),
            handle_counter: SplitHandle(1),
        };
        ws.delete_dock_by_handle(DockHandle(0));
        match ws.root_area.unwrap() {
            Area::Container(ref c) => assert_eq!(c.docks[0], DockHandle(1)),
            _ => panic!("Root node should become container"),
        }
    }

    #[test]
    fn test_delete_dock_by_handle_delete_empty_container_from_split() {
        let first = test_area_container(0);
        let second = test_area_container(1);
        let third = test_area_container(2);
        let mut split = Split::from_two(Direction::Horizontal,
                                        0.5,
                                        SplitHandle(0),
                                        Rect::default(),
                                        first,
                                        second);
        split.push_child(third);
        let mut ws = Workspace {
            root_area: Some(Area::Split(split)),
            rect: Rect::default(),
            handle_counter: SplitHandle(2),
        };
        ws.delete_dock_by_handle(DockHandle(0));
        match ws.root_area.unwrap() {
            Area::Split(ref s) => {
                assert!(is_container_with_single_dock(&s.children[0], 1));
                assert!(is_container_with_single_dock(&s.children[1], 2));
            }
            _ => panic!("Root node should become container"),
        }
    }

    #[test]
    fn test_delete_dock_by_handle_adopts_children() {
        let first = test_area_container(1);
        let second = test_area_container(2);
        let third = test_area_container(3);
        let fourth = test_area_container(4);
        let bottom_split = test_area_split(Direction::Horizontal, 0, third, fourth);
        let middle_split = test_area_split(Direction::Vertical, 1, second, bottom_split);
        let top_split = test_area_split(Direction::Horizontal, 2, middle_split, first);
        let mut ws = Workspace {
            root_area: Some(top_split),
            rect: Rect::default(),
            handle_counter: SplitHandle(3),
        };
        ws.delete_dock_by_handle(DockHandle(2));
        match ws.root_area.unwrap() {
            Area::Split(ref s) => {
                assert_eq!(s.children.len(), 3);
                assert!(is_container_with_single_dock(&s.children[0], 3));
                assert!(is_container_with_single_dock(&s.children[1], 4));
                assert!(is_container_with_single_dock(&s.children[2], 1));
            }
            _ => panic!("Root node should be container"),
        }
    }

    #[test]
    fn test_create_dock_at_split_root_0() {
        let mut ws = Workspace {
            root_area: Some(test_area_container(5)),
            rect: Rect::default(),
            handle_counter: SplitHandle(0),
        };
        let target = ItemTarget::SplitRoot(Direction::Horizontal, 0);
        ws.create_dock_at(target, DockHandle(6));
        match ws.root_area.unwrap() {
            Area::Split(ref s) => {
                assert_eq!(s.children.len(), 2);
                assert!(is_container_with_single_dock(&s.children[0], 6));
                assert!(is_container_with_single_dock(&s.children[1], 5));
            }
            _ => panic!("Root node should be split"),
        }
    }

    #[test]
    fn test_create_dock_at_split_root_1() {
        let mut ws = Workspace {
            root_area: Some(test_area_container(5)),
            rect: Rect::default(),
            handle_counter: SplitHandle(0),
        };
        let target = ItemTarget::SplitRoot(Direction::Vertical, 1);
        ws.create_dock_at(target, DockHandle(6));
        match ws.root_area.unwrap() {
            Area::Split(ref s) => {
                assert_eq!(s.children.len(), 2);
                assert!(is_container_with_single_dock(&s.children[0], 5));
                assert!(is_container_with_single_dock(&s.children[1], 6));
            }
            _ => panic!("Root node should be split"),
        }
    }

    #[test]
    fn test_create_dock_at_split_container() {
        let first = test_area_container(0);
        let second = test_area_container(1);
        let split = test_area_split(Direction::Horizontal, 0, first, second);
        let mut ws = Workspace {
            root_area: Some(split),
            rect: Rect::default(),
            handle_counter: SplitHandle(1),
        };
        let target = ItemTarget::SplitContainer(SplitHandle(0), 0, 1);
        ws.create_dock_at(target, DockHandle(2));
        match ws.root_area.unwrap() {
            Area::Split(ref s) => {
                assert_eq!(s.direction, Direction::Horizontal);
                assert_eq!(s.children.len(), 2);
                assert!(is_container_with_single_dock(&s.children[1], 1));
                match s.children[0] {
                    Area::Split(ref s) => {
                        assert_eq!(s.direction, Direction::Vertical);
                        assert_eq!(s.children.len(), 2);
                        assert!(is_container_with_single_dock(&s.children[0], 0));
                        assert!(is_container_with_single_dock(&s.children[1], 2));
                    }
                    _ => panic!("First child should be split"),
                }
            }
            _ => panic!("Root node should be split"),
        }
    }

    #[test]
    fn test_create_dock_at_split_dock_1() {
        let first = test_area_container(0);
        let second = test_area_container(1);
        let split = test_area_split(Direction::Horizontal, 0, first, second);
        let mut ws = Workspace {
            root_area: Some(split),
            rect: Rect::default(),
            handle_counter: SplitHandle(1),
        };
        let target = ItemTarget::SplitDock(DockHandle(1), Direction::Horizontal, 1);
        ws.create_dock_at(target, DockHandle(2));
        match ws.root_area.unwrap() {
            Area::Split(ref s) => {
                assert_eq!(s.children.len(), 3);
                assert!(is_container_with_single_dock(&s.children[0], 0));
                assert!(is_container_with_single_dock(&s.children[1], 1));
                assert!(is_container_with_single_dock(&s.children[2], 2));
            }
            _ => panic!("Root node should be split"),
        }
    }

    #[test]
    fn test_create_dock_at_split_dock_2() {
        let first = test_area_container(0);
        let second = test_area_container(1);
        let split = test_area_split(Direction::Horizontal, 0, first, second);
        let mut ws = Workspace {
            root_area: Some(split),
            rect: Rect::default(),
            handle_counter: SplitHandle(1),
        };
        let target = ItemTarget::SplitDock(DockHandle(0), Direction::Vertical, 0);
        ws.create_dock_at(target, DockHandle(2));
        match ws.root_area.unwrap() {
            Area::Split(ref s) => {
                assert_eq!(s.direction, Direction::Horizontal);
                assert_eq!(s.children.len(), 2);
                assert!(is_container_with_single_dock(&s.children[1], 1));
                match s.children[0] {
                    Area::Split(ref s) => {
                        assert_eq!(s.direction, Direction::Vertical);
                        assert_eq!(s.children.len(), 2);
                        assert!(is_container_with_single_dock(&s.children[0], 2));
                        assert!(is_container_with_single_dock(&s.children[1], 0));
                    }
                    _ => panic!("First child should be split"),
                }
            }
            _ => panic!("Root node should be split"),
        }
    }

    #[test]
    fn test_workspace_serialize_0() {
        let ws_in = Workspace {
            root_area: None,
            rect: Rect::new(4.0, 5.0, 2.0, 8.0),
            handle_counter: SplitHandle(2),
        };

        let serialized = serde_json::to_string(&ws_in).unwrap();
        let ws_out: Workspace = serde_json::from_str(&serialized).unwrap();

        assert!(ws_out.root_area.is_none());
        assert!(rects_are_equal(&Rect::default(), &ws_out.rect));
    }

    #[test]
    fn test_workspace_serialize_1() {
        let ws_in = Workspace {
            root_area: Some(Area::container_from_dock(DockHandle(5))),
            rect: Rect::new(4.0, 5.0, 2.0, 8.0),
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
