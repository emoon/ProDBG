#include <CuTest.h>
#include <stdio.h>
#include <ProDBGAPI.h>
#include <PDReadWrite.h>

static PDReader readerData;
static PDWriter writerData;
static PDReader* reader = 0; 
static PDWriter* writer = 0; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// These are private functions that is needed to setup the reader/writer

void PDBinaryWriter_init(PDWriter* writer);
void PDBinaryWriter_reset(PDWriter* writer);
void* PDBinaryWriter_getData(PDWriter* writer);
unsigned int PDBinaryWriter_getSize(PDWriter* writer);

void PDBinaryReader_init(PDReader* reader);
void PDBinaryReader_initStream(PDReader* reader, void* data, unsigned int size);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testWriteEvent(CuTest* tc)
{
	PDBinaryWriter_reset(writer);
	CuAssertTrue(tc, PDWrite_eventBegin(writer, 1) == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_eventEnd(writer) == PDWriteStatus_ok);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testReadEvent(CuTest* tc)
{
	void* data;
	unsigned int size;
	PDReaderIterator it;

	CuAssertTrue(tc, (data = PDBinaryWriter_getData(writer)) != 0);
	CuAssertTrue(tc, (size = PDBinaryWriter_getSize(writer)) != 0);

	PDBinaryReader_initStream(reader, data, size); 

	CuAssertTrue(tc, PDRead_iteratorBeginEvent(reader, &it) == 1);	// expect 1 as written testWriteEvent
	CuAssertTrue(tc, PDRead_iteratorNextEvent(reader, &it) == 0);	// should be 0 as no more events
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testWriteSingleString(CuTest* tc)
{
	PDBinaryWriter_reset(writer);

	CuAssertTrue(tc, PDWrite_eventBegin(writer, 2) == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_string(writer, "my_id", "my_string") == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_eventEnd(writer) == PDWriteStatus_ok);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testReadSingleString(CuTest* tc)
{
	void* data;
	const char* keyName;
	const char* keyNameTemp;
	const char* stringVal;
	unsigned int size;
	PDReaderIterator eventIt;
	PDReaderIterator it;

	CuAssertTrue(tc, (data = PDBinaryWriter_getData(writer)) != 0);
	CuAssertTrue(tc, (size = PDBinaryWriter_getSize(writer)) != 0);

	PDBinaryReader_initStream(reader, data, size); 

	CuAssertTrue(tc, PDRead_iteratorBeginEvent(reader, &eventIt) == 2);
	CuAssertTrue(tc, (PDRead_iteratorBegin(reader, &it, &keyName, eventIt) & PDReadStatus_typeMask) == PDReadType_string);

	CuAssertStrEquals(tc, keyName, "my_id");

	PDRead_string(reader, &stringVal, &keyNameTemp, &it);
	CuAssertStrEquals(tc, stringVal, "my_string");
	CuAssertStrEquals(tc, keyNameTemp, "my_id");

	CuAssertTrue(tc, PDRead_iteratorNextEvent(reader, &eventIt) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t s_data[] = { 1, 2, 3, 80, 50, 60 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testAllValueTypes(CuTest* tc)
{
	PDBinaryWriter_reset(writer);

	PDWrite_eventBegin(writer, 3);

	CuAssertTrue(tc, PDWrite_s8(writer, "my_s8", -2) == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_u8(writer, "my_u8", 3) == PDWriteStatus_ok);

	CuAssertTrue(tc, PDWrite_s16(writer, "my_s16", -2000) == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_u16(writer, "my_u16", 3000) == PDWriteStatus_ok);

	CuAssertTrue(tc, PDWrite_s32(writer, "my_s32", -300000) == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_u32(writer, "my_u32", 4000000) == PDWriteStatus_ok);

	CuAssertTrue(tc, PDWrite_s64(writer, "my_s64", -1400000L) == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_u64(writer, "my_u64", 6000000L) == PDWriteStatus_ok);

	CuAssertTrue(tc, PDWrite_float(writer, "my_float", 14.0f) == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_float(writer, "my_float2", -24.0f) == PDWriteStatus_ok);

	CuAssertTrue(tc, PDWrite_double(writer, "my_double", 23.0) == PDWriteStatus_ok);
	CuAssertTrue(tc, PDWrite_double(writer, "my_double2", 63.0) == PDWriteStatus_ok);

	CuAssertTrue(tc, PDWrite_string(writer, "my_string", "foobar1337") == PDWriteStatus_ok);

	CuAssertTrue(tc, PDWrite_data(writer, "my_data", s_data, sizeof(s_data)) == PDWriteStatus_ok);

	PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testAllValuesRead(CuTest* tc)
{
	int8_t s8;
	uint8_t u8;
	int16_t s16;
	uint16_t u16;
	int32_t s32;
	uint32_t u32;
	int64_t s64;
	uint64_t u64;
	float fvalue;
	double dvalue;
	const char* string;
	uint8_t* data;
	unsigned int size;
	const char* s8_id;
	const char* u8_id;
	const char* s16_id;
	const char* u16_id;
	const char* s32_id;
	const char* u32_id;
	const char* s64_id;
	const char* u64_id;
	const char* float_id;
	const char* double_id;
	const char* string_id;
	const char* data_id;
	PDReaderIterator eventIt;
	PDReaderIterator it;

	PDBinaryReader_initStream(reader, PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer)); 

	CuAssertTrue(tc, PDRead_iteratorBeginEvent(reader, &eventIt) == 2);
	CuAssertTrue(tc, (PDRead_iteratorBegin(reader, &it, 0, eventIt) & PDReadStatus_typeMask) == PDReadType_s8);

	// s8

	CuAssertTrue(tc, (PDRead_s8(reader, &s8, &s8_id, &it) & PDReadStatus_typeMask) == PDReadType_s8);
	CuAssertStrEquals(tc, s8_id, "my_s8");
	CuAssertTrue(tc, s8 == -2);  

	// u8

	CuAssertTrue(tc, (PDRead_u8(reader, &u8, &u8_id, &it) & PDReadStatus_typeMask) == PDReadType_u8);
	CuAssertStrEquals(tc, s8_id, "my_u8");
	CuAssertTrue(tc, u8 == 3);  

	// s16

	CuAssertTrue(tc, (PDRead_s16(reader, &s16, &s16_id, &it) & PDReadStatus_typeMask) == PDReadType_s16);
	CuAssertStrEquals(tc, s16_id, "my_s16");
	CuAssertTrue(tc, s16 == -2000);  

	// u16

	CuAssertTrue(tc, (PDRead_u16(reader, &u16, &u16_id, &it) & PDReadStatus_typeMask) == PDReadType_u16);
	CuAssertStrEquals(tc, u16_id, "my_u16");
	CuAssertTrue(tc, u16 == 3000);  

	// s32

	CuAssertTrue(tc, (PDRead_s32(reader, &s32, &s32_id, &it) & PDReadStatus_typeMask) == PDReadType_s32);
	CuAssertStrEquals(tc, s32_id, "my_s32");
	CuAssertTrue(tc, s32 == -300000);  

	// u32

	CuAssertTrue(tc, (PDRead_u32(reader, &u32, &u32_id, &it) & PDReadStatus_typeMask) == PDReadType_u32);
	CuAssertStrEquals(tc, u32_id, "my_u32");
	CuAssertTrue(tc, u32 == 4000000);  

	// s64

	CuAssertTrue(tc, (PDRead_s64(reader, &s64, &s64_id, &it) & PDReadStatus_typeMask) == PDReadType_s64);
	CuAssertStrEquals(tc, s64_id, "my_s64");
	CuAssertTrue(tc, s64 == -1400000L);  

	// u64

	CuAssertTrue(tc, (PDRead_u64(reader, &u64, &u64_id, &it) & PDReadStatus_typeMask) == PDReadType_u64);
	CuAssertStrEquals(tc, u64_id, "my_u64");
	CuAssertTrue(tc, u64 == 6000000L);  

	// float

	CuAssertTrue(tc, (PDRead_float(reader, &fvalue, &float_id, &it) & PDReadStatus_typeMask) == PDReadType_float);
	CuAssertStrEquals(tc, float_id, "my_float");
	CuAssertTrue(tc, fvalue == 14.0f);  

	// float 2

	CuAssertTrue(tc, (PDRead_float(reader, &fvalue, &float_id, &it) & PDReadStatus_typeMask) == PDReadType_float);
	CuAssertStrEquals(tc, float_id, "my_float2");
	CuAssertTrue(tc, fvalue > -24.0f + 0.001f && fvalue < -24.0f - 0.0001f);  

	// double

	CuAssertTrue(tc, (PDRead_double(reader, &dvalue, &double_id, &it) & PDReadStatus_typeMask) == PDReadType_double);
	CuAssertStrEquals(tc, double_id, "my_double");
	CuAssertTrue(tc, dvalue == 23.0f);  

	// double 2

	CuAssertTrue(tc, (PDRead_double(reader, &dvalue, &double_id, &it) & PDReadStatus_typeMask) == PDReadType_double);
	CuAssertStrEquals(tc, double_id, "my_double2");
	CuAssertTrue(tc, dvalue == 63.0);  

	// string 

	CuAssertTrue(tc, (PDRead_string(reader, &string, &string_id, &it) & PDReadStatus_typeMask) == PDReadType_string);
	CuAssertStrEquals(tc, string_id, "my_string");
	CuAssertStrEquals(tc, string, "fobar1337");

	// data

	CuAssertTrue(tc, (PDRead_data(reader, (void*)&data, &size, &data_id, &it) & PDReadStatus_typeMask) == PDReadType_string);
	CuAssertStrEquals(tc, data_id, "my_data");
	CuAssertTrue(tc, size == sizeof(s_data));  
	CuAssertTrue(tc, data[0] == s_data[0]); 
	CuAssertTrue(tc, data[1] == s_data[1]); 
	CuAssertTrue(tc, data[2] == s_data[2]); 
	CuAssertTrue(tc, data[5] == s_data[5]); 

	CuAssertTrue(tc, PDRead_iteratorNextEvent(reader, &eventIt) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testFind(CuTest* tc)
{
	uint16_t u16;
	PDReaderIterator eventIt;

	PDBinaryReader_initStream(reader, PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer)); 

	CuAssertTrue(tc, PDRead_iteratorBeginEvent(reader, &eventIt) == 2);

	// u16

	CuAssertTrue(tc, PDRead_findU16(reader, &u16, "my_u16", eventIt) == (PDReadStatus_ok | PDReadType_u16));
	CuAssertTrue(tc, u16 == 3000);  

	CuAssertTrue(tc, PDRead_iteratorNextEvent(reader, &eventIt) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testArray(CuTest* cu)
{
	(void)cu;

	// TODO: More tests

	PDBinaryWriter_reset(writer);

	PDWrite_eventBegin(writer, 4);

	// write two entries in the array

	PDWrite_arrayBegin(writer, "items");

	// Entry one

	PDWrite_arrayEntryBegin(writer);
	PDWrite_string(writer, "test", "some test data");
	PDWrite_string(writer, "more_test", "and even more test data");
	PDWrite_arrayEntryEnd(writer);

	// Entry two

	PDWrite_arrayEntryBegin(writer);
	PDWrite_s8(writer, "test", 2);
	PDWrite_u16(writer, "more_test", 0x7870);
	PDWrite_arrayEntryEnd(writer);

	PDWrite_arrayEnd(writer);
	PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testArrayRead(CuTest* tc)
{
	PDReaderIterator eventIt;
	PDReaderIterator arrayIter;

	PDBinaryReader_initStream(reader, PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer)); 

	CuAssertTrue(tc, PDRead_iteratorBeginEvent(reader, &eventIt) == 4);

	CuAssertTrue(tc, (PDRead_findArray(reader, &arrayIter, "items", eventIt) & PDReadStatus_typeMask) == PDReadType_array);

	// make sure we actually found the array here before we try to do some more stuff

	if (arrayIter)
	{
		int8_t s8;
		const char* string1;

		CuAssertTrue(tc, PDRead_findString(reader, &string1, "test", arrayIter) == (PDReadType_string | PDReadStatus_ok));
		CuAssertStrEquals(tc, string1, "some test data");

		CuAssertTrue(tc, PDRead_findString(reader, &string1, "illegal id", arrayIter) == PDReadStatus_notFound);

		PDRead_iteratorNext(reader, 0, &arrayIter);

		CuAssertTrue(tc, PDRead_findS8(reader, &s8, "test", arrayIter) == (PDReadType_s8 | PDReadStatus_ok));
		CuAssertTrue(tc, s8 == 2);
	}

	CuAssertTrue(tc, PDRead_iteratorNextEvent(reader, &eventIt) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CuSuite* getSuite()
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, testWriteEvent);
	SUITE_ADD_TEST(suite, testReadEvent);
	SUITE_ADD_TEST(suite, testWriteSingleString);
	SUITE_ADD_TEST(suite, testReadSingleString);
	SUITE_ADD_TEST(suite, testAllValueTypes);
	SUITE_ADD_TEST(suite, testAllValuesRead);
	SUITE_ADD_TEST(suite, testAllValueTypes);
	SUITE_ADD_TEST(suite, testFind);
	SUITE_ADD_TEST(suite, testArray);
	SUITE_ADD_TEST(suite, testArrayRead);

	return suite;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void runAllTests()
{
	CuString* output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, getSuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	reader = &readerData;
	writer = &writerData;

	PDBinaryReader_init(reader);
	PDBinaryWriter_init(writer);

	runAllTests();
	return 0;
}
