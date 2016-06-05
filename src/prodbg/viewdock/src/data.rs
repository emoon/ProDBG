extern crate serde;

use {DockHandle, SplitHandle, Rect, Container, Dock, Direction, Split, Workspace};

struct RectMapVisitor<'a> {
    value: &'a Rect
}

impl<'a> serde::ser::MapVisitor for RectMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
        where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("x", &self.value.x));
        try!(serializer.serialize_struct_elt("y", &self.value.y));
        try!(serializer.serialize_struct_elt("width", &self.value.width));
        try!(serializer.serialize_struct_elt("height", &self.value.height));
        Ok(None)
    }
}

impl serde::ser::Serialize for Rect {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer {
        serializer.serialize_struct("", RectMapVisitor { value: self }).map(|_| ())
    }
}

enum RectField {
    X,
    Y,
    WIDTH,
    HEIGHT,
}

impl serde::Deserialize for RectField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<RectField, D::Error>
        where D: serde::de::Deserializer {
        struct RectFieldVisitor;

        impl serde::de::Visitor for RectFieldVisitor {
            type Value = RectField;

            fn visit_str<E>(&mut self, value: &str) -> Result<RectField, E>
                where E: serde::de::Error {
                match value {
                    "x" => Ok(RectField::X),
                    "y" => Ok(RectField::Y),
                    "width" => Ok(RectField::WIDTH),
                    "height" => Ok(RectField::HEIGHT),
                    _ => Err(serde::de::Error::custom("expected x,y,width or height")),
                }
            }
        }

        deserializer.deserialize(RectFieldVisitor)
    }
}

struct RectVisitor;

impl serde::Deserialize for Rect {
    fn deserialize<D>(deserializer: &mut D) -> Result<Rect, D::Error>
        where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["x", "y", "width", "height"];
        deserializer.deserialize_struct("Rect", FIELDS, RectVisitor)
    }
}

impl serde::de::Visitor for RectVisitor {
    type Value = Rect;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Rect, V::Error>
        where V: serde::de::MapVisitor {
        let mut x = None;
        let mut y = None;
        let mut width = None;
        let mut height = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(RectField::X) => { x = Some(try!(visitor.visit_value())); }
                Some(RectField::Y) => { y = Some(try!(visitor.visit_value())); }
                Some(RectField::WIDTH) => { width = Some(try!(visitor.visit_value())); }
                Some(RectField::HEIGHT) => { height = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let x = match x {
            Some(x) => x,
            None => try!(visitor.missing_field("x")),
        };

        let y = match y {
            Some(y) => y,
            None => try!(visitor.missing_field("y")),
        };

        let width = match width {
            Some(width) => width,
            None => try!(visitor.missing_field("width")),
        };

        let height = match height {
            Some(height) => height,
            None => try!(visitor.missing_field("height")),
        };

        try!(visitor.end());

        Ok(Rect{ x: x, y: y, width: width, height: height })
    }
}


// Use a macro here otherwise we would have needed to copy'n'paste this code twice.

macro_rules! gen_handle {
    ($name:expr, $type_name:ident, $visitor:ident) => {

        impl serde::ser::Serialize for $type_name {
            fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
                where S: serde::ser::Serializer {
                serializer.serialize_newtype_struct($name, &self.0)
            }
        }

        struct $visitor;

        impl serde::Deserialize for $type_name  {
            fn deserialize<D>(deserializer: &mut D) -> Result<$type_name, D::Error> where D: serde::de::Deserializer {
                deserializer.deserialize_newtype_struct($name, $visitor)
            }
        }

        impl serde::de::Visitor for $visitor {
            type Value = $type_name;

            fn visit_newtype_struct<D>(&mut self, deserializer: &mut D) -> Result<Self::Value, D::Error>
                where D: serde::de::Deserializer {
                    let value = try!(serde::de::Deserialize::deserialize(deserializer));
                    Ok($type_name(value))
            }

            fn visit_seq<V>(&mut self, mut visitor: V) -> Result<$type_name, V::Error>
                where V: serde::de::SeqVisitor {
                let v = match try!(visitor.visit()) {
                    Some(value) => { value }
                    None => { return Err(serde::de::Error::end_of_stream()); }
                };
                try!(visitor.end());
                Ok($type_name(v))
            }
        }
    }
}

gen_handle!("SplitHandle", SplitHandle, SplitHandleVisitor);
gen_handle!("DockHandle", DockHandle, DockHandleVisitor);

struct DockMapVisitor<'a> {
    value: &'a Dock
}

impl<'a> serde::ser::MapVisitor for DockMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
        where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("handle", &self.value.handle));
        try!(serializer.serialize_struct_elt("plugin_name", &self.value.plugin_name));
        try!(serializer.serialize_struct_elt("plugin_data", &self.value.plugin_data));
        Ok(None)
    }
}

