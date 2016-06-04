extern crate serde;

use Rect;
use DockHandle;
use SplitHandle;
use Container;
use Dock;

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


/*
extern crate serde;
extern crate serde_json;

#[derive(Debug)]
struct Point {
    x: i32,
    y: i32,
}

enum PointField {
    X,
    Y,
}

impl serde::Deserialize for PointField {
    fn deserialize<D>(deserializer: &mut D) -> Result<PointField, D::Error>
        where D: serde::de::Deserializer
    {
        struct PointFieldVisitor;

        impl serde::de::Visitor for PointFieldVisitor {
            type Value = PointField;

            fn visit_str<E>(&mut self, value: &str) -> Result<PointField, E>
                where E: serde::de::Error
            {
                match value {
                    "x" => Ok(PointField::X),
                    "y" => Ok(PointField::Y),
                    _ => Err(serde::de::Error::custom("expected x or y")),
                }
            }
        }

        deserializer.deserialize(PointFieldVisitor)
    }
}

impl serde::Deserialize for Point {
    fn deserialize<D>(deserializer: &mut D) -> Result<Point, D::Error>
        where D: serde::de::Deserializer
    {
        static FIELDS: &'static [&'static str] = &["x", "y"];
        deserializer.deserialize_struct("Point", FIELDS, PointVisitor)
    }
}

struct PointVisitor;

impl serde::de::Visitor for PointVisitor {
    type Value = Point;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Point, V::Error>
        where V: serde::de::MapVisitor
    {
        let mut x = None;
        let mut y = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(PointField::X) => { x = Some(try!(visitor.visit_value())); }
                Some(PointField::Y) => { y = Some(try!(visitor.visit_value())); }
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

        try!(visitor.end());

        Ok(Point{ x: x, y: y })
    }
}
*/
