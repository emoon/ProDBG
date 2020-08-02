#pragma once

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDReadMessage {
    /// private internal data
    void* priv;
    /// Get the next message in the queue, when there is no more messages NULL will be returned
    const void* (*next_message)(struct PDReadMessage* reader);
} PDReadMessage;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDWriteMessage {
    /// private internal data
    void* priv;
    /// Write message to the queue. Returns false if write failed
    bool (*write_message)(struct PDWriteMessage* writer, uint8_t* data, uint64_t size);
} PDWriteMessage;