impl serde::ser::Serialize for Dock {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer {
        serializer.serialize_struct("Dock", DockMapVisitor { value: self }).map(|_| ())
    }
}

enum DockField {
    Handle,
    PluginName,
    PluginData,
}

impl serde::Deserialize for DockField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<DockField, D::Error>
        where D: serde::de::Deserializer {
        struct DockFieldVisitor;

        impl serde::de::Visitor for DockFieldVisitor {
            type Value = DockField;

            fn visit_str<E>(&mut self, value: &str) -> Result<DockField, E>
                where E: serde::de::Error {
                match value {
                    "handle" => Ok(DockField::Handle),
                    "plugin_name" => Ok(DockField::PluginName),
                    "plugin_data" => Ok(DockField::PluginData),
                    _ => Err(serde::de::Error::custom("expected handle,plugin_name or pluin_data")),
                }
            }
        }

        deserializer.deserialize(DockFieldVisitor)
    }
}

struct DockVisitor;

impl serde::Deserialize for Dock {
    fn deserialize<D>(deserializer: &mut D) -> Result<Dock, D::Error>
        where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["handle", "plugin_name", "plugin_data"];
        deserializer.deserialize_struct("Dock", FIELDS, DockVisitor)
    }
}

impl serde::de::Visitor for DockVisitor {
    type Value = Dock;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Dock, V::Error>
        where V: serde::de::MapVisitor {
        let mut handle = None;
        let mut plugin_name = None;
        let mut plugin_data = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(DockField::Handle) => { handle = Some(try!(visitor.visit_value())); }
                Some(DockField::PluginName) => { plugin_name = Some(try!(visitor.visit_value())); }
                Some(DockField::PluginData) => { plugin_data = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let handle = match handle {
            Some(handle) => handle,
            None => try!(visitor.missing_field("handle")),
        };

        let plugin_name = match plugin_name {
            Some(plugin_name) => plugin_name,
            None => try!(visitor.missing_field("plugin_name")),
        };

        let plugin_data = match plugin_data {
            Some(plugin_data) => plugin_data,
            None => try!(visitor.missing_field("plugin_data")),
        };

        try!(visitor.end());

        Ok(Dock {
            handle: handle,
            plugin_name: plugin_name,
            plugin_data: plugin_data,
            rect: Rect::default(), // We use default here as this is always recalculated
        })
    }
}

struct ContainerMapVisitor<'a> {
    value: &'a Container
}

impl<'a> serde::ser::MapVisitor for ContainerMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
        where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("docks", &self.value.docks));
        Ok(None)
    }
}

impl serde::ser::Serialize for Container {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer {
        serializer.serialize_struct("Container", ContainerMapVisitor { value: self }).map(|_| ())
    }
}

enum ContainerField {
    Docks,
}

impl serde::Deserialize for ContainerField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<ContainerField, D::Error>
        where D: serde::de::Deserializer {
        struct ContainerFieldVisitor;

        impl serde::de::Visitor for ContainerFieldVisitor {
            type Value = ContainerField;

            fn visit_str<E>(&mut self, value: &str) -> Result<ContainerField, E>
                where E: serde::de::Error {
                match value {
                    "docks" => Ok(ContainerField::Docks),
                    _ => Err(serde::de::Error::custom("expected docks")),
                }
            }
        }

        deserializer.deserialize(ContainerFieldVisitor)
    }
}

struct ContainerVisitor;

impl serde::Deserialize for Container {
    fn deserialize<D>(deserializer: &mut D) -> Result<Container, D::Error>
        where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["docks"];
        deserializer.deserialize_struct("Container", FIELDS, ContainerVisitor)
    }
}

impl serde::de::Visitor for ContainerVisitor {
    type Value = Container;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Container, V::Error>
        where V: serde::de::MapVisitor {
        let mut docks = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(ContainerField::Docks) => { docks = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let docks = match docks {
            Some(docks) => docks,
            None => Vec::new(),
        };

        try!(visitor.end());

        Ok(Container {
            docks: docks,
            rect: Rect::default(), // We use default here as this is always recalculated
        })
    }
}

impl serde::ser::Serialize for Direction {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer {
        match *self {
            Direction::Vertical => serde::ser::Serializer::serialize_unit_variant(serializer, "Direction", 0usize, "Vertical"),
            Direction::Horizontal => serde::ser::Serializer::serialize_unit_variant(serializer, "Direction", 1usize, "Horizontal"),
            Direction::Full => serde::ser::Serializer::serialize_unit_variant(serializer, "Direction", 2usize, "Full"),
        }
    }
}

enum DirectionField {
    Vertical,
    Horizontal,
    Full,
}

impl serde::Deserialize for DirectionField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<DirectionField, D::Error>
        where D: serde::de::Deserializer {
        struct DirectionFieldVisitor;

        impl serde::de::Visitor for DirectionFieldVisitor {
            type Value = DirectionField;

            fn visit_usize<E>(&mut self, value: usize) -> Result<DirectionField, E>
                where E: serde::de::Error {
                match value {
                    0usize => Ok(DirectionField::Vertical),
                    1usize => Ok(DirectionField::Horizontal),
                    2usize => Ok(DirectionField::Full),
                    _ => Err(serde::de::Error::invalid_value("expected a variant")),
                }
            }

            fn visit_str<E>(&mut self, value: &str) -> Result<DirectionField, E>
                where E: serde::de::Error {
                match value {
                    "Vertial" => Ok(DirectionField::Vertical),
                    "Horizontal" => Ok(DirectionField::Horizontal),
                    "Full" => Ok(DirectionField::Full),
                    _ => Err(serde::de::Error::invalid_value("expected a variant")),
                }
            }
        }

        deserializer.deserialize_struct_field(DirectionFieldVisitor)
    }
}

