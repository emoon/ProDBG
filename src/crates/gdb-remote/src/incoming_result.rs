use std::str;

pub struct IncomingResult<'a> {
    pub data: &'a [u8],
}

impl<'a> IncomingResult<'a> {
    pub fn begins_with(&self, name: &str) -> Option<&[u8]> {
        let len = name.len();

        // Make sure name fits with the data
        if len > self.data.len() {
            return None;
        }

        for vals in self.data.iter().zip(name.as_bytes().iter()) {
            if vals.0 != vals.1 {
                return None;
            }
        }

        Some(&self.data[len..])
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ok_string() {
        let res = IncomingResult { data: b"QTest123456" };
        let t = res.begins_with("QTest").unwrap();
        assert_eq!(t[0], b'1');
        assert_eq!(t[1], b'2');
    }

    #[test]
    fn test_too_long_in_string() {
        let res = IncomingResult { data: b"S10" };
        assert_eq!(true, res.begins_with("TooLongString").is_none());
    }

    #[test]
    fn test_same_length() {
        let res = IncomingResult { data: b"S10" };
        assert_eq!(true, res.begins_with("S10").is_some());
    }

    #[test]
    fn test_bad_data() {
        let res = IncomingResult { data: &[0,244,239] };
        assert_eq!(true, res.begins_with("S").is_none());
    }
}
