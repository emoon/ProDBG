extern crate serde;
use super::Area;

// Serialization

impl serde::ser::Serialize for Area {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
        match *self {
            Area::Container(ref c) => serde::ser::Serializer::serialize_newtype_variant(serializer, "Area", 0usize, "Container", c),
            Area::Split(ref s) => serde::ser::Serializer::serialize_newtype_variant(serializer, "Area", 1usize, "Split", s),
        }
    }
}

// Deserialization of Direction

impl serde::Deserialize for Area {
    fn deserialize<D>(deserializer: &mut D) -> Result<Area, D::Error> where D: serde::de::Deserializer {
        const VARIANTS: &'static [&'static str] = &["Container", "Split"];
        deserializer.deserialize_enum("Direction", VARIANTS, AreaVisitor)
    }
}

struct AreaVisitor;

impl serde::de::EnumVisitor for AreaVisitor {
    type Value = Area;

    fn visit<V>(&mut self, mut visitor: V) -> Result<Area, V::Error> where V: serde::de::VariantVisitor {
        match try!(visitor.visit_variant()) {
            AreaField::Container => {
                let res = try!(visitor.visit_newtype());
                Ok(Area::Container(res))
            },
            AreaField::Split => {
                let res = try!(visitor.visit_newtype());
                Ok(Area::Split(res))
            }
        }
    }
}

enum AreaField {
    Container,
    Split,
}

impl serde::Deserialize for AreaField  {
    fn deserialize<D>(deserializer: &mut D) -> Result<AreaField, D::Error> where D: serde::de::Deserializer {
        struct AreaFieldVisitor;

        impl serde::de::Visitor for AreaFieldVisitor {
            type Value = AreaField;

            fn visit_usize<E>(&mut self, value: usize) -> Result<AreaField, E>
                where E: serde::de::Error {
                    match value {
                        0usize => Ok(AreaField::Container),
                        1usize => Ok(AreaField::Split),
                        _ => Err(serde::de::Error::invalid_value("expected a variant")),
                    }
                }

            fn visit_str<E>(&mut self, value: &str) -> Result<AreaField, E>
                where E: serde::de::Error {
                    match value {
                        "Container" => Ok(AreaField::Container),
                        "Split" => Ok(AreaField::Split),
                        _ => Err(serde::de::Error::invalid_value("expected a variant")),
                    }
                }
        }

        deserializer.deserialize_struct_field(AreaFieldVisitor)
    }
}
