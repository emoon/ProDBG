#pragma once

#include "PDMessages_generated.h"

template<typename T>
inline void PDMessage_end_msg(PDWriter* writer, T& msg, flatbuffers::FlatBufferBuilder& builder) {
    builder.Finish(CreateMessageDirect(builder,
        MessageTypeTraits<typename T::Table>::enum_value, msg.Finish().Union()));

    printf("reply enum %d\n", MessageTypeTraits<T>::enum_value);

    // TODO: Streamline this
    PDWrite_event_begin(writer, PDEventType_Dummy);
    PDWrite_data(writer, "data", builder.GetBufferPointer(), builder.GetSize());
    PDWrite_event_end(writer);
}

