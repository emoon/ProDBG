//! Memory model as segment of inaccessible memory with single segment of accessible memory in it.

use ::std::iter::Iterator;
use ::std::cmp::{min, max};


/// Returns `Some((start, len))` for common part for two segments if `len` is more then zero.
fn intersect(first_start: usize,
             first_len: usize,
             second_start: usize,
             second_len: usize)
             -> Option<(usize, usize)> {
    let istart = max(first_start, second_start);
    let iend = min(first_start + first_len, second_start + second_len);
    if iend > istart {
        Some((istart, iend - istart))
    } else {
        None
    }
}

/// Structure to describe some desired chunk in memory. All memory from `start` to `start + len` is
/// considered inaccessible with only part of it (described by inner buffer) being accessible.
#[derive(Debug)]
pub struct MemoryChunk {
    /// Start of memory chunk
    start: usize,
    /// Number of bytes in memory chunk
    len: usize,
    /// Start of accessible bytes in memory
    bytes_start: usize,
    /// Accessible bytes
    bytes: Vec<u8>,
}

impl MemoryChunk {
    /// Constructs memory chunk of zero length
    pub fn new() -> MemoryChunk {
        MemoryChunk {
            start: 0,
            len: 0,
            bytes_start: 0,
            bytes: Vec::new(),
        }
    }

    /// Returns start of memory chunk
    pub fn start(&self) -> usize {
        self.start
    }

    /// Returns length of memory chunk
    pub fn len(&self) -> usize {
        self.len
    }

    /// Changes this chunk of memory to be from `start` to `len`. Reuses every possible byte of
    /// accessible memory.
    pub fn transform(&mut self, start: usize, len: usize) {
        self.start = start;
        self.len = len;
        let end = start + len;
        if end <= self.bytes_start {
            // No usable data in `self.bytes` left
            self.bytes_start = self.start + self.len;
            self.bytes.truncate(0);
        } else {
            self.bytes.truncate(end - self.bytes_start);
        }
        if self.bytes_start < self.start {
            let len_to_shrink = self.start - self.bytes_start;
            self.bytes = if len_to_shrink < self.bytes.len() {
                let new_len = self.bytes.len() - len_to_shrink;
                let mut res = Vec::with_capacity(new_len);
                res.extend_from_slice(&self.bytes[self.start - self.bytes_start..]);
                res
            } else {
                Vec::new()
            };
            self.bytes_start = self.start;
        }
    }

    /// Sets accessible memory
    pub fn set_accessible(&mut self, start: usize, bytes: &[u8]) {
        if let Some((common_start, common_len)) = intersect(self.start,
                                                            self.len,
                                                            start,
                                                            bytes.len()) {
            self.bytes.resize(common_len, 0);
            self.bytes_start = common_start;
            let offset = common_start - start;
            self.bytes.copy_from_slice(&bytes[offset..offset + common_len]);
        } else {
            self.bytes_start = self.start + self.len;
            self.bytes.truncate(0);
        }
    }

    /// Extends current accessible memory using provided. New data will only replace regions of
    /// memory previously inaccessible. Will do nothing if previous accessible memory and new
    /// accessible memory do not intersect or adjoin.
    pub fn extend_accessible(&mut self, start: usize, bytes: &[u8]) {
        if self.bytes.len() == 0 {
            self.set_accessible(start, bytes);
            return;
        }
        let mut res;
        {
            let mut parts: [&[u8]; 3] = [&[], &self.bytes, &[]];
            let end = start + bytes.len();
            let cur_bytes_end = self.bytes_start + self.bytes.len();
            if start < self.bytes_start && end >= self.bytes_start {
                let left_start = max(start, self.start);
                parts[0] = &bytes[left_start - start..self.bytes_start - start];
                self.bytes_start = left_start;
            }
            if start <= cur_bytes_end && end > cur_bytes_end {
                let right_end = min(end, self.start + self.len);
                parts[2] = &bytes[cur_bytes_end - start..right_end - start];
            }
            let size_sum = parts.iter().fold(0, |sum, part| sum + part.len());
            res = Vec::with_capacity(size_sum);
            for part in parts.iter() {
                (&mut res).extend_from_slice(part);
            }
        }
        self.bytes = res;
    }

    /// Returns a slice of memory. Allows to take slice even for indexes out of this memory chunk
    /// bounds.
    pub fn slice_mut(&mut self, start: usize, len: usize) -> MemorySlice {
        let (bytes_start, bytes) = intersect(start, len, self.bytes_start, self.bytes.len())
            .map(move |(common_start, common_len)| {
                let offset = common_start - self.bytes_start;
                (common_start, &mut self.bytes[offset..offset + common_len])
            })
            .unwrap_or((start.saturating_add(len), &mut []));
        MemorySlice::new(start, len, bytes_start, bytes)
    }
}


pub struct MemorySlice<'a> {
    /// Start of memory slice
    start: usize,
    /// Number of bytes in memory slice
    len: usize,
    /// Start of accessible bytes in memory
    bytes_start: usize,
    /// Accessible bytes
    bytes: &'a mut [u8],
}

