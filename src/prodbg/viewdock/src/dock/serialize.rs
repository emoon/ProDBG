extern crate serde;
use super::{Dock, DockHandle};

gen_handle!("DockHandle", DockHandle, DockHandleVisitor);

// Serialization

impl serde::ser::Serialize for Dock {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
        serializer.serialize_struct("Dock", DockMapVisitor { value: self }).map(|_| ())
    }
}

struct DockMapVisitor<'a> {
    value: &'a Dock
}

impl<'a> serde::ser::MapVisitor for DockMapVisitor<'a> {
    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error> where S: serde::Serializer {
        try!(serializer.serialize_struct_elt("handle", &self.value.handle));
        try!(serializer.serialize_struct_elt("plugin_name", &self.value.plugin_name));
        try!(serializer.serialize_struct_elt("plugin_data", &self.value.plugin_data));
        Ok(None)
    }
}

// Deserialization
gen_struct_deserializer!(Dock;
    handle => "handle", Handle,
    plugin_name => "plugin_name", PluginName,
    plugin_data => "plugin_data", PluginData;
);