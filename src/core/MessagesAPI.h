#pragma once

#include "api/include/pd_message_readwrite.h"

namespace prodbg {

class MessageAPI {
public:
    void allocate_buffers(int size);

    void init_reader(PDReadMessage* reader);
    void init_writer(PDWriteMessage* write);

    void reset_reader(PDReadMessage* reader);

    void swap_buffers();
private:

};

}
