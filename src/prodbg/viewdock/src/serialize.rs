extern crate serde;

use {SplitHandle, Direction, Split, Workspace};

struct WorkspaceMapVisitor<'a> {
    value: &'a Workspace
}

impl<'a> serde::ser::MapVisitor for WorkspaceMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error> where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("splits", &self.value.splits));
        try!(serializer.serialize_struct_elt("rect", &self.value.rect));
        try!(serializer.serialize_struct_elt("window_border", &self.value.window_border));
        try!(serializer.serialize_struct_elt("handle_counter", &self.value.handle_counter));
        Ok(None)
    }
}

impl serde::ser::Serialize for Workspace {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
        serializer.serialize_struct("", WorkspaceMapVisitor { value: self }).map(|_| ())
    }
}

enum WorkspaceField {
    Splits,
    Rect,
    WindowBorder,
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
                        "splits" => Ok(WorkspaceField::Splits),
                        "rect" => Ok(WorkspaceField::Rect),
                        "window_border" => Ok(WorkspaceField::WindowBorder),
                        "handle_counter" => Ok(WorkspaceField::HandleCounter),
                        _ => Err(serde::de::Error::custom("expected splits,rect,window_border or handle_counter")),
                    }
                }
        }

        deserializer.deserialize(WorkspaceFieldVisitor)
    }
}

struct WorkspaceVisitor;

impl serde::Deserialize for Workspace {
    fn deserialize<D>(deserializer: &mut D) -> Result<Workspace, D::Error> where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["splits", "rect", "window_border", "handle_counter"];
        deserializer.deserialize_struct("Workspace", FIELDS, WorkspaceVisitor)
    }
}

impl serde::de::Visitor for WorkspaceVisitor {
    type Value = Workspace;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Workspace, V::Error> where V: serde::de::MapVisitor {
        let mut splits = None;
        let mut rect = None;
        let mut window_border = None;
        let mut handle_counter = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(WorkspaceField::Splits) => { splits = Some(try!(visitor.visit_value())); }
                Some(WorkspaceField::Rect) => { rect = Some(try!(visitor.visit_value())); }
                Some(WorkspaceField::WindowBorder) => { window_border = Some(try!(visitor.visit_value())); }
                Some(WorkspaceField::HandleCounter) => { handle_counter = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let splits = match splits {
            Some(splits) => splits,
            None => try!(visitor.missing_field("splits")),
        };

        let rect = match rect {
            Some(rect) => rect,
            None => try!(visitor.missing_field("rect")),
        };

        let window_border = match window_border {
            Some(window_border) => window_border,
            None => try!(visitor.missing_field("window_border")),
        };

        let handle_counter = match handle_counter {
            Some(handle_counter) => handle_counter,
            None => try!(visitor.missing_field("handle_counter")),
        };

        try!(visitor.end());

        Ok(Workspace {
            splits: splits,
            rect: rect,
            window_border: window_border,
            handle_counter: handle_counter,
        })
    }
}

