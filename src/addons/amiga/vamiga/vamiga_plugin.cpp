#include <stdint.h>
#include "pd_backend.h"
#include "pd_backend_messages.h"
#include "pd_host.h"
#include "pd_message_readwrite.h"
#include "Amiga.h"
#include "../amiga_structs.h"
#include "Constants.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct VAmigaPlugin {
    Amiga* amiga;
} VAmigaPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void log_callback(const void* t, long t0, long t1) {

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* create_instance(ServiceFunc* serviceFunc) {
    VAmigaPlugin* t = new VAmigaPlugin;
    t->amiga = new Amiga;
    t->amiga->queue.setListener(t, log_callback);
    t->amiga->mem.loadRomFromFile("/home/emoon/kick13.rom");
    t->amiga->mem.allocChip(512 * 1024);
    t->amiga->mem.allocSlow(512 * 1024);
    t->amiga->powerOn();
    t->amiga->run();

    return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroy_instance(void* user_data) {
    VAmigaPlugin* plugin = (VAmigaPlugin*)user_data;
    delete plugin->amiga;
    delete plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
static void target_reply(VAmigaPlugin* plugin, const FileTargetRequest* request, PDWriteMessage* writer) {
    /*
    flatbuffers::FlatBufferBuilder builder(1024);

    const char* filename = request->path()->c_str();

    FILE* f = fopen(filename, "rb");
    char error_msg[4096];
    bool status = true;

    if (!f) {
        sprintf(error_msg, "VAmigaPlugin: Unable to open file %s", filename);
        status = false;
    } else {
        fseek(f, 0,  SEEK_END);
        auto size = ftell(f);
        fseek(f, 0, SEEK_SET);
        plugin->buffer = (uint8_t*)malloc(size);

        if (!plugin->buffer) {
            sprintf(error_msg, "VAmigaPlugin: To allocate space (%ld) bytes for file %s", size, filename);
            status = false;
        } else {
            fread(plugin->buffer, 1, size, f);
            plugin->filename = strdup(filename);
            plugin->buffer_size = size;
            fclose(f);
        }
    }

    auto error_str = builder.CreateString(error_msg);
    TargetReplyBuilder reply(builder);
    reply.add_status(status);
    reply.add_error_message(error_str);
    PDMessage_end_msg(writer, reply, builder);
    */
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void memory_reply(VAmigaPlugin* plugin, const MemoryRequest* request, PDWriteMessage* writer) {
    /*
    flatbuffers::FlatBufferBuilder builder(1024);
    std::vector<unsigned char> return_data;

    uint64_t start_address = request->start_address();
    uint64_t size = request->size();
    MemoryReplyStatus memory_status = MemoryReplyStatus_Ok;

    if (plugin->buffer_size > (start_address + size)) {
        return_data.resize(size);
        const uint8_t* data = plugin->buffer + start_address;
        //uint8_t* output_data = return_data.data();

        for (uint64_t i = 0; i < size; ++i) {
            return_data[i] = data[i];
        }
    } else {
        if (start_address >= plugin->buffer_size) {
            memory_status = MemoryReplyStatus_InvalidRange;
        } else {
            // partial buffer
            return_data.resize(size);
            size = (start_address + size) - plugin->buffer_size;

            const uint8_t* data = plugin->buffer + start_address;

            for (uint64_t i = 0; i < size; ++i) {
                return_data[i] = data[i];
            }

            memory_status = MemoryReplyStatus_Partial;
        }
    }

    auto t = builder.CreateVector<uint8_t>(return_data.data(), return_data.size());

    MemoryReplyBuilder reply(builder);

    reply.add_start_address(start_address);
    reply.add_data(t);
    reply.add_status(memory_status);
    PDMessage_end_msg(writer, reply, builder);
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void send_framebuffer(VAmigaPlugin* plugin, PDWriteMessage* writer) {

    u32* source = plugin->amiga->denise.pixelEngine.getStableBuffer().data;

    int xStart = 4 * HBLANK_MAX + 1;
    int xEnd = HPIXELS + 4 * HBLANK_MIN;
    int yStart = VBLANK_CNT;
    int yEnd = VPIXELS - 2;

    int width = (xEnd - xStart);
    int height = (yEnd - yStart);

    source += xStart + yStart * HPIXELS;

    printf("width %d height %d\n", width, height);

    int buffer_size = sizeof(AmigaFrameBufferData) + (width * height * sizeof(u32));

    AmigaFrameBufferData* output = (AmigaFrameBufferData*)malloc(buffer_size);

    output->width = width;
    output->height = height;

    for (isize y = 0; y < height; y++) {
        for (isize x = 0; x < width; x++) {
            output->data[(y * width) + x] = source[x];
        }
        source += HPIXELS;
    }

    flatbuffers::FlatBufferBuilder builder(1024);
    auto t = builder.CreateVector<uint8_t>((uint8_t*)output, buffer_size);

    CustomRequestBuilder reply(builder);
    reply.add_message_id(AmigaMessages_ReplyFrameBuffer);
    reply.add_data(t);
    PDMessage_end_msg(writer, reply, builder);

    free(output);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void custom_request(VAmigaPlugin* plugin, const CustomRequest* request, PDWriteMessage* writer) {
    int id = request->message_id();

    switch (request->message_id()) {
        case AmigaMessages_RequestFramebuffer: send_framebuffer(plugin, writer); break;
        default: printf("unknown custom request_id %d\n", id); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void process_events(VAmigaPlugin* plugin, PDReadMessage* reader, PDWriteMessage* writer) {
    const uint8_t* data;
    uint64_t size;

    while ((data = PDReadMessage_next_message(reader, &size))) {
        const PDMessage* msg = GetPDMessage(data);

        switch (msg->message_type()) {
            case PDMessageType_custom_request: {
                custom_request(plugin, msg->message_as_custom_request(), writer);
                break;
            }

            case PDMessageType_memory_request: {
                memory_reply(plugin, msg->message_as_memory_request(), writer);
                break;
            }

            default: break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data, PDAction action, PDReadMessage* reader, PDWriteMessage* writer) {
    VAmigaPlugin* plugin = (VAmigaPlugin*)user_data;

    process_events(plugin, reader, writer);

    /*
    u32* source = plugin->amiga->denise.pixelEngine.getStableBuffer().data;

    int xStart = 4 * HBLANK_MAX + 1;
    int xEnd = HPIXELS + 4 * HBLANK_MIN;
    int yStart = VBLANK_CNT;
    int yEnd = VPIXELS - 2;

    int width = (xEnd - xStart);
    int height = (yEnd - yStart);

    source += xStart + yStart * HPIXELS;

    printf("width %d height %d\n", width, height);


    for (isize y = 0; y < height; y++) {
        for (isize x = 0; x < width; x++) {
            target[x] = source[x * dx];
        }
        source += dy * HPIXELS;
        target += width;
    }
    */


    /*
    if (plugin->buffer) {
       return PDDebugState_Trace;
    } else {
       return PDDebugState_NoTarget;
    }
    */
    return PDDebugState_Running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin = {
    "Amiga", create_instance, destroy_instance, update,
    0,  // save_state
    0,  // load_state
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" PD_EXPORT void pd_register_backend(PDRegisterBackendPlugin* register_plugin, void* private_data) {
    register_plugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}


