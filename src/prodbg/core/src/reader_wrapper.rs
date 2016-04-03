use std::os::raw::c_void;

use prodbg_api::read_write::{CPDReaderAPI, CPDWriterAPI, Reader, Writer};

pub struct ReaderWrapper;
pub struct WriterWrapper;

impl ReaderWrapper {
    pub fn create_reader() -> Reader {
        unsafe {
            Reader::new(pd_binary_reader_create(), 0)
        }
    }

    pub fn init_from_writer(reader: &mut Reader, writer: &Writer) {
        unsafe {
            let data = pd_binary_writer_get_data(writer.api);
            let size = pd_binary_writer_get_size(writer.api);

            pd_binary_reader_init_stream(reader.api, data, size);
        }
    }
}

impl WriterWrapper {
    pub fn create_writer() -> Writer {
        unsafe {
            Writer {
                api: pd_binary_writer_create(),
            }
        }
    }
}

extern "C" {
    fn pd_binary_writer_create() -> *mut CPDWriterAPI;
    fn pd_binary_reader_create() -> *mut CPDReaderAPI;
    fn pd_binary_writer_get_data(api: *mut CPDWriterAPI) -> *mut c_void;
    fn pd_binary_writer_get_size(api: *mut CPDWriterAPI) -> u32;
    fn pd_binary_reader_init_stream(api: *mut CPDReaderAPI, data: *mut c_void, size: u32);
}