impl<'a> MemorySlice<'a> {
    pub fn new(start: usize, len: usize, bytes_start: usize, bytes: &mut [u8]) -> MemorySlice {
        MemorySlice {
            start: start,
            len: len,
            bytes_start: bytes_start,
            bytes: bytes,
        }
    }

    /// Creates iterator that acts like `slice::chunks_mut`. For inaccessible or partially
    /// accessible chunks it will emit empty slice, for accessible bytes - slice of size `size`.
    pub fn chunks_mut(&mut self, size: usize) -> ChunksMut {
        ChunksMut::new(self.start, self.len, self.bytes_start, self.bytes, size)
    }
}


pub struct ChunksMut<'a> {
    cur_address: usize,
    end_address: usize,
    data_address: usize,
    data: ::std::slice::ChunksMut<'a, u8>,
    size: usize,
}

impl<'a> ChunksMut<'a> {
    pub fn new(start_address: usize,
               len: usize,
               data_address: usize,
               data: &'a mut [u8],
               size: usize)
               -> ChunksMut<'a> {
        let offset = if data_address > start_address {
            (size - (data_address - start_address) % size) % size
        } else {
            (start_address - data_address) % size
        };
        let iter = if offset < data.len() {
            data[offset..].chunks_mut(size)
        } else {
            [].chunks_mut(size)
        };
        ChunksMut {
            cur_address: start_address,
            end_address: start_address + len,
            data_address: data_address + offset,
            data: iter,
            size: size,
        }
    }
}

impl<'a> Iterator for ChunksMut<'a> {
    type Item = &'a mut [u8];

    fn next(&mut self) -> Option<Self::Item> {
        if self.cur_address >= self.end_address {
            return None;
        }
        let res: Option<Self::Item> = if self.cur_address < self.data_address {
            Some(&mut [])
        } else {
            Some(self.data.next().unwrap_or(&mut []))
        };
        self.cur_address += self.size;
        res
    }
}

#[cfg(test)]
mod test {
    macro_rules! assert_chunk {
        ($chunk:expr, $start:expr, $len:expr, $bytes_start:expr, $bytes:expr) => {
            assert_eq!($chunk.start, $start);
            assert_eq!($chunk.len, $len);
            assert_eq!($chunk.bytes_start, $bytes_start);
            assert_eq!($chunk.bytes, $bytes);
        };
        ($chunk:expr, $start:expr, $len:expr, empty) => {
            assert_eq!($chunk.start, $start);
            assert_eq!($chunk.len, $len);
            assert!($chunk.bytes.is_empty());
            assert!($chunk.bytes_start >= $chunk.start);
            assert!($chunk.bytes_start <= $chunk.start + $chunk.len);
        };
    }
    use super::MemoryChunk;

    fn chunk(start: usize, len: usize, bytes_start: usize, bytes: Vec<u8>) -> MemoryChunk {
        MemoryChunk {
            start: start,
            len: len,
            bytes_start: bytes_start,
            bytes: bytes,
        }
    }

    #[test]
    pub fn test_transform_shrinks_bytes_from_end_partially() {
        let mut chunk = chunk(10, 10, 15, vec![1, 2, 3]);
        chunk.transform(5, 12);
        assert_chunk!(chunk, 5, 12, 15, [1, 2]);
    }

    #[test]
    pub fn test_transform_shrinks_bytes_from_end_fully() {
        let mut chunk = chunk(10, 10, 15, vec![1, 2, 3]);
        chunk.transform(10, 5);
        assert_chunk!(chunk, 10, 5, empty);
    }

    #[test]
    pub fn test_transform_shrinks_bytes_from_start_partially() {
        let mut chunk = chunk(10, 10, 15, vec![1, 2, 3]);
        chunk.transform(16, 10);
        assert_chunk!(chunk, 16, 10, 16, [2, 3]);
    }

    #[test]
    pub fn test_transform_shrinks_bytes_from_start_fully() {
        let mut chunk = chunk(10, 10, 15, vec![1, 2, 3]);
        chunk.transform(18, 10);
        assert_chunk!(chunk, 18, 10, empty);
    }

    #[test]
    pub fn test_set_accessible_1() {
        // start and end are to the left of memory region
        let mut chunk = chunk(10, 10, 15, vec![1, 2, 3]);
        chunk.set_accessible(5, &[4, 5, 6]);
        assert_chunk!(chunk, 10, 10, empty);
    }

    #[test]
    pub fn test_set_accessible_2() {
        // start is to the left, end is within bounds
        let mut chunk = chunk(10, 10, 15, vec![1, 2, 3]);
        chunk.set_accessible(5, &[4, 5, 6, 7, 8, 9, 10]);
        assert_chunk!(chunk, 10, 10, 10, [9, 10]);
    }

    #[test]
    pub fn test_set_accessible_3() {
        // start is to the left, end is to the right
        let mut chunk = chunk(10, 5, 11, vec![1, 2, 3]);
        chunk.set_accessible(8, &[4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]);
        assert_chunk!(chunk, 10, 5, 10, [6, 7, 8, 9, 10]);
    }

