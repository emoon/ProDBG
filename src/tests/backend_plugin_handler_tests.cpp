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
            found_registers = true;
        }
    }

    ASSERT_EQ(found_registers, true);

    close_dummy_plugin(p);
}

