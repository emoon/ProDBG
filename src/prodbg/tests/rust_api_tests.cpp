#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "core/alloc.h"
#include "core/commands.h"
#include "core/core.h"
#include "core/file.h"
#include "core/file_monitor.h"
#include "core/log.h"
#include "core/capstone_service.h"
#include "core/plugin_handler.h"
#include "core/settings.h"
#include "api/src/remote/pd_readwrite_private.h"
#include "session/session.h"
#include "session/session_private.h"

static Session* s_session;
static uint8_t s_data[] = { 1, 2, 3, 80, 50, 60 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_rust_init(void**) {
	int count = 0;
    //assert_true(PluginHandler_addPlugin(OBJECT_DIR, "c64_vice_plugin"));
    assert_true(PluginHandler_addPlugin("src/prodbg/tests/rust_api_test/target/debug", "rust_api_test"));
    assert_non_null(PluginHandler_getBackendPlugins(&count)[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_rust_init_instance(void**) {
	int count = 0;

    PluginData* pluginData = PluginHandler_getBackendPlugins(&count)[0];
    s_session = Session_createLocal((PDBackendPlugin*)pluginData->plugin, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_rust_write_all(void**) {
    int8_t s8;
    uint8_t u8;
    int16_t s16;
    uint16_t u16;
    int32_t s32;
    uint32_t u32;
    int64_t s64;
    uint64_t u64;
    float fvalue = 0.0f;
    double dvalue = 0.0;
    const char* string;
    uint8_t* data;
    uint64_t size;

	Session_update(s_session);

	// Verify that correct data has been written here

    PDReader* reader = s_session->reader;
    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(s_session->currentWriter), PDBinaryWriter_getSize(s_session->currentWriter));

    assert_true(PDRead_get_event(reader) == 3);

    // s8

    assert_true((PDRead_find_s8(reader, &s8, "my_s8", 0) & PDReadStatus_TypeMask) == PDReadType_S8);
    assert_true(s8 == -2);

    // u8

    assert_true((PDRead_find_u8(reader, &u8, "my_u8", 0) & PDReadStatus_TypeMask) == PDReadType_U8);
    assert_true(u8 == 3);

    // s16

    assert_true((PDRead_find_s16(reader, &s16, "my_s16", 0) & PDReadStatus_TypeMask) == PDReadType_S16);
    assert_true(s16 == -2000);

    // u16

    assert_true((PDRead_find_u16(reader, &u16, "my_u16", 0) & PDReadStatus_TypeMask) == PDReadType_U16);
    assert_true(u16 == 56);

    // s32

    assert_true((PDRead_find_s32(reader, &s32, "my_s32", 0) & PDReadStatus_TypeMask) == PDReadType_S32);
    assert_true(s32 == -300000);

    // u32

    assert_true((PDRead_find_u32(reader, &u32, "my_u32", 0) & PDReadStatus_TypeMask) == PDReadType_U32);
    assert_true(u32 == 4000000);

    // s64

    assert_true((PDRead_find_s64(reader, &s64, "my_s64", 0) & PDReadStatus_TypeMask) == PDReadType_S64);
    assert_true(s64 == -1400000L);

    // u64

    assert_true((PDRead_find_u64(reader, &u64, "my_u64", 0) & PDReadStatus_TypeMask) == PDReadType_U64);
    assert_true(u64 == 6000000L);

    // float

    assert_true((PDRead_find_float(reader, &fvalue, "my_float", 0) & PDReadStatus_TypeMask) == PDReadType_Float);
    assert_true(fvalue == 14.0f);

    // float 2

    assert_true((PDRead_find_float(reader, &fvalue, "my_float2", 0) & PDReadStatus_TypeMask) == PDReadType_Float);
    assert_true(((fvalue > (-24.0f - 0.001f)) && fvalue < ((-24.0f + 0.0001f))));

    // double

    assert_true((PDRead_find_double(reader, &dvalue, "my_double", 0) & PDReadStatus_TypeMask) == PDReadType_Double);
    assert_true(dvalue == 23.0);

    // double 2

    assert_true((PDRead_find_double(reader, &dvalue, "my_double2", 0) & PDReadStatus_TypeMask) == PDReadType_Double);
    assert_true(dvalue == 63.0);

    // string

    assert_true((PDRead_find_string(reader, &string, "my_string", 0) & PDReadStatus_TypeMask) == PDReadType_String);
    assert_string_equal(string, "foobar1337");

    // data

    assert_true((PDRead_find_data(reader, (void**)&data, &size, "my_data", 0) & PDReadStatus_TypeMask) == PDReadType_Data);
    assert_true(size == sizeof(s_data));
    assert_true(data[0] == s_data[0]);
    assert_true(data[1] == s_data[1]);
    assert_true(data[2] == s_data[2]);
    assert_true(data[5] == s_data[5]);

    assert_true(PDRead_get_event(reader) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_rust_read(void**) {
    PDReader* reader = s_session->reader;
    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(s_session->currentWriter), PDBinaryWriter_getSize(s_session->currentWriter));

	Session_update(s_session);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_rust_capstone(void**) {

	// This depends on the data written in the previous function. Here we validate that we can read back the same
	// data that was written before (and verified by the C code)

	Session_update(s_session);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    log_set_level(LOG_DEBUG);
    Core_init();
    CapstoneService_init();

    const UnitTest tests[] = {
        unit_test(test_rust_init),
        unit_test(test_rust_init_instance),
        unit_test(test_rust_write_all),
        unit_test(test_rust_read),
        unit_test(test_rust_capstone),
    };

    return run_tests(tests);
}




