#pragma once

#include "PDMessages_generated.h"
#include "api/include/pd_message_readwrite.h"

template<typename T>
inline void PDMessage_end_msg(PDWriteMessage* writer, T& msg, flatbuffers::FlatBufferBuilder& builder) {
    builder.Finish(CreateMessageDirect(builder,
        MessageTypeTraits<typename T::Table>::enum_value, msg.Finish().Union()));

    PDWriteMessage_write(writer, builder.GetBufferPointer(), builder.GetSize());
}

