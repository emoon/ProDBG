extern crate serde;

use Rect;
use DockHandle;
use SplitHandle;

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
