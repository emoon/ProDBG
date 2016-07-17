extern crate serde;
use super::{Split, SplitHandle};
use rect::Rect;

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

gen_struct_deserializer!(Split;
    children => "children", Children,
    ratios => "ratios", Ratios,
    direction => "direction", Direction,
    handle => "handle", Handle;
    rect => Rect::default()
);