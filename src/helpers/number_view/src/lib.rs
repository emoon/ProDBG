//! Structures describing number view:
//! * representation (hex, signed decimal, unsigned decimal, float)
//! * size (one to eight bytes)
//! * endianness (little-endian, big-endian)
//! Also capable of formatting memory (slice of u8) into such view.

#[cfg(feature = "serialization")]
#[macro_use]
extern crate serde_macros;
#[cfg(feature = "serialization")]
mod serialize;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct NumberView {
    pub representation: NumberRepresentation,
    pub size: NumberSize,
    pub endianness: Endianness,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum NumberRepresentation {
    Hex,
    UnsignedDecimal,
    SignedDecimal,
    Float,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum NumberSize {
    OneByte,
    TwoBytes,
    FourBytes,
    EightBytes,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Endianness {
    Little,
    Big,
}

impl NumberView {
    /// Maximum number of characters needed to show number
    // TODO: change to calculation from MAX/MIN when `const fn` is in stable Rust
    pub fn maximum_chars_needed(&self) -> usize {
        match self.representation {
            NumberRepresentation::Hex => self.size.byte_count() * 2,
            NumberRepresentation::UnsignedDecimal => {
                match self.size {
                    NumberSize::OneByte => 3,
                    NumberSize::TwoBytes => 5,
                    NumberSize::FourBytes => 10,
                    NumberSize::EightBytes => 20,
                }
            }
            NumberRepresentation::SignedDecimal => {
                match self.size {
                    NumberSize::TwoBytes => 6,
                    NumberSize::OneByte => 4,
                    NumberSize::FourBytes => 11,
                    NumberSize::EightBytes => 20,
                }
            }
            NumberRepresentation::Float => {
                match self.size {
                    NumberSize::FourBytes => 14,
                    NumberSize::EightBytes => 23,
                    _ => 5, // For "Error" message
                }
            }
        }
    }

    /// Format memory. Returns "Error" if representation and size do not match (one- and two-bytes
    /// float currently).
    /// # Panics
    /// Panics if slice of memory is less than number size.
    pub fn format(&self, buffer: &[u8]) -> String {
        macro_rules! format_buffer {
            ($data_type:ty, $len:expr, $endianness:expr, $format:expr) => {
                unsafe {
                    if buffer.len() < $len {
                        panic!("Could not convert buffer of length {} into data type of size {}", buffer.len(), $len);
                    }
                    let num_ref: &$data_type = std::mem::transmute(buffer.as_ptr());
                    let num = match $endianness {
                        Endianness::Little => num_ref.to_le(),
                        Endianness::Big => num_ref.to_be(),
                    };
                    format!($format, num)
                }
            };
            // m for manual endianness swap
            (m: $data_type:ty, $len:expr, $endianness:expr, $max_len: expr, $format:expr) => {
                unsafe {
                    if buffer.len() < $len {
                        panic!("Could not convert buffer of length {} into data type of size {}", buffer.len(), $len);
                    }
                    let mut rev_buf: [u8; $len] = [0; $len];
                    let buf = if $endianness == Endianness::default() {
                        buffer.as_ptr()
                    } else {
                        for (i, byte) in buffer.iter().enumerate() {
                            rev_buf[$len - i - 1] = *byte;
                        }
                        rev_buf.as_ptr()
                    };
                    let num_ref: &$data_type = std::mem::transmute(buf);
                    let power = num_ref.abs().log(10.0);
                    let mut power_digits = std::cmp::max(1, (power.abs() + 1.0).log(10.0).abs().ceil() as usize);
                    if power < 0.0 && power.is_normal() {
                        power_digits += 1;
                    };
                    // Formatting floats as -?.?????e??
                    // Four symbols are for -?._____e__
                    // Amount of symbols occupied by power is power_digits
                    // Everything else is precision
                    format!($format, (($max_len - 4) as usize).saturating_sub(power_digits), num_ref)
                }
            };
        }
        match self.representation {
            NumberRepresentation::Hex => {
                match self.size {
                    NumberSize::OneByte => format_buffer!(u8, 1, self.endianness, "{:02x}"),
                    NumberSize::TwoBytes => format_buffer!(u16, 2, self.endianness, "{:04x}"),
                    NumberSize::FourBytes => format_buffer!(u32, 4, self.endianness, "{:08x}"),
                    NumberSize::EightBytes => format_buffer!(u64, 8, self.endianness, "{:016x}"),
                }
            }
            NumberRepresentation::UnsignedDecimal => {
                match self.size {
                    NumberSize::OneByte => format_buffer!(u8, 1, self.endianness, "{:3}"),
                    NumberSize::TwoBytes => format_buffer!(u16, 2, self.endianness, "{:5}"),
                    NumberSize::FourBytes => format_buffer!(u32, 4, self.endianness, "{:10}"),
                    NumberSize::EightBytes => format_buffer!(u64, 8, self.endianness, "{:20}"),
                }
            }
            NumberRepresentation::SignedDecimal => {
                match self.size {
                    NumberSize::OneByte => format_buffer!(i8, 1, self.endianness, "{:4}"),
                    NumberSize::TwoBytes => format_buffer!(i16, 2, self.endianness, "{:6}"),
                    NumberSize::FourBytes => format_buffer!(i32, 4, self.endianness, "{:11}"),
                    NumberSize::EightBytes => format_buffer!(i64, 8, self.endianness, "{:20}"),
                }
            }
            NumberRepresentation::Float => {
                match self.size {
                    NumberSize::FourBytes => format_buffer!(m: f32, 4, self.endianness, 14, "{:14.*e}"),
                    NumberSize::EightBytes => format_buffer!(m: f64, 8, self.endianness, 23, "{:23.*e}"),
                    // Should never be available to pick through user interface
                    _ => return "Error".to_owned(),
                }
            }
        }
    }

    /// Changes number representation and picks default size if current size do not match new
    /// representation.
    pub fn change_representation(&mut self, representation: NumberRepresentation) {
        self.representation = representation;
        if !representation.can_be_of_size(self.size) {
            self.size = representation.get_default_size();
        }
    }
}

impl Default for NumberView {
    fn default() -> NumberView {
        NumberView {
            representation: NumberRepresentation::Hex,
            size: NumberSize::OneByte,
            endianness: Endianness::default(),
        }
    }
}

impl NumberSize {
    /// String representation of this `NumberSize`
    pub fn as_str(&self) -> &'static str {
        match *self {
            NumberSize::OneByte => "1 byte",
            NumberSize::TwoBytes => "2 bytes",
            NumberSize::FourBytes => "4 bytes",
            NumberSize::EightBytes => "8 bytes",
        }
    }

    /// Number of bytes represented by this `NumberSize`
    pub fn byte_count(&self) -> usize {
        match *self {
            NumberSize::OneByte => 1,
            NumberSize::TwoBytes => 2,
            NumberSize::FourBytes => 4,
            NumberSize::EightBytes => 8,
        }
    }

    /// String representing number of bits in this `NumberSize`
    pub fn as_bit_len_str(&self) -> &'static str {
        match *self {
            NumberSize::OneByte => "8",
            NumberSize::TwoBytes => "16",
            NumberSize::FourBytes => "32",
            NumberSize::EightBytes => "64",
        }
    }
}

static FLOAT_AVAILABLE_SIZES: [NumberSize; 2] = [NumberSize::FourBytes, NumberSize::EightBytes];
static OTHER_AVAILABLE_SIZES: [NumberSize; 4] =
    [NumberSize::OneByte, NumberSize::TwoBytes, NumberSize::FourBytes, NumberSize::EightBytes];
impl NumberRepresentation {
    pub fn can_be_of_size(&self, size: NumberSize) -> bool {
        match *self {
            NumberRepresentation::Float => {
                match size {
                    NumberSize::FourBytes => true,
                    NumberSize::EightBytes => true,
                    _ => false,
                }
            }
            _ => true,
        }
    }

