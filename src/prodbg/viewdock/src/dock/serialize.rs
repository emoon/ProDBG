extern crate serde;
use super::{Dock, DockHandle};
use rect::Rect;

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

impl serde::Deserialize for Dock {
    fn deserialize<D>(deserializer: &mut D) -> Result<Dock, D::Error> where D: serde::de::Deserializer {
        static FIELDS: &'static [&'static str] = &["handle", "plugin_name", "plugin_data"];
        deserializer.deserialize_struct("Dock", FIELDS, DockVisitor)
    }
}

struct DockVisitor;

impl serde::de::Visitor for DockVisitor {
    type Value = Dock;

    fn visit_map<V>(&mut self, mut visitor: V) -> Result<Dock, V::Error> where V: serde::de::MapVisitor {
        let mut handle = None;
        let mut plugin_name = None;
        let mut plugin_data = None;

        loop {
            match try!(visitor.visit_key()) {
                Some(DockField::Handle) => { handle = Some(try!(visitor.visit_value())); }
                Some(DockField::PluginName) => { plugin_name = Some(try!(visitor.visit_value())); }
                Some(DockField::PluginData) => { plugin_data = Some(try!(visitor.visit_value())); }
                None => { break; }
            }
        }

        let handle = match handle {
            Some(handle) => handle,
            None => try!(visitor.missing_field("handle")),
        };

        let plugin_name = match plugin_name {
            Some(plugin_name) => plugin_name,
            None => try!(visitor.missing_field("plugin_name")),
        };

        let plugin_data = match plugin_data {
            Some(plugin_data) => plugin_data,
            None => try!(visitor.missing_field("plugin_data")),
        };

        try!(visitor.end());

        Ok(Dock {
            handle: handle,
            plugin_name: plugin_name,
            plugin_data: plugin_data,
            rect: Rect::default(), // We use default here as this is always recalculated
        })
    }
}

enum DockField {
    Handle,
    PluginName,
    PluginData,
}

impl serde::Deserialize for DockField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<DockField, D::Error> where D: serde::de::Deserializer {
        struct DockFieldVisitor;

        impl serde::de::Visitor for DockFieldVisitor {
            type Value = DockField;

            fn visit_str<E>(&mut self, value: &str) -> Result<DockField, E>
                where E: serde::de::Error {
                    match value {
                        "handle" => Ok(DockField::Handle),
                        "plugin_name" => Ok(DockField::PluginName),
                        "plugin_data" => Ok(DockField::PluginData),
                        _ => Err(serde::de::Error::custom("expected handle,plugin_name or pluin_data")),
                    }
                }
        }

        deserializer.deserialize(DockFieldVisitor)
    }
}
