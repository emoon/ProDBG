#pragma once

#include <stdint.h>
#include <stdbool.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDReadMessage {
    /// private internal data
    void* priv;
    /// Get the next message in the queue, when there is no more messages NULL will be returned
    const uint8_t* (*next_message)(struct PDReadMessage* reader, uint64_t* size);
} PDReadMessage;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDWriteMessage {
    /// private internal data
    void* priv;
    /// Write message to the queue. Returns false if write failed
    bool (*write_message)(struct PDWriteMessage* writer, uint8_t* data, uint64_t size);
} PDWriteMessage;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDWriteMessage_write(msg, data, size) msg->write_message(msg, data, size)
#define PDReadMessage_next_message(msg, size) msg->next_message(msg, size)