    pub fn get_avaialable_sizes(&self) -> &'static [NumberSize] {
        match *self {
            NumberRepresentation::Float => &FLOAT_AVAILABLE_SIZES,
            _ => &OTHER_AVAILABLE_SIZES,
        }
    }

    pub fn get_default_size(&self) -> NumberSize {
        match *self {
            NumberRepresentation::Float => NumberSize::FourBytes,
            _ => NumberSize::OneByte,
        }
    }

    pub fn as_str(&self) -> &'static str {
        match *self {
            NumberRepresentation::Hex => "Hex",
            NumberRepresentation::UnsignedDecimal => "Unsigned decimal",
            NumberRepresentation::SignedDecimal => "Signed decimal",
            NumberRepresentation::Float => "Float",
        }
    }

    pub fn as_short_str(&self) -> &'static str {
        match *self {
            NumberRepresentation::Hex => "x",
            NumberRepresentation::UnsignedDecimal => "u",
            NumberRepresentation::SignedDecimal => "s",
            NumberRepresentation::Float => "f",
        }
    }
}

impl Endianness {
    pub fn as_str(&self) -> &'static str {
        match self {
            &Endianness::Little => "Little-endian",
            &Endianness::Big => "Big-endian",
        }
    }
}

impl Default for Endianness {
    fn default() -> Endianness {
        if cfg!(target_endian = "little") {
            Endianness::Little
        } else {
            Endianness::Big
        }
    }
}

