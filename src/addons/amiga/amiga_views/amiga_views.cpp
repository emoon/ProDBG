#include "pd_ui_register_plugin.h"
#include "amiga_framebuffer.h"

extern "C" void pd_register_view(PDRegisterViewPlugin* reg) {
    reg->register_view(new AmigaFrameBuffer);
}

