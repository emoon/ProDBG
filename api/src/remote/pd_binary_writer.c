#include <pd_readwrite.h>
#include "pd_readwrite_private.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WriterData {
    uint8_t*     dataStart;
    uint8_t*     data;
    uint8_t*     eventOffset;
    uint8_t*     arrayOffset;
    uint8_t*     entryOffset;
    unsigned int writingEvent;
    unsigned int writingArray;
    unsigned int writingArrayEntry;
    unsigned int entryCount;
    unsigned int maxSize;
    unsigned int size;
} WriterData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define inline __inline
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint8_t* writeIdSize(uint8_t* data, const char* id, uint8_t type, uint16_t typeSize) {
    size_t len = strlen(id);
    uint16_t totalSize = ((uint16_t)len) + typeSize + 4;    // + 4 for: type (1 byte) size (2 bytes) null term (1 byte)

    data[0] = type;
    data[1] = (totalSize >> 8) & 0xff;
    data[2] = (totalSize >> 0) & 0xff;

    memcpy(data + 3, id, len + 1);

    return data + len + 4;    // size (2) bytes, 1 byte (type), 1 byte (null terminator)
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_s8(struct PDWriter* writer, const char* id, int8_t v) {
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_S8, sizeof(int8_t));
    *wData->data++ = v;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_u8(struct PDWriter* writer, const char* id, uint8_t v) {
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_U8, sizeof(uint8_t));
    *wData->data++ = v;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_s16(struct PDWriter* writer, const char* id, int16_t v) {
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_S16, sizeof(int16_t));

    wData->data[0] = (v >> 8) & 0xff;
    wData->data[1] = (v >> 0) & 0xff;
    wData->data += 2;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_u16(struct PDWriter* writer, const char* id, uint16_t v) {
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_U16, sizeof(uint16_t));

    wData->data[0] = (v >> 8) & 0xff;
    wData->data[1] = (v >> 0) & 0xff;
    wData->data += 2;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_s32(struct PDWriter* writer, const char* id, int32_t v) {
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_S32, sizeof(int32_t));

    wData->data[0] = (v >> 24) & 0xff;
    wData->data[1] = (v >> 16) & 0xff;
    wData->data[2] = (v >> 8) & 0xff;
    wData->data[3] = (v >> 0) & 0xff;
    wData->data += 4;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_u32(struct PDWriter* writer, const char* id, uint32_t v) {
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_U32, sizeof(uint32_t));

    wData->data[0] = (v >> 24) & 0xff;
    wData->data[1] = (v >> 16) & 0xff;
    wData->data[2] = (v >> 8) & 0xff;
    wData->data[3] = (v >> 0) & 0xff;
    wData->data += 4;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_s64(struct PDWriter* writer, const char* id, int64_t v) {
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_S64, sizeof(int64_t));

    wData->data[0] = (v >> 56) & 0xff;
    wData->data[1] = (v >> 48) & 0xff;
    wData->data[2] = (v >> 40) & 0xff;
    wData->data[3] = (v >> 32) & 0xff;
    wData->data[4] = (v >> 24) & 0xff;
    wData->data[5] = (v >> 16) & 0xff;
    wData->data[6] = (v >> 8) & 0xff;
    wData->data[7] = (v >> 0) & 0xff;
    wData->data += 8;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_u64(struct PDWriter* writer, const char* id, uint64_t v) {
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_U64, sizeof(uint64_t));

    wData->data[0] = (v >> 56) & 0xff;
    wData->data[1] = (v >> 48) & 0xff;
    wData->data[2] = (v >> 40) & 0xff;
    wData->data[3] = (v >> 32) & 0xff;
    wData->data[4] = (v >> 24) & 0xff;
    wData->data[5] = (v >> 16) & 0xff;
    wData->data[6] = (v >> 8) & 0xff;
    wData->data[7] = (v >> 0) & 0xff;
    wData->data += 8;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

union Convert {
    double dv;
    float fv;
    uint32_t u32;
    uint64_t u64;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_float(struct PDWriter* writer, const char* id, float v) {
    union Convert c;
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_Float, sizeof(uint32_t));

    c.fv = v;

    wData->data[0] = (c.u32 >> 24) & 0xff;
    wData->data[1] = (c.u32 >> 16) & 0xff;
    wData->data[2] = (c.u32 >> 8) & 0xff;
    wData->data[3] = (c.u32 >> 0) & 0xff;
    wData->data += 4;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_double(struct PDWriter* writer, const char* id, double v) {
    union Convert c;
    WriterData* wData = (WriterData*)writer->data;
    wData->data = writeIdSize(wData->data, id, PDReadType_Double, sizeof(uint64_t));

    c.dv = v;

    wData->data[0] = (c.u64 >> 56) & 0xff;
    wData->data[1] = (c.u64 >> 48) & 0xff;
    wData->data[2] = (c.u64 >> 40) & 0xff;
    wData->data[3] = (c.u64 >> 32) & 0xff;
    wData->data[4] = (c.u64 >> 24) & 0xff;
    wData->data[5] = (c.u64 >> 16) & 0xff;
    wData->data[6] = (c.u64 >> 8) & 0xff;
    wData->data[7] = (c.u64 >> 0) & 0xff;
    wData->data += 8;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_string(struct PDWriter* writer, const char* id, const char* v) {
    size_t len;
    WriterData* wData = (WriterData*)writer->data;

    len = strlen(v) + 1;

    wData->data = writeIdSize(wData->data, id, PDReadType_String, (uint16_t)len);
    memcpy(wData->data, v, len);

    wData->data += len;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_data(struct PDWriter* writer, const char* id, void* data, unsigned int len) {
    WriterData* wData = (WriterData*)writer->data;
    size_t idLen = strlen(id);

    // for data we special case a bit with having the size in 32-bit instead to support > 64k size

    uint32_t totalSize = ((uint16_t)idLen) + 4 + 1 + len + 1; // size (4) + type (1) + id_len (+1) null teminator

    wData->data[0] = PDReadType_Data;
    wData->data[1] = (totalSize >> 24) & 0xff;
    wData->data[2] = (totalSize >> 16) & 0xff;
    wData->data[3] = (totalSize >> 8) & 0xff;
    wData->data[4] = (totalSize >> 0) & 0xff;

    memcpy(wData->data + 5, id, idLen + 1);
    memcpy(wData->data + 5 + idLen + 1, data, len);

    wData->data += totalSize;

    if (wData->writingArrayEntry) {
        wData->entryCount++;
    }

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_event_begin(struct PDWriter* writer, uint16_t event) {
    WriterData* wData = (WriterData*)writer->data;
    wData->eventOffset = wData->data + 3;

    if (wData->writingEvent) {
        // \todo proper logging here
        printf("Unable to write eventBegin as no writeEndEvent has been called for previous event\n");
        return PDWriteStatus_Fail;
    }

    wData->data[0] = PDReadType_Event;
    wData->data[1] = (event >> 8) & 0xff;
    wData->data[2] = (event >> 0) & 0xff;
    wData->writingEvent = 1;

    // we will store the size here (at writeEndEvent) so skip 4 bytes a head
    wData->data += 7;

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_event_end(struct PDWriter* writer) {
    uint32_t size;
    WriterData* wData = (WriterData*)writer->data;

    if (!wData->writingEvent) {
        // \todo proper logging here
        printf("Unable to write endEvent as no evenhBegin has been called before this call\n");
        return PDWriteStatus_Fail;
    }

    // + 3 to include the meta data at the begining with the size
    size = (uint32_t)(uintptr_t)(wData->data - wData->eventOffset) + 3;
    wData->eventOffset[0] = (size >> 24) & 0xff;
    wData->eventOffset[1] = (size >> 16) & 0xff;
    wData->eventOffset[2] = (size >> 8) & 0xff;
    wData->eventOffset[3] = (size >> 0) & 0xff;
    wData->writingEvent = 0;

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_header_array_begin(struct PDWriter* writer, const char** ids) {
    (void)writer;
    (void)ids;
    return PDWriteStatus_Fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_header_array_end(struct PDWriter* writer) {
    (void)writer;
    return PDWriteStatus_Fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_array_entry_begin(struct PDWriter* writer) {
    WriterData* wData = (WriterData*)writer->data;
    wData->entryOffset = wData->data + 1;

    if (wData->writingArrayEntry) {
        // \todo proper logging here
        printf("Unable to write arrayEntryBegin as no endArrayEntry has been called for previous entry.\n");
        return PDWriteStatus_Fail;
    }

    wData->data[0] = PDReadType_ArrayEntry;
    wData->writingArrayEntry = 1;
    wData->entryCount = 0;

    // we will store the size and entryCount here (at writeEndEvent) so skip 4 bytes a head
    wData->data += 7;

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_array_entry_end(struct PDWriter* writer) {
    uint32_t size;
    WriterData* wData = (WriterData*)writer->data;

    if (!wData->writingArrayEntry) {
        // \todo proper logging here
        printf("Unable to write arrayEntryEnd as no arrayEntryBegin has been called before this call\n");
        return PDWriteStatus_Fail;
    }

    // + 1 to include the meta data at the begining with the size
    size = (uint32_t)(uintptr_t)(wData->data - wData->entryOffset) + 1;
    wData->entryOffset[0] = (size >> 24) & 0xff;
    wData->entryOffset[1] = (size >> 16) & 0xff;
    wData->entryOffset[2] = (size >> 8) & 0xff;
    wData->entryOffset[3] = (size >> 0) & 0xff;
    wData->entryOffset[4] = (wData->entryCount >> 8) & 0xff;
    wData->entryOffset[5] = (wData->entryCount >> 0) & 0xff;

    wData->writingArrayEntry = 0;

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_array_begin(struct PDWriter* writer, const char* name) {
    WriterData* wData = (WriterData*)writer->data;
    int len = (int)strlen(name) + 1;
    wData->arrayOffset = wData->data + 1;

    if (wData->writingArray) {
        // \todo proper logging here
        printf("Unable to write arrayBegin as no endArray has been called for previous array.\n");
        return PDWriteStatus_Fail;
    }

    wData->data[0] = PDReadType_Array;
    memcpy(wData->data + 5, name, len);
    wData->writingArray = 1;

    // we will store the size here (at writArrayEnd) so skip 4 bytes a head
    wData->data += len + 5;

    return PDWriteStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDWriteStatus write_array_end(struct PDWriter* writer) {
    uint32_t size;
    WriterData* wData = (WriterData*)writer->data;

    if (!wData->writingArray) {
        // \todo proper logging here
        printf("Unable to write arrayEnd as no arrayBegin has been called before this call\n");
        return PDWriteStatus_Fail;
    }

    // write an empty arrayEntry to indicate there are no more entries in the array

    write_array_entry_begin(writer);
    write_array_entry_end(writer);

    // + 1 to include the meta data at the begining with the size
    size = (uint32_t)(uintptr_t)(wData->data - wData->arrayOffset) + 1;
    wData->arrayOffset[0] = (size >> 24) & 0xff;
    wData->arrayOffset[1] = (size >> 16) & 0xff;
    wData->arrayOffset[2] = (size >> 8) & 0xff;
    wData->arrayOffset[3] = (size >> 0) & 0xff;
    wData->writingArray = 0;

    return PDWriteStatus_ok;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryWriter_init(PDWriter* writer) {
    WriterData* data;

    writer->write_event_begin = write_event_begin;
    writer->write_event_end = write_event_end;
    writer->write_header_array_begin = write_header_array_begin;
    writer->write_header_array_end = write_header_array_end;
    writer->write_array_begin = write_array_begin;
    writer->write_array_end = write_array_end;
    writer->write_array_entry_begin = write_array_entry_begin;
    writer->write_array_entry_end = write_array_entry_end;
    writer->write_s8 = write_s8;
    writer->write_u8 = write_u8;
    writer->write_s16 = write_s16;
    writer->write_u16 = write_u16;
    writer->write_s32 = write_s32;
    writer->write_u32 = write_u32;
    writer->write_s64 = write_s64;
    writer->write_u64 = write_u64;
    writer->write_float = write_float;
    writer->write_double = write_double;
    writer->write_string = write_string;
    writer->write_data = write_data;

    //printf("PDBinaryWriter_init\n");

    writer->data = malloc(sizeof(WriterData));
    memset(writer->data, 0, sizeof(WriterData));

    data = (WriterData*)writer->data;

    // \todo: Make this tweakble/custom allocator 1 meg should be enough most of the time

    data->data = data->dataStart = malloc(1024 * 1024);
    // reserve 4 bytes at the start (to be used for size and 2 flags at the top)
    data->data += 4;
    data->maxSize = 1024 * 1024;

    //printf("data-start %p\n", data->dataStart);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char* PDBinaryWriter_getData(PDWriter* writer) {
    WriterData* data = (WriterData*)writer->data;
    return data->dataStart;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write size at the very start of the data

void PDBinaryWriter_finalize(PDWriter* writer) {
    WriterData* data = (WriterData*)writer->data;
    uint8_t* wData = data->dataStart;
    uint32_t v = PDBinaryWriter_getSize(writer) + 4;

    wData[0] = (v >> 24) & 0xff;
    wData[1] = (v >> 16) & 0xff;
    wData[2] = (v >> 8) & 0xff;
    wData[3] = (v >> 0) & 0xff;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int PDBinaryWriter_getSize(PDWriter* writer) {
    WriterData* data = (WriterData*)writer->data;
    return (int)(uintptr_t)(data->data - (data->dataStart + 4));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryWriter_reset(PDWriter* writer) {
    WriterData* data = (WriterData*)writer->data;
    void* tempData = data->dataStart;
    memset(data, 0, sizeof(WriterData));
    data->data = data->dataStart = tempData;
    data->data += 4;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryWriter_destroy(PDWriter* writer) {
    //WriterData* data = (WriterData*)writer->data;
    // TODO: Skip this free due to bad implementation of reader/writer in fake6502, fix!
    //free(data->dataStart);
    free(writer->data);
    //data->dataStart = 0;
    writer->data = 0;
}