    #[test]
    pub fn test_set_accessible_4() {
        // start is within bounds, end is within bounds
        let mut chunk = chunk(10, 10, 15, vec![1, 2, 3]);
        chunk.set_accessible(11, &[4, 5, 6]);
        assert_chunk!(chunk, 10, 10, 11, [4, 5, 6]);
    }

    #[test]
    pub fn test_set_accessible_5() {
        // start is within bounds, end is to the right
        let mut chunk = chunk(10, 5, 11, vec![1, 2, 3]);
        chunk.set_accessible(12, &[4, 5, 6, 7, 8, 9, 10]);
        assert_chunk!(chunk, 10, 5, 12, [4, 5, 6]);
    }

    #[test]
    pub fn test_set_accessible_6() {
        // start and end are to the right
        let mut chunk = chunk(10, 5, 11, vec![1, 2, 3]);
        chunk.set_accessible(16, &[4, 5, 6, 7, 8, 9, 10]);
        assert_chunk!(chunk, 10, 5, empty);
    }

    #[test]
    pub fn test_extend_accessible_1() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(5, &[4, 5, 6]);
        assert_chunk!(chunk, 10, 10, 13, [1, 2, 3]);
    }

    #[test]
    pub fn test_extend_accessible_2() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(9, &[4, 5, 6]);
        assert_chunk!(chunk, 10, 10, 13, [1, 2, 3]);
    }

    #[test]
    pub fn test_extend_accessible_3() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(9, &[4, 5, 6, 7, 8, 9]);
        assert_chunk!(chunk, 10, 10, 10, [5, 6, 7, 1, 2, 3]);
    }

    #[test]
    pub fn test_extend_accessible_4() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(9, &[4, 5, 6, 7, 8, 9, 10, 11, 12]);
        assert_chunk!(chunk, 10, 10, 10, [5, 6, 7, 1, 2, 3, 11, 12]);
    }

    #[test]
    pub fn test_extend_accessible_5() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(9,
                                &[4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                                  21]);
        assert_chunk!(chunk, 10, 10, 10, [5, 6, 7, 1, 2, 3, 11, 12, 13, 14]);
    }

    #[test]
    pub fn test_extend_accessible_6() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(10, &[4, 5]);
        assert_chunk!(chunk, 10, 10, 13, [1, 2, 3]);
    }

    #[test]
    pub fn test_extend_accessible_7() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(10, &[4, 5, 6]);
        assert_chunk!(chunk, 10, 10, 10, [4, 5, 6, 1, 2, 3]);
    }

    #[test]
    pub fn test_extend_accessible_8() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(11, &[4, 5, 6, 7, 8, 9, 10, 11]);
        assert_chunk!(chunk, 10, 10, 11, [4, 5, 1, 2, 3, 9, 10, 11]);
    }

    #[test]
    pub fn test_extend_accessible_9() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(11, &[4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17]);
        assert_chunk!(chunk, 10, 10, 11, [4, 5, 1, 2, 3, 9, 10, 11, 12]);
    }

    #[test]
    pub fn test_extend_accessible_10() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(14, &[4, 5]);
        assert_chunk!(chunk, 10, 10, 13, [1, 2, 3]);
    }

    #[test]
    pub fn test_extend_accessible_11() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(14, &[4, 5, 6, 7, 8, 9]);
        assert_chunk!(chunk, 10, 10, 13, [1, 2, 3, 6, 7, 8, 9]);
    }

    #[test]
    pub fn test_extend_accessible_12() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(14, &[4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14]);
        assert_chunk!(chunk, 10, 10, 13, [1, 2, 3, 6, 7, 8, 9]);
    }

    #[test]
    pub fn test_extend_accessible_13() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(16, &[4, 5]);
        assert_chunk!(chunk, 10, 10, 13, [1, 2, 3, 4, 5]);
    }

    #[test]
    pub fn test_extend_accessible_14() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        chunk.extend_accessible(17, &[4, 5]);
        assert_chunk!(chunk, 10, 10, 13, [1, 2, 3]);
    }

    #[test]
    pub fn test_slice_mut_1() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        let slice = chunk.slice_mut(5, 3);
        assert_chunk!(slice, 5, 3, empty);
    }

    #[test]
    pub fn test_slice_mut_2() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        let slice = chunk.slice_mut(5, 10);
        assert_chunk!(slice, 5, 10, 13, [1, 2]);
    }

    #[test]
    pub fn test_slice_mut_3() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        let slice = chunk.slice_mut(5, 15);
        assert_chunk!(slice, 5, 15, 13, [1, 2, 3]);
    }

    #[test]
    pub fn test_slice_mut_4() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        let slice = chunk.slice_mut(14, 1);
        assert_chunk!(slice, 14, 1, 14, [2]);
    }

    #[test]
    pub fn test_slice_mut_5() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        let slice = chunk.slice_mut(14, 7);
        assert_chunk!(slice, 14, 7, 14, [2, 3]);
    }

    #[test]
    pub fn test_slice_mut_6() {
        let mut chunk = chunk(10, 10, 13, vec![1, 2, 3]);
        let slice = chunk.slice_mut(16, 3);
        assert_chunk!(slice, 16, 3, empty);
    }
}
