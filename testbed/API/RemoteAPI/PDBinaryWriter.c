#include "../PDReadWrite.h"
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WriterData
{
	int foo;
} WriterData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeEventBegin(struct PDWriter* writer, int event)
{
	(void)writer;
	(void)event;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeEventEnd(struct PDWriter* writer)
{
	(void)writer;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeHeaderArrayBegin(struct PDWriter* writer, const char** ids)
{
	(void)writer;
	(void)ids;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeHeaderArrayEnd(struct PDWriter* writer)
{
	(void)writer;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeArrayBegin(struct PDWriter* writer, const char* name)
{
	(void)writer;
	(void)name;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeArrayEnd(struct PDWriter* writer)
{
	(void)writer;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeArrayEntryBegin(struct PDWriter* writer)
{
	(void)writer;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeArrayEntryEnd(struct PDWriter* writer)
{
	(void)writer;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeS8(struct PDWriter* writer, const char* id, int8_t v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeU8(struct PDWriter* writer, const char* id, uint8_t v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeS16(struct PDWriter* writer, const char* id, int16_t v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeU16(struct PDWriter* writer, const char* id, uint16_t v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeS32(struct PDWriter* writer, const char* id, int32_t v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeU32(struct PDWriter* writer, const char* id, uint32_t v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeS64(struct PDWriter* writer, const char* id, int64_t v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeU64(struct PDWriter* writer, const char* id, uint64_t v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeFloat(struct PDWriter* writer, const char* id, float v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeDouble(struct PDWriter* writer, const char* id, double v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeString(struct PDWriter* writer, const char* id, const char* v)
{
	(void)writer;
	(void)id;
	(void)v;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus writeData(struct PDWriter* writer, const char* id, void* data, unsigned int len)
{
	(void)writer;
	(void)id;
	(void)data;
	(void)len;
	return PDWriteStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryWriter_init(PDWriter* writer)
{
	writer->writeEventBegin = writeEventBegin;
	writer->writeEventEnd = writeEventEnd;
	writer->writeHeaderArrayBegin = writeHeaderArrayBegin;
	writer->writeHeaderArrayEnd = writeHeaderArrayEnd;
	writer->writeArrayBegin = writeArrayBegin;
	writer->writeArrayEnd = writeArrayEnd;
	writer->writeArrayEntryBegin = writeArrayEntryBegin;
	writer->writeArrayEntryEnd = writeArrayEntryEnd;
	writer->writeS8 = writeS8;
	writer->writeU8 = writeU8;
	writer->writeS16 = writeS16;
	writer->writeU16 = writeU16;
	writer->writeS32 = writeS32;
	writer->writeU32 = writeU32;
	writer->writeS64 = writeS64;
	writer->writeU64 = writeU64;
	writer->writeFloat = writeFloat;
	writer->writeDouble = writeDouble;
	writer->writeString = writeString;
	writer->writeData = writeData;

	writer->data = malloc(sizeof(WriterData));
	memset(writer->data, 0, sizeof(WriterData));
}

