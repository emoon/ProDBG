//! Helper macros to create serialization and deserialization serde code. It is handy if one does
//! not want to use code generators or nightly version of Rust (serde has plugin compiler for that).
//!
//! Crate `serde` should be imported in namespace. No macro introduces new names.


/// Generates code for newtype (tuple struct with one value).
///
/// # Examples
///
/// ```
/// # #[macro_use] extern crate serde_macros;
/// # extern crate serde;
/// # extern crate serde_json;
///
/// # fn main() {
///     #[derive(Debug, PartialEq)]
///     struct Handle(u64);
///     gen_newtype_code!(Handle);
///
///     let target = Handle(15u64);
///     let serialized = serde_json::to_string(&target).unwrap();
///     let deserialized: Handle = serde_json::from_str(&serialized).unwrap();
///     assert_eq!(target, deserialized);
/// # }
/// ```
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


/// Generates code for struct serialization.
///
/// Usage:
///
/// ```text
/// gen_struct_code!(struct_name,
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
/// # Examples
///
/// ```
/// # #[macro_use] extern crate serde_macros;
/// # extern crate serde;
/// # extern crate serde_json;
///
/// # fn main() {
///     #[derive(Debug)]
///     struct Workspace {
///         root_area: i64,
///         handle_counter: i32,
///         description: Option<String>,
///     };
///     gen_struct_code!(Workspace,
///         root_area, handle_counter;
///         description => None
///     );
///
///     let target = Workspace {
///         root_area: 15,
///         handle_counter: 5,
///         description: Some("some description".to_string())
///     };
///     let serialized = serde_json::to_string(&target).unwrap();
///     let deserialized: Workspace = serde_json::from_str(&serialized).unwrap();
///     assert_eq!(deserialized.root_area, target.root_area);
///     assert_eq!(deserialized.handle_counter, target.handle_counter);
///     assert_eq!(deserialized.description, None);
/// # }
/// ```

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
                                        &format!("Unexpected field name {}. Expected one of ({})",
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

/// Generates serialization and deserialization code for unit enum (consisting only of unit
/// variants).
///
/// Usage:
///
/// ```text
/// gen_unit_enum_code!(EnumName, Variant1 => index1, Variant2 => index2, ...);
/// ```
///
/// `index` is of type `usize`. It is part of `serde` enum serialization interface and is not used
/// in most of serializers.
///
/// # Examples
///
/// ```
/// # #[macro_use] extern crate serde_macros;
/// # extern crate serde;
/// # extern crate serde_json;
///
/// # fn main() {
///     #[derive(Debug, PartialEq)]
///     enum NumberRepresentation {
///         Hex,
///         UnsignedDecimal,
///     }
///     gen_unit_enum_code!(NumberRepresentation, Hex => 0, UnsignedDecimal => 1);
///
///     let target = NumberRepresentation::UnsignedDecimal;
///     let serialized = serde_json::to_string(&target).unwrap();
///     let deserialized: NumberRepresentation = serde_json::from_str(&serialized).unwrap();
///     assert_eq!(deserialized, target);
/// # }
/// ```
#[macro_export]
macro_rules! gen_unit_enum_code {
    ($name:ident, $($variant:ident => $index:expr),*) => {
        impl serde::Serialize for $name {
            fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error>
                where S: serde::Serializer
            {
                match *self {
                    $($name::$variant => {
                        serializer.serialize_unit_variant(stringify!($name),
                                                          $index,
                                                          stringify!($variant))
                    },)*
                }
            }
        }

        impl serde::Deserialize for $name {
            fn deserialize<D>(deserializer: &mut D) -> Result<$name, D::Error>
                where D: serde::Deserializer
            {
                struct Visitor;

                impl serde::de::EnumVisitor for Visitor {
                    type Value = $name;

                    fn visit<V>(&mut self, mut visitor: V) -> Result<$name, V::Error>
                        where V: serde::de::VariantVisitor
                    {
                        match try!(visitor.visit_variant::<String>()).as_str() {
                            $(stringify!($variant) => {
                                try!(visitor.visit_unit());
                                Ok($name::$variant)
                            }),*
                            other => Err(serde::de::Error::invalid_value(
                                &format!("Unexpected variant name {}. Expected one of ({})",
                                         other,
                                         concat!($(stringify!($variant), ", "),*)
                                 )
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

/// Generates serialization and deserialization code for newtype enum (consisting only of variants
/// with one value).
///
/// Usage:
///
/// ```text
/// gen_newtype_enum_code!(EnumName, Variant1 => index1, Variant2 => index2, ...);
/// ```
///
/// `index` is of type `usize`. It is part of `serde` enum serialization interface and is not used
/// in most of serializers.
///
/// # Examples
///
/// ```
/// # #[macro_use] extern crate serde_macros;
/// # extern crate serde;
/// # extern crate serde_json;
///
/// # fn main() {
///     #[derive(Debug, PartialEq)]
///     enum Area {
///         Integer(i64),
///         Float(f64),
///     }
///     gen_newtype_enum_code!(Area, Integer => 0, Float => 1);
///
///     let target = Area::Integer(23);
///     let serialized = serde_json::to_string(&target).unwrap();
///     let deserialized: Area = serde_json::from_str(&serialized).unwrap();
///     assert_eq!(deserialized, target);
/// # }
/// ```
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
            fn deserialize<D>(deserializer: &mut D) -> Result<$name, D::Error>
                where D: serde::Deserializer
            {
                struct Visitor;

                impl serde::de::EnumVisitor for Visitor {
                    type Value = $name;

                    fn visit<V>(&mut self, mut visitor: V) -> Result<$name, V::Error>
                        where V: serde::de::VariantVisitor
                    {
                        match try!(visitor.visit_variant::<String>()).as_str() {
                            $(stringify!($variant) => Ok($name::$variant(try!(visitor.visit_newtype()))),)*
                            other => Err(serde::de::Error::invalid_value(
                                        &format!("Unexpected variant name {}. Expected one of ({})",
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


#[cfg(test)]
mod test {
    extern crate serde;
    extern crate serde_json;

    #[test]
    fn test_gen_struct_code_fails_if_miss_field() {
        let serialized = {
            #[derive(Debug)]
            struct Test {
                a: i32,
            }
            gen_struct_code!(Test, a;);
            let target = Test {a: 0};
            serde_json::to_string(&target).unwrap()
        };
        #[derive(Debug)]
        struct Test {
            a: i32,
            b: i32,
        }
        gen_struct_code!(Test, a, b;);

        let res = serde_json::from_str::<Test>(&serialized);
        assert!(res.is_err());
    }

    #[test]
    fn test_gen_struct_code_fails_if_more_field() {
        let serialized = {
            #[derive(Debug)]
            struct Test {
                a: i32,
                b: i32,
                c: i32,
            }
            gen_struct_code!(Test, a, b, c;);
            let target = Test {a: 0, b: 0, c: 0};
            serde_json::to_string(&target).unwrap()
        };
        #[derive(Debug)]
        struct Test {
            a: i32,
            b: i32,
        }
        gen_struct_code!(Test, a, b;);

        let res = serde_json::from_str::<Test>(&serialized);
        assert!(res.is_err());
    }

    #[test]
    fn test_gen_unit_enum_code_fails_if_bad_variant() {
        let serialized = {
            #[derive(Debug)]
            enum Test {
                A,
                B,
                C,
            }
            gen_unit_enum_code!(Test, A => 0, B => 1, C => 1);
            let target = Test::C;
            serde_json::to_string(&target).unwrap()
        };
        #[derive(Debug)]
        enum Test {
            A,
            B,
        }
        gen_unit_enum_code!(Test, A => 0, B => 1);

        let res = serde_json::from_str::<Test>(&serialized);
        assert!(res.is_err());
    }

    #[test]
    fn test_gen_newtype_enum_code_fails_if_bad_variant() {
        let serialized = {
            #[derive(Debug)]
            enum Test {
                A(i32),
                B(i32),
                C(i32),
            }
            gen_newtype_enum_code!(Test, A => 0, B => 1, C => 1);
            let target = Test::C(15);
            serde_json::to_string(&target).unwrap()
        };
        #[derive(Debug)]
        enum Test {
            A(i32),
            B(i32),
        }
        gen_newtype_enum_code!(Test, A => 0, B => 1);

        let res = serde_json::from_str::<Test>(&serialized);
        assert!(res.is_err());
    }
}