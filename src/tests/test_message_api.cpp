#include "test_harness.h"
#include "core/MessagesAPI.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(MessagesAPI, create) {
    uint64_t size = 1024;

    prodbg::MessagesAPI* api = new prodbg::MessagesAPI (size);

    ASSERT_NE(api->m_memory_start, nullptr);
    ASSERT_NE(api->m_data_read, nullptr);
    ASSERT_NE(api->m_data_write, nullptr);

    ASSERT_NE(api->m_data_read, api->m_data_write);
    ASSERT_EQ(api->m_total_size, size / 2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(MessagesAPI, write) {
    uint64_t size = 1024;
    uint8_t data[] = { 1, 2, 3, 4 };

    prodbg::MessagesAPI* api = new prodbg::MessagesAPI (size);
    PDWriteMessage* writer = api->get_writer();

    // Expected to pass
    PDWriteMessage_write(writer, data, sizeof(data));

    ASSERT_EQ(api->m_write_offset, sizeof(uint64_t) + sizeof(data));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(MessagesAPI, read_write) {
    uint64_t size = 1024;
    uint8_t data[] = { 1, 2, 3, 4 };
    uint64_t read_size = 0;

    prodbg::MessagesAPI* api = new prodbg::MessagesAPI(size);
    PDWriteMessage* writer = api->get_writer();
    PDReadMessage* reader = api->get_reader();

    // Expected to pass
    PDWriteMessage_write(writer, data, sizeof(data));
    ASSERT_EQ(api->m_write_offset, sizeof(uint64_t) + sizeof(data));

    api->swap_buffers();

    const uint8_t* data_read = PDReadMessage_next_message(reader, &read_size);
    ASSERT_EQ(read_size, sizeof(data));

    for (uint64_t i = 0; i < read_size; ++i) {
        ASSERT_EQ(data[i], data_read[i]);
    }

    data_read = PDReadMessage_next_message(reader, &read_size);
    ASSERT_EQ(read_size, uint64_t(0));
    ASSERT_EQ(data_read, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(MessagesAPI, too_small_alloc_buffer) {
    uint8_t data[] = { 1, 2, 3, 4 };
    prodbg::MessagesAPI* api = new prodbg::MessagesAPI(16);
    auto writer = api->get_writer();
    ASSERT_EQ(PDWriteMessage_write(writer, data, sizeof(data)), false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(MessagesAPI, small_1_buffer_allowed) {
    uint8_t data[] = { 1, 2, 3, 4 };
    prodbg::MessagesAPI* api = new prodbg::MessagesAPI(2 * (8 + 8 + 4));
    auto writer = api->get_writer();
    ASSERT_EQ(PDWriteMessage_write(writer, data, sizeof(data)), true);
    ASSERT_EQ(PDWriteMessage_write(writer, data, sizeof(data)), false);
}



