#include "BinarySerializer.h"
#include <ProDBGAPI.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct BinarySerializerData
{
	uint8_t* dataStart;
	int maxAllocSize;
	int readOffset;

	int writeOffset;
	int readSaveOffset;

	int beginEventOffset;
	int writeEventStarted; 
} BinarySerializerData;

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
		// \todo proper logging macro
		printf("Unable to write to serializeData because it's full (writeOffset %d maxSize %d)\n", 
			   writeOffset, data->maxAllocSize);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeU16(void* writeData, uint16_t v)
{
	BinarySerializerData* data = (BinarySerializerData*)writeData;

	int writeOffset = data->writeOffset;

	if (writeOffset + 2 <= data->maxAllocSize)
	{
		uint8_t* writePtr = data->dataStart + writeOffset;

		writePtr[0] = (v >> 8) & 0xff;
		writePtr[1] = (v >> 0) & 0xff;

		data->writeOffset = writeOffset + 2;
	}
	else
	{
		// \todo proper logging macro
		printf("Unable to write to serializeData because it's full (writeOffset %d maxSize %d)\n", 
			   writeOffset, data->maxAllocSize);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeU8(void* writeData, uint8_t v)
{
	BinarySerializerData* data = (BinarySerializerData*)writeData;

	int writeOffset = data->writeOffset;

	if (writeOffset + 1 <= data->maxAllocSize)
	{
		uint8_t* writePtr = data->dataStart + writeOffset;

		writePtr[0] = (v >> 0) & 0xff;

		data->writeOffset = writeOffset + 1;
	}
	else
	{
		// \todo proper logging macro
		printf("Unable to write to serializeData because it's full (writeOffset %d maxSize %d)\n", 
			   writeOffset, data->maxAllocSize);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeString(void* writeData, const char* string)
{
	BinarySerializerData* data = (BinarySerializerData*)writeData;

	int writeOffset = data->writeOffset;

	int len = (int)strlen(string) + 1;

	if (writeOffset + len <= data->maxAllocSize)
	{
		uint8_t* writePtr = data->dataStart + writeOffset;
		memcpy(writePtr, string, (size_t)len); // memcpy as we want to include the 0 at the end
		data->writeOffset = writeOffset + len;
	}
	else
	{
		// \todo proper logging macro
		printf("Unable to write to serializeData because it's full (writeOffset %d maxSize %d)\n", 
			   writeOffset, data->maxAllocSize);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* readString(void* readData)
{
	BinarySerializerData* data = (BinarySerializerData*)readData;

	const char* string = (const char*)(data->dataStart + data->readOffset);
	data->readOffset += strlen(string) + 1;

	if (data->readOffset > data->writeOffset)
	{
		// \todo proper logging macro
		printf("Reading past end of writeData (readOffset %d writeOffset %d)\n", data->readOffset, data->writeOffset);
	}

	return string; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int readInt(void* readData)
{
	BinarySerializerData* data = (BinarySerializerData*)readData;

	uint8_t* intData = (uint8_t*)(data->dataStart + data->readOffset);

	int v = (intData[0] << 24) | (intData[1] << 16) | (intData[2] << 8) | intData[3];

	data->readOffset += 4;

	if (data->readOffset > data->writeOffset)
	{
		// \todo proper logging macro
		printf("Reading past end of writeData (readOffset %d writeOffset %d)\n", data->readOffset, data->writeOffset);
	}

	return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned short readU16(void* readData)
{
	BinarySerializerData* data = (BinarySerializerData*)readData;

	uint8_t* intData = (uint8_t*)(data->dataStart + data->readOffset);

	uint16_t v = (uint16_t)(intData[0] << 8) | (uint16_t)intData[1];

	data->readOffset += 2;

	if (data->readOffset > data->writeOffset)
	{
		// \todo proper logging macro
		printf("Reading past end of writeData (readOffset %d writeOffset %d)\n", data->readOffset, data->writeOffset);
	}

	return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t readU8(void* readData)
{
	BinarySerializerData* data = (BinarySerializerData*)readData;

	uint8_t* intData = (uint8_t*)(data->dataStart + data->readOffset);
	uint8_t v = intData[0];

	data->readOffset += 1;

	if (data->readOffset > data->writeOffset)
	{
		// \todo proper logging macro
		printf("Reading past end of writeData (readOffset %d writeOffset %d)\n", data->readOffset, data->writeOffset);
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
		// \todo proper logging macro
		printf("Skiping past end of writeData (readOffset %d writeOffset %d)\n", data->readOffset, data->writeOffset);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_saveReadOffset(struct PDReader* reader)
{
	BinarySerializerData* data = (BinarySerializerData*)reader->data;
	data->readSaveOffset = data->readOffset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_gotoNextOffset(struct PDReader* reader, int offset)
{
	BinarySerializerData* data = (BinarySerializerData*)reader->data;
	data->readOffset = data->readSaveOffset + offset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_initWriter(struct PDWriter* writer)
{
	BinarySerializerData* data = (BinarySerializerData*)malloc(sizeof(BinarySerializerData));
	memset(data, 0, sizeof(BinarySerializerData));

	data->maxAllocSize = 256 * 1024;
	data->dataStart = (uint8_t*)malloc((size_t)data->maxAllocSize);

	writer->data = data;
	//writer->writeInt = writeInt;
	writer->writeU8 = writeU8;
	writer->writeU16 = writeU16;
	writer->writeString = writeString;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerialize_beginEvent(struct PDWriter* writer, PDEventType eventType, int eventId)
{
	BinarySerializerData* data = (BinarySerializerData*)writer->writeData;

	// event id should be 0 at this point (endEvent should have been called)

	if (data->writeEventStarted)
	{
		// \todo proper logging macro
		printf("Event is still in progress yet beginEvent has been called. Make sure to call endEvent first\n");
	}

	// store start of the event (where we are going to write the size of the event
	data->beginEventOffset = data->writeOffset;
	data->writeEventStarted = 1;

	PDWRITE_INT(writer, 0);		// size (will be filled in later
	PDWRITE_INT(writer, (int)eventType);		
	PDWRITE_INT(writer, eventId);		
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerialize_endEvent(struct PDWriter* writer)
{
	BinarySerializerData* data = (BinarySerializerData*)writer->writeData;

	if (!data->writeEventStarted)
	{
		// \todo proper logging macro
		printf("No event has been started. Make sure to call beginEvent first\n");
	}

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

void BinarySerializer_initReader(struct PDReader* reader, void* data)
{
	reader->data = data;
	reader->readInt = readInt;
	reader->readU8 = readU8;
	reader->readU16 = readU16;
	reader->readString = readString;
	reader->bytesLeft = bytesLeft;
	reader->skipBytes = skipBytes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerializer_initReaderFromStream(struct PDReader* reader, void* inputData, int size) 
{
	BinarySerializerData* data = (BinarySerializerData*)malloc(sizeof(BinarySerializerData));
	memset(data, 0, sizeof(BinarySerializerData));

	data->dataStart = inputData;
	data->maxAllocSize = size;
	data->writeOffset = size;

	BinarySerializer_initReader(reader, data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int BinarySerializer_writeSize(struct PDWriter* writer)
{
	BinarySerializerData* data = (BinarySerializerData*)writer->writeData;
	return data->writeOffset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* BinarySerializer_getStartData(struct PDWriter* writer)
{
	BinarySerializerData* data = (BinarySerializerData*)writer->writeData;
	return data->dataStart;
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

*/

