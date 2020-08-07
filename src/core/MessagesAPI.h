#pragma once

#include "api/include/pd_message_readwrite.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Notice that this code isn't thread-safe

class MessagesAPI {
public:
    // Allocate memory for the read/write buffers the size will be split in half for reader/writer
    MessagesAPI(int size);
    ~MessagesAPI();

    inline PDReadMessage* get_reader() { return &m_reader; }
    inline PDWriteMessage* get_writer() { return &m_writer; }

    // This is used to allow you to restart reading the messages again. Useful if there are several loops reading
    inline void reset_reader() { m_read_offset = 0; }
    void swap_buffers();

    PDReadMessage m_reader;
    PDWriteMessage m_writer;

    uint8_t* m_memory_start = nullptr;

    uint8_t* m_data_read = nullptr;
    uint8_t* m_data_write = nullptr;

    uint64_t m_read_offset = 0;
    uint64_t m_write_offset = 0;

    uint64_t m_total_size = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
