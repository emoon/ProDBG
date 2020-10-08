#include "test_harness.h"
#include "core/BackendPluginHandler.h"
#include "../api/include/pd_backend.h"
#include "../api/include/pd_backend_messages.h"
#include "core/MessagesAPI.h"

struct Plugin {
    void* user_data;
    PDBackendPlugin* plugin;
    prodbg::MessagesAPI* msg_api;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Plugin setup_dummy_plugin() {
    void* user_data = nullptr;
    PDBackendPlugin* plugin = nullptr;

    prodbg::MessagesAPI* api = new prodbg::MessagesAPI(512 * 1024);
    /*
    ASSERT_NE(plugin = prodbg::BackendPluginHandler::find_plugin("Dummy Backend"), nullptr);
    ASSERT_NE(user_data = plugin->create_instance(nullptr), nullptr);
    */
    prodbg::BackendPluginHandler::add_plugin("dummy_backend");
    plugin = prodbg::BackendPluginHandler::find_plugin("Dummy Backend");
    user_data = plugin->create_instance(nullptr);

    return Plugin { user_data, plugin, api };
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void request_registers(Plugin& p, CPURegisterType reg_type) {
    flatbuffers::FlatBufferBuilder builder(1024);

    // Setup the request
    CPURegistersRequestBuilder request(builder);
    request.add_registers_type(CPURegisterType_Integer);
    PDMessage_end_msg(p.msg_api->get_writer(), request, builder);

    p.msg_api->swap_buffers();

    p.plugin->update(p.user_data, PDAction_None, p.msg_api->get_reader(), p.msg_api->get_writer());

    p.msg_api->swap_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void close_dummy_plugin(Plugin p) {
    delete p.msg_api;
    p.plugin->destroy_instance(p.user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BackendPluginHandler, load_dummy_backend) {
    ASSERT_EQ(prodbg::BackendPluginHandler::add_plugin("dummy_backend"), true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BackendPluginHandler, find_dummy_plugin) {
    ASSERT_NE(prodbg::BackendPluginHandler::find_plugin("Dummy Backend"), nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BackendPluginHandler, load_backend_fail) {
    ASSERT_EQ(prodbg::BackendPluginHandler::add_plugin("no_such_plugin"), false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BackendPluginHandler, find_fail) {
    ASSERT_EQ(prodbg::BackendPluginHandler::find_plugin("No Such plugin"), nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BackendPluginHandler, read_integer_registers) {
    const uint8_t* data;
    uint64_t size;

    auto p = setup_dummy_plugin();

    request_registers(p, CPURegisterType_Integer);

    auto reader = p.msg_api->get_reader();
    bool found_registers = false;

    while ((data = PDReadMessage_next_message(reader, &size))) {
        const Message* msg = GetMessage(data);

        if (msg->message_type() == MessageType_cpu_registers_reply) {
            auto registers = msg->message_as_cpu_registers_reply();

            for (auto t : *registers->entries()) {
                if (!strcmp(t->name()->c_str(), "eax")) {
                    auto data = t->data()->data();
                    ASSERT_EQ(t->data()->Length(), (unsigned int)4);
                    ASSERT_EQ(data[0], (uint8_t)0xca);
                    ASSERT_EQ(data[1], (uint8_t)0xcb);
                    ASSERT_EQ(data[2], (uint8_t)0xcc);
                    ASSERT_EQ(data[3], (uint8_t)0xcd);
                    ASSERT_EQ(t->read_only(), false);
                } else if (!strcmp(t->name()->c_str(), "elf")) {
                    auto data = t->data()->data();
                    ASSERT_EQ(t->data()->Length(), (unsigned int)4);
                    ASSERT_EQ(data[0], (uint8_t)0x00);
                    ASSERT_EQ(data[1], (uint8_t)0x00);
                    ASSERT_EQ(data[2], (uint8_t)0x02);
                    ASSERT_EQ(data[3], (uint8_t)0x00);
                    ASSERT_EQ(t->read_only(), true);
                }
            }

            found_registers = true;
        }
    }

    ASSERT_EQ(found_registers, true);

    close_dummy_plugin(p);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void request_memory(Plugin& p, uint64_t start_address, int64_t size) {
    flatbuffers::FlatBufferBuilder builder(1024);

    // Setup the request
    MemoryRequestBuilder request(builder);
    request.add_start_address(start_address);
    request.add_size(size);
    PDMessage_end_msg(p.msg_api->get_writer(), request, builder);

    p.msg_api->swap_buffers();

    p.plugin->update(p.user_data, PDAction_None, p.msg_api->get_reader(), p.msg_api->get_writer());

    p.msg_api->swap_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BackendPluginHandler, read_memory_valid_range) {
    const uint8_t* data;
    uint64_t size;

    auto p = setup_dummy_plugin();

    // memory for dummy starts at 0x10000
    request_memory(p, 0x1000, 8);

    auto reader = p.msg_api->get_reader();
    bool found_memory = false;

    while ((data = PDReadMessage_next_message(reader, &size))) {
        const Message* msg = GetMessage(data);

        if (msg->message_type() == MessageType_memory_reply) {
            auto memory = msg->message_as_memory_reply();

            ASSERT_EQ((int)memory->data()->size(), 8);
            ASSERT_EQ(memory->status(), MemoryReplyStatus_Ok);
            auto data = memory->data()->data();

            for (int i = 0; i < 8; ++i) {
                ASSERT_EQ(data[i], i);
            }

            found_memory = true;
        }
    }

    ASSERT_EQ(found_memory, true);
}

