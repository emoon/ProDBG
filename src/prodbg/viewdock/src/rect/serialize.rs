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
gen_struct_deserializer!(Rect;
    x => "x", X,
    y => "y", Y,
    width => "width", Width,
    height => "height", Height;
);

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
