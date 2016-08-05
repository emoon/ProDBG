extern crate serde;
use super::Area;

// Serialization

impl serde::ser::Serialize for Area {
    fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
        where S: serde::ser::Serializer
    {
        match *self {
            Area::Container(ref c) => {
                serde::ser::Serializer::serialize_newtype_variant(serializer,
                                                                  "Area",
                                                                  0usize,
                                                                  "container",
                                                                  c)
            }
            Area::Split(ref s) => {
                serde::ser::Serializer::serialize_newtype_variant(serializer,
                                                                  "Area",
                                                                  1usize,
                                                                  "split",
                                                                  s)
            }
        }
    }
}

// Deserialization

impl serde::Deserialize for Area {
    fn deserialize<D>(deserializer: &mut D) -> Result<Area, D::Error>
        where D: serde::de::Deserializer
    {
        const VARIANTS: &'static [&'static str] = &["Container", "Split"];
        deserializer.deserialize_enum("Direction", VARIANTS, AreaVisitor)
    }
}

struct AreaVisitor;

impl serde::de::EnumVisitor for AreaVisitor {
    type Value = Area;

    fn visit<V>(&mut self, mut visitor: V) -> Result<Area, V::Error>
        where V: serde::de::VariantVisitor
    {
        match try!(visitor.visit_variant()) {
            AreaField::Container => {
                let res = try!(visitor.visit_newtype());
                Ok(Area::Container(res))
            }
            AreaField::Split => {
                let res = try!(visitor.visit_newtype());
                Ok(Area::Split(res))
            }
        }
    }
}

gen_field_enum!(AreaField, "container" => Container, "split" => Split);
