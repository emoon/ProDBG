extern crate serde;
use super::{Rect, Direction};

// Serialization of Rect

impl serde::ser::Serialize for Rect {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
        serializer.serialize_struct("", RectMapVisitor { value: self }).map(|_| ())
    }
}

struct RectMapVisitor<'a> {
    value: &'a Rect
}

impl<'a> serde::ser::MapVisitor for RectMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error> where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("x", &self.value.x));
        try!(serializer.serialize_struct_elt("y", &self.value.y));
        try!(serializer.serialize_struct_elt("width", &self.value.width));
        try!(serializer.serialize_struct_elt("height", &self.value.height));
        Ok(None)
    }
}

// Deserialization of Rect

impl serde::Deserialize for Rect {
    fn deserialize<D>(deserializer: &mut D) -> Result<Rect, D::Error> where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["x", "y", "width", "height"];
        deserializer.deserialize_struct("Rect", FIELDS, RectVisitor)
    }
}

struct RectVisitor;

impl serde::de::Visitor for RectVisitor {
    type Value = Rect;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Rect, V::Error> where V: serde::de::MapVisitor {
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

// Serialization of Direction

impl serde::ser::Serialize for Direction {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
        match *self {
            Direction::Vertical => serde::ser::Serializer::serialize_unit_variant(serializer, "Direction", 0usize, "Vertical"),
            Direction::Horizontal => serde::ser::Serializer::serialize_unit_variant(serializer, "Direction", 1usize, "Horizontal"),
        }
    }
}

// Deserialization of Direction

impl serde::Deserialize for Direction {
    fn deserialize<D>(deserializer: &mut D) -> Result<Direction, D::Error> where D: serde::de::Deserializer {
        const VARIANTS: &'static [&'static str] = &["Vertical", "Horizontal"];
        deserializer.deserialize_enum("Direction", VARIANTS, DirectionVisitor)
    }
}

struct DirectionVisitor;

impl serde::de::EnumVisitor for DirectionVisitor {
    type Value = Direction;

    fn visit<V>(&mut self, mut visitor: V) -> Result<Direction, V::Error> where V: serde::de::VariantVisitor {
        match try!(visitor.visit_variant()) {
            DirectionField::Vertical => {
                try!(visitor.visit_unit());
                Ok(Direction::Vertical)
            },
            DirectionField::Horizontal => {
                try!(visitor.visit_unit());
                Ok(Direction::Horizontal)
            }
        }
    }
}

enum DirectionField {
    Vertical,
    Horizontal,
}

impl serde::Deserialize for DirectionField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<DirectionField, D::Error> where D: serde::de::Deserializer {
        struct DirectionFieldVisitor;

        impl serde::de::Visitor for DirectionFieldVisitor {
            type Value = DirectionField;

            fn visit_usize<E>(&mut self, value: usize) -> Result<DirectionField, E>
                where E: serde::de::Error {
                    match value {
                        0usize => Ok(DirectionField::Vertical),
                        1usize => Ok(DirectionField::Horizontal),
                        _ => Err(serde::de::Error::invalid_value("expected a variant")),
                    }
                }

            fn visit_str<E>(&mut self, value: &str) -> Result<DirectionField, E>
                where E: serde::de::Error {
                    match value {
                        "Vertical" => Ok(DirectionField::Vertical),
                        "Horizontal" => Ok(DirectionField::Horizontal),
                        _ => Err(serde::de::Error::invalid_value("expected a variant")),
                    }
                }
        }

        deserializer.deserialize_struct_field(DirectionFieldVisitor)
    }
}
