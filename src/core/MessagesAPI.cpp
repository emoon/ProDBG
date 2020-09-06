#include "MessagesAPI.h"
#include "Logging.h"
#include <stdlib.h>
#include <string.h>
#include "api/include/pd_message_readwrite.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

typedef uint64_t size_type;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get the next message in the queue. The data format is <uint64_t>,data,... and ends with size being zero

static const uint8_t* next_message(struct PDReadMessage* reader, uint64_t* size) {
    MessagesAPI* buf = (MessagesAPI*)reader->priv;

    // Make sure we can read the next value
    if ((buf->m_read_offset + sizeof(size_type)) >= buf->m_total_size) {
        log_error("ReadMessage: Out of space in buffer (max %ld current %ld)\n", buf->m_total_size, buf->m_read_offset);
        *size = 0;
        return nullptr;
    }

    size_type curr_size = *(size_type*)&buf->m_data_read[buf->m_read_offset];

    // This is a valid state when there are no messages to read
    if (curr_size == 0) {
        *size = 0;
        return nullptr;
    }

    buf->m_read_offset += sizeof(size_type);

    // Make sure we have enough space left in the buffer to return this size
    if ((curr_size + buf->m_read_offset) >= buf->m_total_size) {
        log_error("ReadMessage: Out of space in for message (Message size is %ld but no space left max %ld - %ld)\n",
                   curr_size, buf->m_total_size, buf->m_read_offset);
        *size = 0;
        return nullptr;
    }

    uint8_t* return_data = (uint8_t*)&buf->m_data_read[buf->m_read_offset];

    buf->m_read_offset += curr_size;
    *size = curr_size;

    return return_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write data to the writer The data format is <uint64_t>,data

static bool write_message(struct PDWriteMessage* writer, uint8_t* data, uint64_t size) {
    MessagesAPI* buf = (MessagesAPI*)writer->priv;

    // in order to write a message we need the data + sizeof(size_type) * 2 to store the data. We always also write
    // zero at the end of the message (which is reused if more messages are coming) the reason for that is that
    // it marks the end of the message stream

    if (((buf->m_write_offset + size) + sizeof(size_type) * 2) > buf->m_total_size) {
        log_error("WriteMessage: Out of space in buffer (max %ld current %ld)\n", buf->m_total_size, buf->m_write_offset);
        return false;
    }

    uint8_t* current_data = &buf->m_data_write[buf->m_write_offset];
    *(size_type*)current_data = size; // current data size

    current_data += sizeof(size_type);
    memcpy(current_data, data, size);
    current_data += size;

    // end marker (will be reused for next message if there is one)
    *(size_type*)current_data = 0;

    buf->m_write_offset += sizeof(size_type) + size;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MessagesAPI::MessagesAPI(int size) {
    m_data_read = m_data_write = m_memory_start = (uint8_t*)malloc(size);

    if (!m_memory_start) {
        fatal("MessagesAPI: Failed to alloc memory");
    }

    m_total_size = size / 2;
    m_data_write += m_total_size;

    m_reader.priv = (void*)this;
    m_reader.next_message = next_message;

    m_writer.priv = (void*)this;
    m_writer.write_message = write_message;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MessagesAPI::~MessagesAPI() {
    free(m_memory_start);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MessagesAPI::swap_buffers() {
    uint8_t* temp = m_data_read;
    m_data_read = m_data_write;
    m_data_write = temp;
    m_read_offset = 0;
    m_write_offset = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

