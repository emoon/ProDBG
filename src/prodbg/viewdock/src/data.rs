#[derive(Debug, PartialEq, Clone, Copy)]
pub struct DockHandle(pub u64);

#[derive(Debug, PartialEq, Clone, Copy)]
pub struct SplitHandle(pub u64);

#[derive(Debug, Default, Clone, Copy)]
pub struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

/*
struct RectVisitor<'a> {
    value: &'a Rect
}

impl<'a> serde::ser::MapVisitor for RectVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
        where S: serde::Serializer
    {
        try!(serializer.serialize_struct_elt("x", &self.value.x));
        try!(serializer.serialize_struct_elt("y", &self.value.y));
        try!(serializer.serialize_struct_elt("width", &self.value.y));
        try!(serializer.serialize_struct_elt("height", &self.value.height));
        Ok(None)
    }
}

impl serde::ser::Serialize for Rect {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> 
        where S: serde::ser::Serializer {
        serializer.serialize_struct("", RectVisitor { value: self }).map(|_| ())
    }
}
*/

#[derive(Debug, Clone)]
pub struct Dock {
    pub handle: DockHandle,
    pub plugin_name: String,
    pub plugin_data: Option<Vec<String>>,
    pub rect: Rect
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

#[derive(Debug)]
pub struct Split {
    /// left/top slipit
    pub left: Option<SplitHandle>,
    /// right/bottom split
    pub right: Option<SplitHandle>,
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
    /// Rect
    rect: Rect,
}

#[derive(Debug)]
pub struct Workspace {
    pub splits: Vec<Split>,
    pub rect: Rect,
    /// border size of the windows (in pixels)
    pub window_border: f32,
    handle_counter: SplitHandle,
}



