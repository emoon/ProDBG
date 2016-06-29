extern crate serde;
use super::{Split, SplitHandle};
use rect::Rect;

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

// Serialization

impl serde::ser::Serialize for Split {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
        serializer.serialize_struct("Split", SplitMapVisitor { value: self }).map(|_| ())
    }
}

struct SplitMapVisitor<'a> {
    value: &'a Split
}

impl<'a> serde::ser::MapVisitor for SplitMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error> where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("children", &self.value.children));
        try!(serializer.serialize_struct_elt("ratios", &self.value.ratios));
        try!(serializer.serialize_struct_elt("direction", &self.value.direction));
        try!(serializer.serialize_struct_elt("handle", &self.value.handle));
        Ok(None)
    }
}

// Deserialization

impl serde::Deserialize for Split {
    fn deserialize<D>(deserializer: &mut D) -> Result<Split, D::Error> where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &[ "left", "right", "left_docks", "right_docks", "ratio", "direction", "handle"];
        deserializer.deserialize_struct("Split", FIELDS, SplitVisitor)
    }
}

struct SplitVisitor;

impl serde::de::Visitor for SplitVisitor {
    type Value = Split;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Split, V::Error> where V: serde::de::MapVisitor {
        let mut children = None;
        let mut ratios = None;
        let mut direction = None;
        let mut handle = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(SplitField::Children) => { children = Some(try!(visitor.visit_value())); }
                Some(SplitField::Ratios) => { ratios = Some(try!(visitor.visit_value())); }
                Some(SplitField::Direction) => { direction = Some(try!(visitor.visit_value())); }
                Some(SplitField::Handle) => { handle = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let children = match children {
            Some(right_docks) => right_docks,
            None => try!(visitor.missing_field("right_docks")),
        };

        let ratios = match ratios {
            Some(ratio) => ratio,
            None => try!(visitor.missing_field("ratios")),
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
            children: children,
            ratios: ratios,
            direction: direction,
            handle: handle,
            rect: Rect::default(), // reconstructed during update
        })
    }
}

enum SplitField {
    Children,
    Ratios,
    Direction,
    Handle,
}

impl serde::Deserialize for SplitField {
    fn deserialize<D>(deserializer: &mut D) -> Result<SplitField, D::Error> where D: serde::de::Deserializer {
        struct SplitFieldVisitor;

        impl serde::de::Visitor for SplitFieldVisitor {
            type Value = SplitField;

            fn visit_str<E>(&mut self, value: &str) -> Result<SplitField, E>
                where E: serde::de::Error {
                    match value {
                        "children" => Ok(SplitField::Children),
                        "ratios" => Ok(SplitField::Ratios),
                        "direction" => Ok(SplitField::Direction),
                        "handle" => Ok(SplitField::Handle),
                        _ => Err(serde::de::Error::custom("expected left, right, left_docks, right_docs, ratio, direction or handle")),
                    }
                }
        }

        deserializer.deserialize(SplitFieldVisitor)
    }
}
