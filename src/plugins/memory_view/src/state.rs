use number_view::{NumberView, NumberRepresentation, NumberSize, Endianness};
use serde;
use serde_json;

pub struct MemoryViewState {
    pub start_address: usize,
    pub columns: usize,
    pub number_view: Option<NumberView>,
    pub text_shown: bool,
}

impl MemoryViewState {
    pub fn to_string(&self) -> String {
        serde_json::to_string(self).unwrap()
    }

    pub fn from_str(source: &str) -> Result<MemoryViewState, serde_json::Error> {
        serde_json::from_str(source)
    }
}

gen_struct_serializer!(NumberView, representation, size, endianness);
gen_struct_deserializer!(NumberView;
    representation => "representation", Representation,
    size => "size", Size,
    endianness => "endianness", Endianness;
);

gen_plain_enum_code!(NumberRepresentation, Hex, UnsignedDecimal, SignedDecimal, Float);
gen_plain_enum_code!(NumberSize, OneByte, TwoBytes, FourBytes, EightBytes);
gen_plain_enum_code!(Endianness, Little, Big);

gen_struct_serializer!(MemoryViewState, start_address, columns, number_view, text_shown);
gen_struct_deserializer!(MemoryViewState;
    start_address => "start_address", StartAddress,
    columns => "columns", Columns,
    number_view => "number_view", NumberView,
    text_shown => "text_shown", TextShown;
);