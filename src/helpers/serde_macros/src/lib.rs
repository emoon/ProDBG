//! Helper macros to create serialization and deserialization serde code. It is handy if one does
//! not want to use code generators or nightly version of Rust (serde has plugin compiler for that).


#[macro_export]
macro_rules! gen_newtype_code {
    ($name:ident) => {
        impl serde::Serialize for $name {
            fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
                where S: serde::Serializer {
                    serializer.serialize_newtype_struct(stringify!($name), &self.0)
                }
        }

        impl serde::Deserialize for $name  {
            fn deserialize<D>(deserializer: &mut D) -> Result<$name, D::Error> where D: serde::Deserializer {
                struct Visitor;

                impl serde::de::Visitor for Visitor {
                    type Value = $name;

                    fn visit_newtype_struct<D>(&mut self, deserializer: &mut D) -> Result<Self::Value, D::Error>
                        where D: serde::Deserializer {
                            let value = try!(serde::Deserialize::deserialize(deserializer));
                            Ok($name(value))
                        }
                }
                deserializer.deserialize_newtype_struct(stringify!($name), Visitor)
            }
        }

    }
}


/// Generates code for struct serialization. Usage:
/// ```
/// gen_struct_deserializer!(struct_name,
///     field_name, field_name2;
///     const_field_name => arbitrary_expression,
///     const_field_name2 => arbitrary_expression,
/// );
/// ```
/// `struct_name` is deserialization target struct;
/// `field_name` is name of field that will be serialized and deserialized;
/// `const_field_name` is name of field that will not be serialized. It is filled with
/// `arbitrary_expression` on deserialization instead;
///
/// For example, following invocation:
/// ```
/// gen_struct_deserializer!(Workspace,
///     root_area, handle_counter;
///     rect => Rect::default()
/// );
/// ```
/// will serialize `root_area`, `handle_counter`, but fill `rect` with `Rect::default()` on
/// deserialization
#[macro_export]
macro_rules! gen_struct_code {
    ($name:ident, $($field:ident),*; $($const_field:ident => $value:expr),*) => {
        impl serde::Serialize for $name {
            fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
                where S: serde::Serializer
            {
                struct Visitor<'a> {
                    value: &'a $name,
                }

                impl<'a> serde::ser::MapVisitor for Visitor<'a> {
                    fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error>
                        where S: serde::Serializer
                    {
                        $(try!(serializer.serialize_struct_elt(stringify!($field), &self.value.$field));)*
                        Ok(None)
                    }
                }

                serializer
                    .serialize_struct(stringify!($name), Visitor { value: self })
                    .map(|_| ())
            }
        }

        impl serde::Deserialize for $name {
            fn deserialize<D>(deserializer: &mut D) -> Result<$name, D::Error> where D: serde::Deserializer {
                struct Visitor;
                impl serde::de::Visitor for Visitor {
                    type Value = $name;

                    fn visit_map<V>(&mut self, mut visitor: V) -> Result<$name, V::Error> where V: serde::de::MapVisitor {
                        $(let mut $field = None;)*

                        loop {
                            match try!(serde::de::MapVisitor::visit_key::<String>(&mut visitor)) {
                                Some(value) => match value.as_str() {
                                    $(stringify!($field) => { $field = Some(try!(visitor.visit_value())); },)*
                                    other => {return Err(serde::de::Error::invalid_value(
                                        &format!("Unexpected field name {}. Expected one of {}",
                                                 other,
                                                 concat!($(stringify!($field), ", "),*))
                                    ))},
                                },
                                None => { break; },
                            }
                        }

                        $(let $field = match $field {
                            Some($field) => $field,
                            None => try!(visitor.missing_field(stringify!($field))),
                        };)*

                        try!(visitor.end());

                        Ok($name {
                            $($field: $field,)*
                            $($const_field: $value,)*
                        })
                    }
                }

                static FIELDS: &'static [&'static str] = &[$(stringify!($field)),*];
                deserializer.deserialize_struct(stringify!($name), FIELDS, Visitor)
            }
        }
    }
}

/// Generates serialization and deserialization code for plain enum (consisting only of unit
/// variants). Usage:
/// ```
/// gen_plain_enum_code!(EnumName, Variant1, Variant2, ...);
/// ```
/// For example,
/// ```
/// gen_plain_enum_code!(NumberRepresentation, Hex, UnsignedDecimal, SignedDecimal, Float);
/// ```
/// Enum is encoded as a single string equal to enum name passed. `EnumName::Variant1` will be
/// encoded as `Variant1`.
#[macro_export]
macro_rules! gen_plain_enum_code {
    ($name:ident, $($variant:ident),*) => {
        impl serde::Serialize for $name {
            fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
                where S: serde::Serializer
            {
                match *self {
                    $($name::$variant => {
                        serializer.serialize_str(stringify!($variant))
                    },)*
                }
            }
        }

        impl serde::Deserialize for $name {
            fn deserialize<D>(deserializer: &mut D) -> Result<$name, D::Error>
                where D: serde::Deserializer
            {
                struct Visitor;

                impl serde::de::Visitor for Visitor {
                    type Value = $name;

                    fn visit_str<E>(&mut self, value: &str) -> Result<$name, E>
                        where E: serde::de::Error
                    {
                        match value {
                            $(stringify!($variant) => Ok($name::$variant)),*,
                            _ => Err(serde::de::Error::invalid_value(
                                &format!("Unexpected variant name {}. Expected one of {}",
                                         value,
                                         concat!($(stringify!($variant), ", "),*)
                                 )
                            )),
                        }
                    }
                }

                deserializer.deserialize_struct_field(Visitor)
            }
        }
    }
}

#[macro_export]
macro_rules! gen_newtype_enum_code {
    ($name:ident, $($variant:ident => $index:expr),*) => {
        impl serde::Serialize for $name {
            fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
                where S: serde::Serializer
            {
                match *self {
                    $($name::$variant(ref value) => {
                        serializer.serialize_newtype_variant(stringify!($name),
                                                             $index,
                                                             stringify!($variant),
                                                             value
                        )
                    },)*
                }
            }
        }

        impl serde::Deserialize for $name {
            fn deserialize<D>(deserializer: &mut D) -> Result<Area, D::Error>
                where D: serde::Deserializer
            {
                struct Visitor;

                impl serde::de::EnumVisitor for Visitor {
                    type Value = $name;

                    fn visit<V>(&mut self, mut visitor: V) -> Result<Area, V::Error>
                        where V: serde::de::VariantVisitor
                    {
                        match try!(serde::de::VariantVisitor::visit_variant::<String>(&mut visitor)).as_str() {
                            $(stringify!($variant) => Ok($name::$variant(try!(visitor.visit_newtype()))),)*
                            other => Err(serde::de::Error::invalid_value(
                                        &format!("Unexpected field name {}. Expected one of {}",
                                                 other,
                                                 concat!($(stringify!($variant), ", "),*))
                            )),
                        }
                    }
                }

                const VARIANTS: &'static [&'static str] = &[$(stringify!($variant),)*];
                deserializer.deserialize_enum(stringify!($name), VARIANTS, Visitor)
            }
        }
    }
}