struct DirectionVisitor;

impl serde::Deserialize for Direction {
    fn deserialize<D>(deserializer: &mut D) -> Result<Direction, D::Error>
        where D: serde::de::Deserializer {
        const VARIANTS: &'static [&'static str] = &["Vertical", "Horizontal", "Full"];
        deserializer.deserialize_enum("Direction", VARIANTS, DirectionVisitor)
    }
}

impl serde::de::EnumVisitor for DirectionVisitor {
    type Value = Direction;

    fn visit<V>(&mut self, mut visitor: V) -> Result<Direction, V::Error>
        where V: serde::de::VariantVisitor {

        match try!(visitor.visit_variant()) {
            DirectionField::Vertical => {
                try!(visitor.visit_unit());
                Ok(Direction::Vertical)
            },
            DirectionField::Horizontal => {
                try!(visitor.visit_unit());
                Ok(Direction::Horizontal)
            }
            DirectionField::Full => {
                try!(visitor.visit_unit());
                Ok(Direction::Full)
            }
        }
    }
}

struct SplitMapVisitor<'a> {
    value: &'a Split
}

impl<'a> serde::ser::MapVisitor for SplitMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
        where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("left", &self.value.left));
        try!(serializer.serialize_struct_elt("right", &self.value.right));
        try!(serializer.serialize_struct_elt("left_docks", &self.value.left_docks));
        try!(serializer.serialize_struct_elt("right_docks", &self.value.right_docks));
        try!(serializer.serialize_struct_elt("ratio", &self.value.ratio));
        try!(serializer.serialize_struct_elt("direction", &self.value.direction));
        try!(serializer.serialize_struct_elt("handle", &self.value.handle));
        Ok(None)
    }
}

impl serde::ser::Serialize for Split {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer {
        serializer.serialize_struct("Split", SplitMapVisitor { value: self }).map(|_| ())
    }
}

enum SplitField {
    Left,
    Right,
    LeftDocks,
    RightDocks,
    Ratio,
    Direction,
    Handle,
}

impl serde::Deserialize for SplitField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<SplitField, D::Error>
        where D: serde::de::Deserializer {
        struct SplitFieldVisitor;

        impl serde::de::Visitor for SplitFieldVisitor {
            type Value = SplitField;

            fn visit_str<E>(&mut self, value: &str) -> Result<SplitField, E>
                where E: serde::de::Error {
                match value {
                    "left" => Ok(SplitField::Left),
                    "right" => Ok(SplitField::Right),
                    "left_docks" => Ok(SplitField::LeftDocks),
                    "right_docks" => Ok(SplitField::RightDocks),
                    "ratio" => Ok(SplitField::Ratio),
                    "direction" => Ok(SplitField::Direction),
                    "handle" => Ok(SplitField::Handle),
                    _ => Err(serde::de::Error::custom("expected left, right, left_docks, right_docs, ratio, direction or handle")),
                }
            }
        }

        deserializer.deserialize(SplitFieldVisitor)
    }
}

struct SplitVisitor;

impl serde::Deserialize for Split {
    fn deserialize<D>(deserializer: &mut D) -> Result<Split, D::Error>
        where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &[ "left", "right", "left_docks", "right_docks", "ratio", "direction", "handle"];
        deserializer.deserialize_struct("Split", FIELDS, SplitVisitor)
    }
}

