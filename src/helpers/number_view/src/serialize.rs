extern crate serde;

use {NumberView, NumberRepresentation, NumberSize, Endianness};

gen_struct_code!(NumberView, representation, size, endianness;);

gen_unit_enum_code!(NumberRepresentation,
    Hex => 0,
    UnsignedDecimal => 1,
    SignedDecimal => 2,
    Float => 3
);
gen_unit_enum_code!(NumberSize, OneByte => 0, TwoBytes => 1, FourBytes => 2, EightBytes => 3);
gen_unit_enum_code!(Endianness, Little => 0, Big => 1);
