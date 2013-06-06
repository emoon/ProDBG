#include "BinarySerializer.h"
#include "core/Log.h"
#include <ProDBGAPI.h>
#include <string.h>
#include <stdlib.h>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BinarySerializerData
{
	uint8_t* dataStart;
	int maxAllocSize;
	int readOffset;

	int writeOffset;
	int readSaveOffset;

	int beginEventOffset;
	int writeEventStarted; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reader and writer functions

static void writeInt(void* writeData, int v)
{
	BinarySerializerData* data = (BinarySerializerData*)writeData;

	int writeOffset = data->writeOffset;

	if (writeOffset + 4 <= data->maxAllocSize)
	{
		uint8_t* writePtr = data->dataStart + writeOffset;

		writePtr[0] = (v >> 24) & 0xff;
		writePtr[1] = (v >> 16) & 0xff;
		writePtr[2] = (v >> 8) & 0xff;
		writePtr[3] = (v >> 0) & 0xff;

		data->writeOffset = writeOffset + 4;
	}
	else
	{
		log_error("Unable to write to serializeData because it's full (writeOffset %d maxSize %d)\n", 
				  writeOffset, data->maxAllocSize);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeString(void* writeData, const char* string)
{
	BinarySerializerData* data = (BinarySerializerData*)writeData;

	int writeOffset = data->writeOffset;

	int len = (int)strlen(string) + 1;

	//log_debug("WriteString %s (length %d) writeOffset %d\n", string, len, writeOffset);

	if (writeOffset + len <= data->maxAllocSize)
	{
		uint8_t* writePtr = data->dataStart + writeOffset;
		memcpy(writePtr, string, (size_t)len); // memcpy as we want to include the 0 at the end
		data->writeOffset = writeOffset + len;
	}
	else
	{
		log_error("Unable to write to serializeData because it's full (writeOffset %d maxSize %d)\n", 
				  writeOffset, data->maxAllocSize);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* readString(void* readData)
{
	BinarySerializerData* data = (BinarySerializerData*)readData;

	log_debug("readOffset %d\n", data->readOffset);

	const char* string = (const char*)(data->dataStart + data->readOffset);
	data->readOffset += strlen(string) + 1;

	if (data->readOffset >= data->writeOffset)
	{
		log_error("Reading past end of writeData (readOffset %d writeOffset %d)\n", data->readOffset, data->writeOffset);
	}

	return string; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int readInt(void* readData)
{
	BinarySerializerData* data = (BinarySerializerData*)readData;

	uint8_t* intData = (uint8_t*)(data->dataStart + data->readOffset);

	int v = (intData[0] << 24) | (intData[1] << 16) | (intData[2] << 8) | intData[3];

	log_debug("readInt %d offset %d\n", v, data->readOffset);

	data->readOffset += 4;

	if (data->readOffset >= data->writeOffset)
	{
		log_error("Reading past end of writeData (readOffset %d writeOffset %d)\n", data->readOffset, data->writeOffset);
	}

	return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int bytesLeft(void* readData)
{
	BinarySerializerData* data = (BinarySerializerData*)readData;
	return data->writeOffset - data->readOffset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void skipBytes(void* readData, int size)
{
	BinarySerializerData* data = (BinarySerializerData*)readData;
	data->readOffset += size;

	if (data->readOffset >= data->writeOffset)
	{
		log_error("Skiping past end of writeData (readOffset %d writeOffset %d)\n", data->readOffset, data->writeOffset);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_saveReadOffset(struct PDSerializeRead* reader)
{
	BinarySerializerData* data = (BinarySerializerData*)reader->readData;
	data->readSaveOffset = data->readOffset;
	log_debug("data->readSaveOffset = %d\n", data->readSaveOffset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_gotoNextOffset(struct PDSerializeRead* reader, int offset)
{
	BinarySerializerData* data = (BinarySerializerData*)reader->readData;
	data->readOffset = data->readSaveOffset + offset;
	log_debug("data->readOffset = %d\n", data->readOffset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_initWriter(struct PDSerializeWrite* writer)
{
	BinarySerializerData* data = (BinarySerializerData*)malloc(sizeof(BinarySerializerData));
	memset(data, 0, sizeof(BinarySerializerData));

	data->maxAllocSize = 256 * 1024;
	data->dataStart = (uint8_t*)malloc((size_t)data->maxAllocSize);

	log_debug("writerData %p\n", data->dataStart);

	writer->writeData = data;
	writer->writeInt = writeInt;
	writer->writeString = writeString;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerialize_beginEvent(struct PDSerializeWrite* writer, PDEventType eventType, int eventId)
{
	BinarySerializerData* data = (BinarySerializerData*)writer->writeData;

	// event id should be 0 at this point (endEvent should have been called)

	if (data->writeEventStarted)
		log_error("Event is still in progress yet beginEvent has been called. Make sure to call endEvent first\n");

	// store start of the event (where we are going to write the size of the event
	data->beginEventOffset = data->writeOffset;
	data->writeEventStarted = 1;

	PDWRITE_INT(writer, 0);		// size (will be filled in later
	PDWRITE_INT(writer, (int)eventType);		
	PDWRITE_INT(writer, eventId);		
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerialize_endEvent(struct PDSerializeWrite* writer)
{
	BinarySerializerData* data = (BinarySerializerData*)writer->writeData;

	if (!data->writeEventStarted)
		log_error("No event has been started. Make sure to call beginEvent first\n");

	// write the correct size 

	int size = data->writeOffset - data->beginEventOffset;
	uint8_t* writePtr = data->dataStart + data->beginEventOffset;

	writePtr[0] = (size >> 24) & 0xff;
	writePtr[1] = (size >> 16) & 0xff;
	writePtr[2] = (size >> 8) & 0xff;
	writePtr[3] = (size >> 0) & 0xff;

	data->writeEventStarted = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_initReader(struct PDSerializeRead* reader, void* data)
{
	reader->readData = data;
	reader->readInt = readInt;
	reader->readString = readString;
	reader->bytesLeft = bytesLeft;
	reader->skipBytes = skipBytes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_destroyData(void* serData)
{
	BinarySerializerData* data = (BinarySerializerData*)serData;

	free(data->dataStart);

	// pattern so if there is a bug and we read from delete memory it's easier to catch
	memset(data, 0xcd, sizeof(BinarySerializerData));

	free(data);
}

}

