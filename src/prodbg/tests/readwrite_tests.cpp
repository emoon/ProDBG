#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <pd_readwrite.h>
#include <pd_backend.h> // For eventTypes
#include "api/src/remote/pd_readwrite_private.h"
#include "core/log.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDReader readerData;
static PDWriter writerData;
static PDReader* reader = 0;
static PDWriter* writer = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t s_data[] = { 1, 2, 3, 80, 50, 60 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testWriteReadEvent(void**) {
    unsigned char* data;
    unsigned int size;

    PDBinaryWriter_reset(writer);

    assert_true(PDWrite_event_begin(writer, 10) == PDWriteStatus_ok);
    assert_true(PDWrite_event_end(writer) == PDWriteStatus_ok);

    PDBinaryWriter_finalize(writer);

    data = PDBinaryWriter_getData(writer);
    size = PDBinaryWriter_getSize(writer);

    assert_true(data != 0);
    assert_true(size != 0);

    PDBinaryReader_initStream(reader, data, size);
    assert_true(PDRead_get_event(reader) == 10);

    assert_true(PDRead_get_event(reader) == 0);    // should be 0 as no more events
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testDataReadWrite(void**) {
    unsigned char* data;
    unsigned int size;
    uint64_t dataSize;

    PDBinaryWriter_reset(writer);

    assert_true(PDWrite_event_begin(writer, 10) == PDWriteStatus_ok);
    assert_true(PDWrite_data(writer, "my_data", s_data, sizeof(s_data)) == PDWriteStatus_ok);
    assert_true(PDWrite_event_end(writer) == PDWriteStatus_ok);

    PDBinaryWriter_finalize(writer);

    data = PDBinaryWriter_getData(writer);
    size = PDBinaryWriter_getSize(writer);

    assert_true(data != 0);
    assert_true(size != 0);

    PDBinaryReader_initStream(reader, data, size);
    assert_true(PDRead_get_event(reader) == 10);

    assert_true((PDRead_find_data(reader, (void**)&data, &dataSize, "my_data", 0) & PDReadStatus_TypeMask) == PDReadType_Data);
    assert_true(dataSize == sizeof(s_data));
    assert_true(data[0] == s_data[0]);
    assert_true(data[1] == s_data[1]);
    assert_true(data[2] == s_data[2]);
    assert_true(data[5] == s_data[5]);

    assert_true(PDRead_get_event(reader) == 0);    // should be 0 as no more events
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   static void testNullReader(void**)
   {
    PDReader* t = 0;
    assert_true(PDRead_get_event(t) == 0);    // if null ptr for getEvent this should always return 0
   }
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testWriteReadAction(void**) {
    const char* filename = 0;
    uint32_t value = 0xfadebabe;
    PDBinaryWriter_reset(writer);

    PDWrite_event_begin(writer, PDEventType_SetExecutable);
    PDWrite_string(writer, "filename", "/Users/emoon/code/ProDBG/testbed/tundra-output/macosx-clang-debug-default/TestReadWrite");
    PDWrite_event_end(writer);

    PDWrite_event_begin(writer, PDEventType_Action);
    PDWrite_u32(writer, "mepa", 3);
    PDWrite_u32(writer, "action", 3);
    PDWrite_event_end(writer);

    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer));

    assert_true(PDRead_get_event(reader) == PDEventType_SetExecutable);
    assert_true(PDRead_find_string(reader, &filename, "filename", 0) == (PDReadStatus_Ok | PDReadType_String));
    assert_string_equal("/Users/emoon/code/ProDBG/testbed/tundra-output/macosx-clang-debug-default/TestReadWrite", filename);

    assert_true(PDRead_get_event(reader) == PDEventType_Action);
    assert_true(PDRead_find_u32(reader, &value, "action", 0) == (PDReadStatus_Ok | PDReadType_U32));
    assert_true(value == 3);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testWriteSingleString(void**) {
    PDBinaryWriter_reset(writer);

    assert_true(PDWrite_event_begin(writer, 2) == PDWriteStatus_ok);
    assert_true(PDWrite_string(writer, "my_id", "my_string") == PDWriteStatus_ok);
    assert_true(PDWrite_event_end(writer) == PDWriteStatus_ok);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   static void testReadSingleString(void**)
   {
    unsigned char* data;
    const char* keyName;
    const char* stringVal;
    unsigned int size;

    assert_true((data = PDBinaryWriter_getData(writer)) != 0);
    assert_true((size = PDBinaryWriter_getSize(writer)) != 0);

    PDBinaryReader_initStream(reader, data, size);

    assert_true(PDRead_get_event(reader) == 2);

    PDRead_string(reader, &stringVal, &keyName, 0);
    assert_string_equal(stringVal, "my_string");
    assert_string_equal(keyName, "my_id");

    assert_true(PDRead_get_event(reader) == 0);
   }
 */


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testAllValueTypes(void**) {
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

    // Write

    PDBinaryWriter_reset(writer);

    PDWrite_event_begin(writer, 3);

    assert_true(PDWrite_s8(writer, "my_s8", -2) == PDWriteStatus_ok);
    assert_true(PDWrite_u8(writer, "my_u8", 3) == PDWriteStatus_ok);

    assert_true(PDWrite_s16(writer, "my_s16", -2000) == PDWriteStatus_ok);
    assert_true(PDWrite_u16(writer, "my_u16", 56) == PDWriteStatus_ok);

    assert_true(PDWrite_s32(writer, "my_s32", -300000) == PDWriteStatus_ok);
    assert_true(PDWrite_u32(writer, "my_u32", 4000000) == PDWriteStatus_ok);

    assert_true(PDWrite_s64(writer, "my_s64", -1400000L) == PDWriteStatus_ok);
    assert_true(PDWrite_u64(writer, "my_u64", 6000000L) == PDWriteStatus_ok);

    assert_true(PDWrite_float(writer, "my_float", 14.0f) == PDWriteStatus_ok);
    assert_true(PDWrite_float(writer, "my_float2", -24.0f) == PDWriteStatus_ok);

    assert_true(PDWrite_double(writer, "my_double", 23.0) == PDWriteStatus_ok);
    assert_true(PDWrite_double(writer, "my_double2", 63.0) == PDWriteStatus_ok);

    assert_true(PDWrite_string(writer, "my_string", "foobar1337") == PDWriteStatus_ok);

    assert_true(PDWrite_data(writer, "my_data", s_data, sizeof(s_data)) == PDWriteStatus_ok);

    PDWrite_event_end(writer);

    PDBinaryWriter_finalize(writer);

    // Read

    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer));

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

void testHeaderArray(void**) {
    PDBinaryWriter_reset(writer);

    // Not implemented so expect it to always fail

    assert_true(PDWrite_header_array_begin(writer, 0) == PDWriteStatus_Fail);
    assert_true(PDWrite_header_array_end(writer) == PDWriteStatus_Fail);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testArrayWriteBreakage(void**) {
    PDBinaryWriter_reset(writer);

    assert_true(PDWrite_array_begin(writer, "test") == PDWriteStatus_ok);
    assert_true(PDWrite_array_begin(writer, "test") == PDWriteStatus_Fail); // Should fail here as we are already writing an array

    assert_true(PDWrite_array_end(writer) == PDWriteStatus_ok);
    assert_true(PDWrite_array_end(writer) == PDWriteStatus_Fail); // Should fail here as we are ended the array

    PDBinaryWriter_reset(writer);

    assert_true(PDWrite_array_begin(writer, "test") == PDWriteStatus_ok);
    assert_true(PDWrite_array_entry_begin(writer) == PDWriteStatus_ok);
    assert_true(PDWrite_array_entry_begin(writer) == PDWriteStatus_Fail); // Must call end before new Begin

    assert_true(PDWrite_entry_end(writer) == PDWriteStatus_ok);
    assert_true(PDWrite_entry_end(writer) == PDWriteStatus_Fail); // must call begin before new End

    assert_true(PDWrite_array_end(writer) == PDWriteStatus_ok);

    PDBinaryWriter_reset(writer);

    assert_true(PDWrite_event_begin(writer, 10) == PDWriteStatus_ok);
    assert_true(PDWrite_event_begin(writer, 10) == PDWriteStatus_Fail);  // Must end even before new event

    assert_true(PDWrite_event_end(writer) == PDWriteStatus_ok);
    assert_true(PDWrite_event_end(writer) == PDWriteStatus_Fail);    // Can't end event wtire
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testFind(void**) {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;

    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer));

    assert_true(PDRead_get_event(reader) == 3);

    // u16

    assert_true(PDRead_find_u16(reader, &u16, "my_u16", 0) == (PDReadStatus_Ok | PDReadType_U16));
    assert_true(u16 == 56);

    assert_true(PDRead_find_u32(reader, &u32, "my_u32", 0) == (PDReadStatus_Ok | PDReadType_U32));
    assert_true(u32 == 4000000);

    assert_true(PDRead_find_u8(reader, &u8, "my_u16", 0) == (PDReadStatus_Converted | PDReadType_U16));
    assert_true(u8 == 56);

    assert_true(PDRead_get_event(reader) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testArray(void**) {
    // TODO: More tests

    PDBinaryWriter_reset(writer);

    PDWrite_event_begin(writer, 5);

    // write two entries in the array

    PDWrite_array_begin(writer, "items");

    // Entry on

    PDWrite_array_entry_begin(writer);
    PDWrite_string(writer, "test", "some test data");
    PDWrite_string(writer, "more_test", "and even more test data");
    PDWrite_string(writer, "and_some_more", "m0r3 t3zt0r");
    PDWrite_entry_end(writer);

    // Entry two

    PDWrite_array_entry_begin(writer);

    assert_true(PDWrite_s8(writer, "my_s8", -2) == PDWriteStatus_ok);
    assert_true(PDWrite_u8(writer, "my_u8", 3) == PDWriteStatus_ok);

    assert_true(PDWrite_s16(writer, "my_s16", -2000) == PDWriteStatus_ok);
    assert_true(PDWrite_u16(writer, "my_u16", 56) == PDWriteStatus_ok);

    assert_true(PDWrite_s32(writer, "my_s32", -300000) == PDWriteStatus_ok);
    assert_true(PDWrite_u32(writer, "my_u32", 4000000) == PDWriteStatus_ok);

    assert_true(PDWrite_s64(writer, "my_s64", -1400000L) == PDWriteStatus_ok);
    assert_true(PDWrite_u64(writer, "my_u64", 6000000L) == PDWriteStatus_ok);

    assert_true(PDWrite_float(writer, "my_float", 14.0f) == PDWriteStatus_ok);
    assert_true(PDWrite_float(writer, "my_float2", -24.0f) == PDWriteStatus_ok);

    assert_true(PDWrite_double(writer, "my_double", 23.0) == PDWriteStatus_ok);
    assert_true(PDWrite_double(writer, "my_double2", 63.0) == PDWriteStatus_ok);

    assert_true(PDWrite_string(writer, "my_string", "foobar1337") == PDWriteStatus_ok);

    assert_true(PDWrite_data(writer, "my_data", s_data, sizeof(s_data)) == PDWriteStatus_ok);

    PDWrite_entry_end(writer);

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testArrayRead(void**) {
    PDReaderIterator arrayIter;

    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer));

    assert_true(PDRead_get_event(reader) == 5);

    assert_true((PDRead_find_array(reader, &arrayIter, "items", 0) & PDReadStatus_TypeMask) == PDReadType_Array);

    // make sure we actually found the array here before we try to do some more stuff

    if (arrayIter) {
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
        const char* string1;
        uint8_t* data;
        uint64_t size;

        assert_true(PDRead_get_next_entry(reader, &arrayIter) == 3);

        assert_true(PDRead_find_string(reader, &string1, "test", arrayIter) == (PDReadType_String | PDReadStatus_Ok));
        assert_string_equal("some test data", string1);

        assert_true(PDRead_find_string(reader, &string1, "illegal id", arrayIter) == PDReadStatus_NotFound);

        assert_int_equal(PDRead_get_next_entry(reader, &arrayIter), 14);

        // s8

        assert_true((PDRead_find_s8(reader, &s8, "my_s8", arrayIter) & PDReadStatus_TypeMask) == PDReadType_S8);
        assert_true(s8 == -2);

        // u8

        assert_true((PDRead_find_u8(reader, &u8, "my_u8", arrayIter) & PDReadStatus_TypeMask) == PDReadType_U8);
        assert_true(u8 == 3);

        // s16

        assert_true((PDRead_find_s16(reader, &s16, "my_s16", arrayIter) & PDReadStatus_TypeMask) == PDReadType_S16);
        assert_true(s16 == -2000);

        // u16

        assert_true((PDRead_find_u16(reader, &u16, "my_u16", arrayIter) & PDReadStatus_TypeMask) == PDReadType_U16);
        assert_true(u16 == 56);

        // s32

        assert_true((PDRead_find_s32(reader, &s32, "my_s32", arrayIter) & PDReadStatus_TypeMask) == PDReadType_S32);
        assert_true(s32 == -300000);

        // u32

        assert_true((PDRead_find_u32(reader, &u32, "my_u32", arrayIter) & PDReadStatus_TypeMask) == PDReadType_U32);
        assert_true(u32 == 4000000);

        // s64

        assert_true((PDRead_find_s64(reader, &s64, "my_s64", arrayIter) & PDReadStatus_TypeMask) == PDReadType_S64);
        assert_true(s64 == -1400000L);

        // u64

        assert_true((PDRead_find_u64(reader, &u64, "my_u64", arrayIter) & PDReadStatus_TypeMask) == PDReadType_U64);
        assert_true(u64 == 6000000L);

        // float

        assert_true((PDRead_find_float(reader, &fvalue, "my_float", arrayIter) & PDReadStatus_TypeMask) == PDReadType_Float);
        assert_true(fvalue == 14.0f);

        // float 2

        assert_true((PDRead_find_float(reader, &fvalue, "my_float2", arrayIter) & PDReadStatus_TypeMask) == PDReadType_Float);
        assert_true(((fvalue > (-24.0f - 0.001f)) && fvalue < ((-24.0f + 0.0001f))));

        // double

        assert_true((PDRead_find_double(reader, &dvalue, "my_double", arrayIter) & PDReadStatus_TypeMask) == PDReadType_Double);
        assert_true(dvalue == 23.0);

        // double 2

        assert_true((PDRead_find_double(reader, &dvalue, "my_double2", arrayIter) & PDReadStatus_TypeMask) == PDReadType_Double);
        assert_true(dvalue == 63.0);

        // string

        assert_true((PDRead_find_string(reader, &string, "my_string", arrayIter) & PDReadStatus_TypeMask) == PDReadType_String);
        assert_string_equal(string, "foobar1337");

        assert_true((PDRead_find_data(reader, (void**)&data, &size, "my_data", arrayIter) & PDReadStatus_TypeMask) == PDReadType_Data);
        assert_true(size == sizeof(s_data));
        assert_true(data[0] == s_data[0]);
        assert_true(data[1] == s_data[1]);
        assert_true(data[2] == s_data[2]);
        assert_true(data[5] == s_data[5]);

        assert_true(PDRead_get_next_entry(reader, &arrayIter) == 0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    log_set_level(LOG_ERROR);

    const UnitTest tests[] =
    {
        unit_test(testWriteReadEvent),
        unit_test(testDataReadWrite),
        unit_test(testWriteSingleString),
        unit_test(testWriteReadAction),
        unit_test(testArrayWriteBreakage),
        unit_test(testAllValueTypes),
        unit_test(testFind),
        unit_test(testArray),
        unit_test(testArrayRead),
        unit_test(testHeaderArray),
    };

    reader = &readerData;
    writer = &writerData;

    PDBinaryReader_init(reader);
    PDBinaryWriter_init(writer);

    int test = run_tests(tests);

    PDBinaryReader_destroy(reader);
    PDBinaryWriter_destroy(writer);

    return test;
}