impl serde::de::Visitor for SplitVisitor {
    type Value = Split;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Split, V::Error>
        where V: serde::de::MapVisitor {
        let mut left = None;
        let mut right = None;
        let mut left_docks = None;
        let mut right_docks = None;
        let mut ratio = None;
        let mut direction = None;
        let mut handle = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(SplitField::Left) => { left = Some(try!(visitor.visit_value())); }
                Some(SplitField::Right) => { right = Some(try!(visitor.visit_value())); }
                Some(SplitField::LeftDocks) => { left_docks = Some(try!(visitor.visit_value())); }
                Some(SplitField::RightDocks) => { right_docks = Some(try!(visitor.visit_value())); }
                Some(SplitField::Ratio) => { ratio = Some(try!(visitor.visit_value())); }
                Some(SplitField::Direction) => { direction = Some(try!(visitor.visit_value())); }
                Some(SplitField::Handle) => { handle = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let left = match left {
            Some(left) => left,
            None => try!(visitor.missing_field("left")),
        };

        let right = match right {
            Some(right) => right,
            None => try!(visitor.missing_field("right")),
        };

        let left_docks = match left_docks {
            Some(left_docks) => left_docks,
            None => try!(visitor.missing_field("left_docks")),
        };

        let right_docks = match right_docks {
            Some(right_docks) => right_docks,
            None => try!(visitor.missing_field("right_docks")),
        };

        let ratio = match ratio {
            Some(ratio) => ratio,
            None => try!(visitor.missing_field("ratio")),
        };

        let direction = match direction {
            Some(direction) => direction,
            None => try!(visitor.missing_field("direction")),
        };

        let handle = match handle {
            Some(handle) => handle,
            None => try!(visitor.missing_field("handle")),
        };

        try!(visitor.end());

        Ok(Split {
            left: left,
            right: right,
            left_docks: left_docks,
            right_docks: right_docks,
            ratio: ratio,
            direction: direction,
            handle: handle,
            rect: Rect::default(), // reconstructed during update
        })
    }
}

struct WorkspaceMapVisitor<'a> {
    value: &'a Workspace
}

impl<'a> serde::ser::MapVisitor for WorkspaceMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
        where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("splits", &self.value.splits));
        try!(serializer.serialize_struct_elt("rect", &self.value.rect));
        try!(serializer.serialize_struct_elt("window_border", &self.value.window_border));
        try!(serializer.serialize_struct_elt("handle_counter", &self.value.handle_counter));
        Ok(None)
    }
}

impl serde::ser::Serialize for Workspace {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer {
        serializer.serialize_struct("", WorkspaceMapVisitor { value: self }).map(|_| ())
    }
}

enum WorkspaceField {
    Splits,
    Rect,
    WindowBorder,
    HandleCounter,
}

impl serde::Deserialize for WorkspaceField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<WorkspaceField, D::Error>
        where D: serde::de::Deserializer {
        struct WorkspaceFieldVisitor;

        impl serde::de::Visitor for WorkspaceFieldVisitor {
            type Value = WorkspaceField;

            fn visit_str<E>(&mut self, value: &str) -> Result<WorkspaceField, E>
                where E: serde::de::Error {
                match value {
                    "splits" => Ok(WorkspaceField::Splits),
                    "rect" => Ok(WorkspaceField::Rect),
                    "window_border" => Ok(WorkspaceField::WindowBorder),
                    "handle_counter" => Ok(WorkspaceField::HandleCounter),
                    _ => Err(serde::de::Error::custom("expected splits,rect,window_border or handle_counter")),
                }
            }
        }

        deserializer.deserialize(WorkspaceFieldVisitor)
    }
}

struct WorkspaceVisitor;

impl serde::Deserialize for Workspace {
    fn deserialize<D>(deserializer: &mut D) -> Result<Workspace, D::Error>
        where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["splits", "rect", "window_border", "handle_counter"];
        deserializer.deserialize_struct("Workspace", FIELDS, WorkspaceVisitor)
    }
}

impl serde::de::Visitor for WorkspaceVisitor {
    type Value = Workspace;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Workspace, V::Error>
        where V: serde::de::MapVisitor {
        let mut splits = None;
        let mut rect = None;
        let mut window_border = None;
        let mut handle_counter = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(WorkspaceField::Splits) => { splits = Some(try!(visitor.visit_value())); }
                Some(WorkspaceField::Rect) => { rect = Some(try!(visitor.visit_value())); }
                Some(WorkspaceField::WindowBorder) => { window_border = Some(try!(visitor.visit_value())); }
                Some(WorkspaceField::HandleCounter) => { handle_counter = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let splits = match splits {
            Some(splits) => splits,
            None => try!(visitor.missing_field("splits")),
        };

        let rect = match rect {
            Some(rect) => rect,
            None => try!(visitor.missing_field("rect")),
        };

        let window_border = match window_border {
            Some(window_border) => window_border,
            None => try!(visitor.missing_field("window_border")),
        };

        let handle_counter = match handle_counter {
            Some(handle_counter) => handle_counter,
            None => try!(visitor.missing_field("handle_counter")),
        };

        try!(visitor.end());

        Ok(Workspace {
            splits: splits,
            rect: rect,
            window_border: window_border,
            handle_counter: handle_counter,
        })
    }
}


