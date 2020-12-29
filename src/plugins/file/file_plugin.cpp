#include <stdint.h>
#include "pd_backend.h"
#include "pd_backend_messages.h"
#include "pd_host.h"
#include "pd_message_readwrite.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct FilePlugin {
    char* filename;
    uint8_t* buffer;
    size_t buffer_size;
} FilePlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* create_instance(ServiceFunc* serviceFunc) {
    FilePlugin* t = (FilePlugin*)calloc(1, sizeof(FilePlugin));
    return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroy_instance(void* user_data) {
    FilePlugin* plugin = (FilePlugin*)user_data;
    free(plugin->filename);
    free(plugin->buffer);
    free(plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void target_reply(FilePlugin* plugin, const FileTargetRequest* request, PDWriteMessage* writer) {
    flatbuffers::FlatBufferBuilder builder(1024);

    const char* filename = request->path()->c_str();

    FILE* f = fopen(filename, "rb");
    char error_msg[4096];
    bool status = true;

    if (!f) {
        sprintf(error_msg, "FilePlugin: Unable to open file %s", filename);
        status = false;
    } else {
        fseek(f, 0,  SEEK_END);
        auto size = ftell(f);
        fseek(f, 0, SEEK_SET);
        plugin->buffer = (uint8_t*)malloc(size);

        if (!plugin->buffer) {
            sprintf(error_msg, "FilePlugin: To allocate space (%ld) bytes for file %s", size, filename);
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void memory_reply(FilePlugin* plugin, const MemoryRequest* request, PDWriteMessage* writer) {
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void process_events(FilePlugin* plugin, PDReadMessage* reader, PDWriteMessage* writer) {
    const uint8_t* data;
    uint64_t size;

    while ((data = PDReadMessage_next_message(reader, &size))) {
        const Message* msg = GetMessage(data);

        switch (msg->message_type()) {
            case MessageType_file_target_request: {
                target_reply(plugin, msg->message_as_file_target_request(), writer);
                break;
            }

            case MessageType_memory_request: {
                memory_reply(plugin, msg->message_as_memory_request(), writer);
                break;
            }

            default: break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data, PDAction action, PDReadMessage* reader, PDWriteMessage* writer) {
    FilePlugin* plugin = (FilePlugin*)user_data;

    process_events(plugin, reader, writer);

    if (plugin->buffer) {
       return PDDebugState_Trace;
    } else {
       return PDDebugState_NoTarget;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin = {
    "File", create_instance, destroy_instance, update,
    0,  // save_state
    0,  // load_state
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" PD_EXPORT void pd_init_plugin(RegisterPlugin* register_plugin, void* private_data) {
    register_plugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

