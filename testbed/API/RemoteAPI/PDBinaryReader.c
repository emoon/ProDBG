#include "../PDReadWrite.h"
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ReaderData
{
	void* data;
	unsigned int size;
} ReaderData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int8_t getS8(uint8_t* ptr)
{
	return (int8_t)ptr[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint8_t getU8(uint8_t* ptr)
{
	return ptr[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint16_t getS16(uint8_t* ptr)
{
	int16_t v = (ptr[0] << 8) | ptr[1];
	return v; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint16_t getU16(uint8_t* ptr)
{
	uint16_t v = (ptr[0] << 8) | ptr[1];
	return v; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int32_t getS32(uint8_t* ptr)
{
	int32_t v = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
	return v; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int32_t getU32(uint8_t* ptr)
{
	uint32_t v = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
	return v; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int64_t getS64(uint8_t* ptr)
{
	int64_t v = ((uint64_t)ptr[0] << 56) | ((uint64_t)ptr[1] << 48) | ((uint64_t)ptr[2] << 40) | ((uint64_t)ptr[3] << 32) |
				(ptr[4] << 24) | (ptr[5] << 16) | (ptr[6]  << 8) | ptr[7];
	return v; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint64_t getU64(uint8_t* ptr)
{
	uint64_t v = ((uint64_t)ptr[0] << 56) | ((uint64_t)ptr[1] << 48) | ((uint64_t)ptr[2] << 40) | ((uint64_t)ptr[3] << 32) |
				(ptr[4] << 24) | (ptr[5] << 16) | (ptr[6]  << 8) | ptr[7];
	return v; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readIteratorBeginEvent(struct PDReader* reader, PDReaderIterator* it)
{
	(void)reader;
	(void)it;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readIteratorNextEvent(struct PDReader* reader, PDReaderIterator* it)
{
	(void)reader;
	(void)it;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readIteratorBegin(struct PDReader* reader, PDReaderIterator* it, const char** keyName, PDReaderIterator parentIt)
{
	(void)reader;
	(void)it;
	(void)keyName;
	(void)parentIt;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readIteratorNext(struct PDReader* reader, const char** keyName, PDReaderIterator* it)
{
	(void)reader;
	(void)keyName;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindS8(struct PDReader* reader, int8_t* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindU8(struct PDReader* reader, uint8_t* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindS16(struct PDReader* reader, int16_t* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindU16(struct PDReader* reader, uint16_t* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindS32(struct PDReader* reader, int32_t* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindU32(struct PDReader* reader, uint32_t* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindS64(struct PDReader* reader, int64_t* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindU64(struct PDReader* reader, uint64_t* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindFloat(struct PDReader* reader, float* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindDouble(struct PDReader* reader, double* res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindString(struct PDReader* reader, const char** res, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindData(struct PDReader* reader, void** data, uint64_t* size, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)data;
	(void)size;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFindArray(struct PDReader* reader, PDReaderIterator* arrayIt, const char* id, PDReaderIterator it)
{
	(void)reader;
	(void)arrayIt;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readS8(struct PDReader* reader, int8_t* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readU8(struct PDReader* reader, uint8_t* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readS16(struct PDReader* reader, int16_t* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readU16(struct PDReader* reader, uint16_t* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readS32(struct PDReader* reader, int32_t* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readU32(struct PDReader* reader, uint32_t* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readS64(struct PDReader* reader, int64_t* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readU64(struct PDReader* reader, uint64_t* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readFloat(struct PDReader* reader, float* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readDouble(struct PDReader* reader, double* res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readString(struct PDReader* reader, const char** res, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readData(struct PDReader* reader, void** res, unsigned int* size, const char** id, PDReaderIterator* it)
{
	(void)reader;
	(void)res;
	(void)size;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t readArray(struct PDReader* reader, PDReaderIterator* arrayIt, const char* id, PDReaderIterator* it)
{
	(void)reader;
	(void)arrayIt;
	(void)id;
	(void)it;
	return PDReadType_none | PDReadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryReader_init(PDReader* reader)
{
	reader->readIteratorBeginEvent = readIteratorBeginEvent;
	reader->readIteratorNextEvent = readIteratorNextEvent;
	reader->readIteratorBegin = readIteratorBegin;
	reader->readIteratorNext = readIteratorNext;
	reader->readFindS8 = readFindS8;
	reader->readFindU8 = readFindU8;
	reader->readFindS16 = readFindS16;
	reader->readFindU16 = readFindU16;
	reader->readFindS32 = readFindS32;
	reader->readFindU32 = readFindU32;
	reader->readFindS64 = readFindS64;
	reader->readFindU64 = readFindU64;
	reader->readFindFloat = readFindFloat;
	reader->readFindDouble = readFindDouble;
	reader->readFindString = readFindString;
	reader->readFindData = readFindData;
	reader->readFindArray = readFindArray;
	reader->readS8 = readS8;
	reader->readU8 = readU8;
	reader->readS16 = readS16;
	reader->readU16 = readU16;
	reader->readS32 = readS32;
	reader->readU32 = readU32;
	reader->readS64 = readS64;
	reader->readU64 = readU64;
	reader->readFloat = readFloat;
	reader->readDouble = readDouble;
	reader->readString = readString;
	reader->readData = readData;
	reader->readArray = readArray;

	reader->data = malloc(sizeof(ReaderData));
	memset(reader->data, 0, sizeof(ReaderData));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryReader_initStream(PDReader* reader, void* data, unsigned int size)
{
	ReaderData* readerData = (ReaderData*)reader->data;
	readerData->data = data;
	readerData->size = size;
}


