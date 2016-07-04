extern crate serde;
use super::Container;
use rect::Rect;

// Serialization

impl serde::ser::Serialize for Container {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
        serializer.serialize_struct("Container", ContainerMapVisitor { value: self }).map(|_| ())
    }
}

struct ContainerMapVisitor<'a> {
    value: &'a Container
}

impl<'a> serde::ser::MapVisitor for ContainerMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error> where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("docks", &self.value.docks));
        try!(serializer.serialize_struct_elt("active_dock", &self.value.active_dock));
        Ok(None)
    }
}

// Deserialization

impl serde::Deserialize for Container {
    fn deserialize<D>(deserializer: &mut D) -> Result<Container, D::Error> where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["docks"];
        deserializer.deserialize_struct("Container", FIELDS, ContainerVisitor)
    }
}

struct ContainerVisitor;

impl serde::de::Visitor for ContainerVisitor {
    type Value = Container;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Container, V::Error> where V: serde::de::MapVisitor {
        let mut docks = None;
        let mut active_dock = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(ContainerField::Docks) => { docks = Some(try!(visitor.visit_value())); }
                Some(ContainerField::ActiveDock) => { active_dock = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let docks = match docks {
            Some(docks) => docks,
            None => try!(visitor.missing_field("docks")),
        };

        let active_dock = match active_dock {
            Some(active_dock) => active_dock,
            None => try!(visitor.missing_field("active_dock")),
        };

        try!(visitor.end());

        Ok(Container {
            docks: docks,
            tab_borders: vec!(0.0),
            active_dock: active_dock,
            rect: Rect::default(), // We use default here as this is always recalculated
        })
    }
}

enum ContainerField {
    Docks,
    ActiveDock
}

impl serde::Deserialize for ContainerField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<ContainerField, D::Error> where D: serde::de::Deserializer {
        struct ContainerFieldVisitor;

        impl serde::de::Visitor for ContainerFieldVisitor {
            type Value = ContainerField;

            fn visit_str<E>(&mut self, value: &str) -> Result<ContainerField, E>
                where E: serde::de::Error {
                    match value {
                        "docks" => Ok(ContainerField::Docks),
                        "active_dock" => Ok(ContainerField::ActiveDock),
                        _ => Err(serde::de::Error::custom("expected docks")),
                    }
                }
        }

        deserializer.deserialize(ContainerFieldVisitor)
    }
}
