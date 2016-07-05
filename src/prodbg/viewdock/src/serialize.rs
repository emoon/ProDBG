extern crate serde;

use Workspace;
use super::Rect;

// Serialization

impl serde::ser::Serialize for Workspace {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
        serializer.serialize_struct("", WorkspaceMapVisitor { value: self }).map(|_| ())
    }
}

struct WorkspaceMapVisitor<'a> {
    value: &'a Workspace
}

impl<'a> serde::ser::MapVisitor for WorkspaceMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error> where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("root_area", &self.value.root_area));
        try!(serializer.serialize_struct_elt("handle_counter", &self.value.handle_counter));
        Ok(None)
    }
}

// Deserialization

impl serde::Deserialize for Workspace {
    fn deserialize<D>(deserializer: &mut D) -> Result<Workspace, D::Error> where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["root_area", "handle_counter"];
        deserializer.deserialize_struct("Workspace", FIELDS, WorkspaceVisitor)
    }
}

struct WorkspaceVisitor;

impl serde::de::Visitor for WorkspaceVisitor {
    type Value = Workspace;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Workspace, V::Error> where V: serde::de::MapVisitor {
        let mut root_area = None;
        let mut handle_counter = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(WorkspaceField::RootArea) => { root_area = Some(try!(visitor.visit_value())); }
                Some(WorkspaceField::HandleCounter) => { handle_counter = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let root_area = match root_area {
            Some(root_area) => root_area,
            None => try!(visitor.missing_field("root_area")),
        };

        let handle_counter = match handle_counter {
            Some(handle_counter) => handle_counter,
            None => try!(visitor.missing_field("handle_counter")),
        };

        try!(visitor.end());

        Ok(Workspace {
            root_area: root_area,
            rect: Rect::default(),
            handle_counter: handle_counter,
        })
    }
}

enum WorkspaceField {
    RootArea,
    HandleCounter,
}

impl serde::Deserialize for WorkspaceField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<WorkspaceField, D::Error> where D: serde::de::Deserializer {
        struct WorkspaceFieldVisitor;

        impl serde::de::Visitor for WorkspaceFieldVisitor {
            type Value = WorkspaceField;

            fn visit_str<E>(&mut self, value: &str) -> Result<WorkspaceField, E>
                where E: serde::de::Error {
                    match value {
                        "root_area" => Ok(WorkspaceField::RootArea),
                        "handle_counter" => Ok(WorkspaceField::HandleCounter),
                        _ => Err(serde::de::Error::custom("expected root_area or handle_counter")),
                    }
                }
        }

        deserializer.deserialize(WorkspaceFieldVisitor)
    }
}