#[cfg(test)]
mod test {
    use super::{NumberView, NumberRepresentation, NumberSize, Endianness};

    macro_rules! test_format {
        ($view:expr, $len:expr, $val:expr, $expected:expr) => {
            unsafe {
                let buf: &[u8; $len] = ::std::mem::transmute(&$val);
                assert_eq!($view.format(buf), $expected);
            }
        }
    }

    fn f32_view() -> NumberView {
        NumberView {
            representation: NumberRepresentation::Float,
            size: NumberSize::FourBytes,
            endianness: Endianness::default(),
        }
    }

    #[test]
    pub fn test_float_format() {
        let x = f32_view();
        test_format!(x, 4, ::std::f32::NAN, "           NaN");
        test_format!(x, 4, ::std::f32::MAX, " 3.40282347e38");
        test_format!(x, 4, ::std::f32::MIN, "-3.40282347e38");
        test_format!(x, 4, ::std::f32::MIN_POSITIVE, " 1.1754944e-38");
        test_format!(x, 4, -::std::f32::MIN_POSITIVE, "-1.1754944e-38");
        test_format!(x, 4, ::std::f32::INFINITY, "           inf");
        test_format!(x, 4, ::std::f32::NEG_INFINITY, "          -inf");
        test_format!(x, 4, 0.0 as f32, " 0.000000000e0");
        test_format!(x, 4, 1.0 as f32, " 1.000000000e0");
        test_format!(x, 4, -1.0 as f32, "-1.000000000e0");
        test_format!(x, 4, 18.123123123123 as f32,      " 1.812312317e1");
        test_format!(x, 4, 2135149841321351684981435135.0 as f32, " 2.13514983e27");
        test_format!(x, 4, -2135149841321351684981435135.0 as f32, "-2.13514983e27");
        test_format!(x, 4, 0.00081510584846516518146189681618168168181 as f32, " 8.15105857e-4");
        test_format!(x, 4, -0.00081510584846516518146189681618168168181 as f32, "-8.15105857e-4");
        test_format!(x, 4, 0.18 as f32, " 1.80000007e-1");
        test_format!(x, 4, -0.18 as f32, "-1.80000007e-1");
    }
}