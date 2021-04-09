#pragma once

#include <stdint.h>

enum AmigaMessages {
    AmigaMessages_RequestFramebuffer = 1,
    AmigaMessages_ReplyFrameBuffer,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct AmigaFrameBufferData {
    int width;
    int height;
    uint32_t data[1];
};
