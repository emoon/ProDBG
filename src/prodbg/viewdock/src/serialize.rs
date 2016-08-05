extern crate serde;

use {DockHandle, Workspace};
use super::Rect;

// Serialization

gen_handle!("DockHandle", DockHandle, DockHandleVisitor);

impl serde::ser::Serialize for Workspace {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer
    {
        serializer.serialize_struct("", WorkspaceMapVisitor { value: self }).map(|_| ())
    }
}

struct WorkspaceMapVisitor<'a> {
    value: &'a Workspace,
}

impl<'a> serde::ser::MapVisitor for WorkspaceMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
        where S: serde::Serializer
    {
        try!(serializer.serialize_struct_elt("root_area", &self.value.root_area));
        try!(serializer.serialize_struct_elt("handle_counter", &self.value.handle_counter));
        Ok(None)
    }
}

// Deserialization

gen_struct_deserializer!(Workspace;
    root_area => "root_area", RootArea,
    handle_counter => "handle_counter", HandleCounter;
    rect => Rect::default()
);
