extern crate serde;
use super::Container;
use rect::Rect;

// Serialization

impl serde::ser::Serialize for Container {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer
    {
        serializer.serialize_struct("Container", ContainerMapVisitor { value: self }).map(|_| ())
    }
}

struct ContainerMapVisitor<'a> {
    value: &'a Container,
}

impl<'a> serde::ser::MapVisitor for ContainerMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
        where S: serde::Serializer
    {
        try!(serializer.serialize_struct_elt("docks", &self.value.docks));
        try!(serializer.serialize_struct_elt("active_dock", &self.value.active_dock));
        Ok(None)
    }
}

// Deserialization

gen_struct_deserializer!(Container;
    docks => "docks", Docks,
    active_dock => "active_dock", ActiveDock;
    tab_borders => vec!(0.0)
    rect => Rect::default()
);
