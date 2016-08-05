/// Generates serialization code for tuple struct
#[macro_export]
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

/// Generates enum that deserializes strings into variants of this enum. Usage:
/// ```
/// gen_field_enum!(Enum, string => variant, string => variant);
/// ```
/// `Enum` is name of enum created;
/// `expr` is serialized variant
/// `variant` is variant of enum;
/// For example, following invocation:
/// ```
/// gen_field_enum!(MyVec, "x" => X, "y" => Y);
/// ```
/// will produce next code:
/// ```
/// enum MyVec {
///     X, Y
/// }
///
/// impl serde::Deserialize for MyVec  {
///     fn deserialize<D>(deserializer: &mut D) -> Result<$enum_name, D::Error> where D: serde::de::Deserializer {
///         struct CurrentVisitor;
///
///         impl serde::de::Visitor for CurrentVisitor {
///             type Value = MyVec;
///
///             fn visit_str<E>(&mut self, value: &str) -> Result<MyVec, E>
///                 where E: serde::de::Error {
///                     match value {
///                         "x" => Ok(MyVec::X),
///                         "y" => Ok(MyVec::Y),
///                         _ => Err(serde::de::Error::custom("Unexpected field name. Expected one of x, y, ")),
///                     }
///                 }
///         }
///         deserializer.deserialize(CurrentVisitor)
///     }
/// }
/// ```
#[macro_export]
macro_rules! gen_field_enum {
    ($enum_name: ident, $($value:expr => $variant:ident),*) => {
        enum $enum_name {
            $($variant,)*
        }

        impl serde::Deserialize for $enum_name  {
            fn deserialize<D>(deserializer: &mut D) -> Result<$enum_name, D::Error> where D: serde::de::Deserializer {
                struct CurrentVisitor;

                impl serde::de::Visitor for CurrentVisitor {
                    type Value = $enum_name;

                    fn visit_str<E>(&mut self, value: &str) -> Result<$enum_name, E>
                        where E: serde::de::Error {
                            match value {
                                $($value => Ok($enum_name::$variant),)*
                                _ => Err(serde::de::Error::custom(concat!("Unexpected field name. Expected one of ", $($value, ", "),*))),
                            }
                        }
                }
                deserializer.deserialize(CurrentVisitor)
            }
        }
    }
}


/// Generates code for struct deserialization. Usage:
/// ```
/// gen_struct_deserializer!(
///     struct_name;
///     field_name => "field_in_string", EnumVariant,
///     field_name2 => "field2_in_string", EnumVariant2, ...;
///     const_field_name => arbitrary_expression,
///     const_field_name2 => arbitrary_expression
/// );
/// ```
/// `struct_name` is deserialization target struct;
/// `field_name` is name of field that will be deserialized;
/// `field_in_string` is serialized name of field;
/// `EnumVariant` is needed to generate enum that will deserialize field names;
/// `const_field_name` is name of field that will not be deserialized;
/// `arbitrary_expression` is value for `const_field_name` in struct constructor;
///
/// For example, following invocation:
/// ```
/// gen_struct_deserializer!(Workspace;
///     root_area => "root_area", RootArea,
///     handle_counter => "handle_counter", HandleCounter;
///     rect => Rect::default()
/// );
/// ```
/// will produce next code:
/// ```
/// impl serde::Deserialize for Workspace {
///     fn deserialize<D>(deserializer: &mut D) -> Result<Workspace, D::Error> where D: serde::de::Deserializer {
///         static FIELDS: &'static [&'static str] = &["root_area", "handle_counter"];
///         deserializer.deserialize_struct("Workspace", FIELDS, Visitor)
///     }
/// }
///
/// struct Visitor;
///
/// impl serde::de::Visitor for Visitor {
///     type Value = Workspace;
///
///     fn visit_map<V>(&mut self, mut visitor: V) -> Result<Workspace, V::Error> where V: serde::de::MapVisitor {
///         let mut root_area = None;
///         let mut handle_counter = None;
///
///         loop {
///             match try!(visitor.visit_key()) {
///                 Some(WorkspaceField::RootArea) => { root_area = Some(try!(visitor.visit_value())); }
///                 Some(WorkspaceField::HandleCounter) => { handle_counter = Some(try!(visitor.visit_value())); }
///                 None => { break; }
///             }
///         }
///
///         let root_area = match root_area {
///             Some(root_area) => root_area,
///             None => try!(visitor.missing_field("root_area")),
///         };
///
///         let handle_counter = match handle_counter {
///             Some(handle_counter) => handle_counter,
///             None => try!(visitor.missing_field("handle_counter")),
///         };
///
///         try!(visitor.end());
///
///         Ok(Workspace {
///             root_area: root_area,
///             handle_counter: handle_counter,
///             rect: Rect::default(),
///         })
///     }
/// }
/// ```
#[macro_export]
macro_rules! gen_struct_deserializer {
    ($name:ident; $($field:ident => $string:expr, $enum_name:ident),*; $($const_field:ident => $value:expr)*) => {
        impl serde::Deserialize for $name {
            fn deserialize<D>(deserializer: &mut D) -> Result<$name, D::Error> where D: serde::de::Deserializer {
                gen_field_enum!(Field, $($string => $enum_name),*);

                struct Visitor;

                impl serde::de::Visitor for Visitor {
                    type Value = $name;

                    fn visit_map<V>(&mut self, mut visitor: V) -> Result<$name, V::Error> where V: serde::de::MapVisitor {
                        $(let mut $field = None;)*

                        loop {
                            match try!(visitor.visit_key()) {
                                $(Some(Field::$enum_name) => { $field = Some(try!(visitor.visit_value())); },)*
                                None => { break; }
                            }
                        }

                        $(let $field = match $field {
                            Some($field) => $field,
                            None => try!(visitor.missing_field($string)),
                        };)*

                        try!(visitor.end());

                        Ok($name {
                            $($field: $field,)*
                            $($const_field: $value,)*
                        })
                    }
                }

                static FIELDS: &'static [&'static str] = &[$($string,)*];
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
        impl serde::ser::Serialize for $name {
            fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
                where S: serde::ser::Serializer
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
                where D: serde::de::Deserializer
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
macro_rules! gen_struct_serializer {
    ($name:ident, $($field:ident),*) => {
        impl serde::ser::Serialize for $name {
            fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
                where S: serde::ser::Serializer
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
    }
}
