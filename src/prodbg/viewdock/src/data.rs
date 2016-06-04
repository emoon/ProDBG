extern crate serde;

use Rect;

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


