#include <pd_readwrite.h>
#include "pd_readwrite_private.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ReaderData {
    uint8_t* data;
    uint8_t* dataStart;
    uint8_t* dataEnd;
    uint8_t* nextEvent;
} ReaderData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define inline __inline
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

union Convert {
    double dv;
    float fv;
    uint64_t u32;
    uint64_t u64;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int8_t getS8(const uint8_t* ptr) {
    return (int8_t)ptr[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint8_t getU8(const uint8_t* ptr) {
    return ptr[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint16_t getS16(const uint8_t* ptr) {
    int16_t v = (ptr[0] << 8) | ptr[1];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint16_t getU16(const uint8_t* ptr) {
    uint16_t v = (ptr[0] << 8) | ptr[1];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int32_t getS32(const uint8_t* ptr) {
    int32_t v = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int32_t getU32(const uint8_t* ptr) {
    uint32_t v = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int64_t getS64(const uint8_t* ptr) {
    int64_t v = ((uint64_t)ptr[0] << 56) | ((uint64_t)ptr[1] << 48) | ((uint64_t)ptr[2] << 40) | ((uint64_t)ptr[3] << 32) |
                (ptr[4] << 24) | (ptr[5] << 16) | (ptr[6] << 8) | ptr[7];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint64_t getU64(const uint8_t* ptr) {
    uint64_t v = ((uint64_t)ptr[0] << 56) | ((uint64_t)ptr[1] << 48) | ((uint64_t)ptr[2] << 40) | ((uint64_t)ptr[3] << 32) |
                 (ptr[4] << 24) | (ptr[5] << 16) | (ptr[6] << 8) | ptr[7];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline double getDouble(const uint8_t* ptr) {
    union Convert c;
    c.u64 = getU64(ptr);
    return c.dv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline float getFloat(const uint8_t* ptr) {
    union Convert c;
    c.u32 = getU32(ptr);
    return c.fv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint64_t getOffsetUpper(ReaderData* readerData, const uint8_t* data) {
    uint64_t t = ((uintptr_t)data - (uintptr_t)readerData->dataStart);
    return t << 32L;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_get_event(struct PDReader* reader) {
    ReaderData* rData = (ReaderData*)reader->data;
    uint16_t event;
    uint8_t type;
    uint8_t* data;

    if (!rData->data) {
        log_debug("no data");
        return 0;
    }

    if (rData->nextEvent >= rData->dataEnd) {
        log_debug("rData->nextEvent %p >= rData->dataEnd %p\n", rData->nextEvent, rData->dataEnd);
        return 0;
    }

    // if this is not set we expect this to be the first event and just read from data

    if (!rData->nextEvent)
        data = rData->data;
    else
        data = rData->nextEvent;

    // make sure we actually have some data to process

    if (data >= rData->dataEnd) {
        log_debug("data %p >= rData->dataEnd %p\n", data, rData->dataEnd);
        return 0;
    }

    type = *data;

    if (type != PDReadType_Event) {
        log_debug("Unable to read event as type is wrong (expected %d but got %d) all read operations will now fail.\n",
                  PDReadType_Event, type);
        return 0;
    }

    event = getU16(data + 1);
    rData->nextEvent = data + getU32(data + 3);
    rData->data = data + 7; // points to the next of data in the stream

    log_debug("returing with event %d\n", event);

    return event;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* typeTable[] =
{
    "PDReadType_None",
    "PDReadType_S8",
    "PDReadType_U8",
    "PDReadType_S16",
    "PDReadType_U16",
    "PDReadType_S32",
    "PDReadType_U32",
    "PDReadType_S64",
    "PDReadType_U64",
    "PDReadType_Float",
    "PDReadType_Double",
    "PDReadType_EndNumericTypes",
    "PDReadType_String",
    "PDReadType_Data",
    "PDReadType_Event",
    "PDReadType_Array",
    "PDReadType_ArrayEntry",
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t* findIdByRange(const char* id, uint8_t* start, uint8_t* end) {
    while (start < end) {
        uint32_t size;
        uint8_t typeId = getU8(start);

        ///if (typeId <PDReadType_Count)
        //	log_debug("typeId %s\n", typeTable[typeId]);
        //else
        //	log_debug("typeId %d (outside valid range)\n", typeId);

        // data is a special case as it has 32-bit size instead of 64k

        if (typeId == PDReadType_Data || typeId == PDReadType_Array) {
            size = getU32(start + 1);

            if (!strcmp((char*)start + 5, id))
                return start;
        }else {
            size = getU16(start + 1);

            //log_debug("current string - %s searching for - %s\n", (char*)start + 3, id);

            if (!strcmp((char*)start + 3, id))
                return start;
        }

        start += size;
    }

    // not found
    //

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t* findId(struct PDReader* reader, const char* id, PDReaderIterator it) {
    ReaderData* rData = (ReaderData*)reader->data;

    // if iterator is 0 we search the event stream,

    if (it == 0) {
        // if no iterater we will just search the whole event
        return findIdByRange(id, rData->data, rData->nextEvent);
    }else {
        // serach within the event but skip 7 bytes ahead to not read the event itself
        uint32_t dataOffset = it >> 32LL;
        uint32_t size = it & 0xffffffffLL;
        uint8_t* start = rData->dataStart + dataOffset;
        uint8_t* end = start + size;
        return findIdByRange(id, start, end);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define findValue(inType, realType, getFunc) \
    uint8_t type; \
    size_t offset; \
    const uint8_t* dataPtr = findId(reader, id, it); \
    if (!dataPtr) \
        return PDReadStatus_NotFound; \
    type = *dataPtr; \
    offset = getU16(dataPtr + 1) - (sizeof(realType)); \
    if (type == inType) \
    { \
        *res = getFunc(dataPtr + offset); \
        return PDReadStatus_Ok | inType; \
    } \
    if (type < PDReadType_EndNumericTypes) \
    { \
        offset = strlen((const char*)dataPtr + 3) + 4; \
        switch (type) \
        { \
            case PDReadType_S8: \
                *res = (realType)getS8(dataPtr + offset); return PDReadType_S8 | PDReadStatus_Converted; \
            case PDReadType_U8: \
                *res = (realType)getU8(dataPtr + offset); return PDReadType_U8 | PDReadStatus_Converted;  \
            case PDReadType_S16: \
                *res = (realType)getU16(dataPtr + offset); return PDReadType_S16 | PDReadStatus_Converted; \
            case PDReadType_U16: \
                *res = (realType)getU16(dataPtr + offset); return PDReadType_U16 | PDReadStatus_Converted; \
            case PDReadType_S32: \
                *res = (realType)getU32(dataPtr + offset); return PDReadType_S32 | PDReadStatus_Converted; \
            case PDReadType_U32: \
                *res = (realType)getU32(dataPtr + offset); return PDReadType_U32 | PDReadStatus_Converted; \
            case PDReadType_S64: \
                *res = (realType)getU64(dataPtr + offset); return PDReadType_S64 | PDReadStatus_Converted; \
            case PDReadType_U64: \
                *res = (realType)getU64(dataPtr + offset); return PDReadType_U64 | PDReadStatus_Converted; \
            case PDReadType_Float: \
                *res = (realType)getFloat(dataPtr + offset); return PDReadType_Float | PDReadStatus_Converted; \
            case PDReadType_Double: \
                *res = (realType)getDouble(dataPtr + offset); return PDReadType_Float | PDReadStatus_Converted; \
        } \
    } \
    return (PDReadType)type | PDReadStatus_IllegalType

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_s8(struct PDReader* reader, int8_t* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_S8, int8_t, getS8);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_u8(struct PDReader* reader, uint8_t* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_U8, uint8_t, getU8);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_s16(struct PDReader* reader, int16_t* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_S16, int16_t, getS16);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_u16(struct PDReader* reader, uint16_t* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_U16, uint16_t, getU16);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_s32(struct PDReader* reader, int32_t* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_S32, int32_t, getS32);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_u32(struct PDReader* reader, uint32_t* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_U32, uint32_t, getU32);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_s64(struct PDReader* reader, int64_t* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_S64, int64_t, getS64);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_u64(struct PDReader* reader, uint64_t* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_U64, uint64_t, getU64);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_float(struct PDReader* reader, float* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_Float, float, getFloat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_double(struct PDReader* reader, double* res, const char* id, PDReaderIterator it) {
    findValue(PDReadType_Double, double, getDouble);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_string(struct PDReader* reader, const char** res, const char* id, PDReaderIterator it) {
    uint8_t type;
    int len;

    const uint8_t* dataPtr = findId(reader, id, it);
    if (!dataPtr)
        return PDReadStatus_NotFound;

    type = *dataPtr;

    if (type != PDReadType_String)
        return (PDReadType)type | PDReadStatus_IllegalType;

    // find the offset to the string

    len = (int)strlen((const char*)dataPtr + 3);
    *res = (const char*)dataPtr + 3 + len + 1;

    return (PDReadType)type | PDReadStatus_Ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_data(struct PDReader* reader, void** data, uint64_t* size, const char* id, PDReaderIterator it) {
    uint8_t type;
    int idLength;

    uint8_t* dataPtr = findId(reader, id, it);
    if (!dataPtr) {
        printf("%s:%d\n", __FILE__, __LINE__);
        return PDReadStatus_NotFound;
    }

    type = *dataPtr;

    if (type != PDReadType_Data)
        return (PDReadType)type | PDReadStatus_IllegalType;

    idLength = (int)strlen((const char*)dataPtr + 5) + 1;

    // find the offset to the string

    *size = (getU32(dataPtr + 1) - idLength) - 5;   // fix hard-coded values
    *data = (void*)(dataPtr + 5 + idLength);

    return PDReadType_Data | PDReadStatus_Ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t read_find_array(struct PDReader* reader, PDReaderIterator* arrayIt, const char* id, PDReaderIterator it) {
    uint8_t type;
    int idLength;
    ReaderData* rData = (ReaderData*)reader->data;

    const uint8_t* dataPtr = findId(reader, id, it);
    if (!dataPtr) {
        return PDReadStatus_NotFound;
    }

    type = *dataPtr;

    if (type != PDReadType_Array)
        return (PDReadType)type | PDReadStatus_IllegalType;

    idLength = (int)strlen((const char*)dataPtr + 5) + 1;

    // get offset to the array entry

    dataPtr += idLength + 5;

    // find the offset to the string

    *arrayIt = getOffsetUpper(rData, dataPtr);

    return PDReadType_Array | PDReadStatus_Ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t read_next_entry(struct PDReader* reader, PDReaderIterator* arrayIt) {
    uint8_t type;
    int32_t entries;
    uint64_t it = *arrayIt;

    // we use a small trick here: The first time we get here the size will be zero (as set by find/readArray)
    // so when we are supposed to jump to the next entry we jump to the current one which is correct as we
    // want to read out the size to the next one (which we will jump to the next time we get here

    ReaderData* rData = (ReaderData*)reader->data;
    uint32_t offset = it >> 32LL;
    uint32_t size = it & 0xffffffffLL;
    uint8_t* entryStart = rData->dataStart + offset + size;

    if ((type = *entryStart) != PDReadType_ArrayEntry) {
        log_info("No arrayEntry found at %p (found %d) but expected %d\n", entryStart, type, PDReadType_ArrayEntry);
        return -1;
    }

    // get size and number of entries

    size = getU32(entryStart + 1);
    entries = getS16(entryStart + 5);

    // at the first entry this is the same as arrayStar

    *arrayIt = getOffsetUpper(rData, entryStart + 7) | (size - 7);

    return entries;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void read_dump_data(struct PDReader* reader) {
    int eventId;
    ReaderData* rData = (ReaderData*)reader->data;

    while ((eventId = PDRead_get_event(reader)) != 0) {
        log_info("{ = event %d - (start %p end %p)\n", eventId, rData->data, rData->nextEvent);

        while (rData->data < rData->nextEvent) {
            uint8_t type = *(uint8_t*)rData->data;
            uint32_t size = getU16(rData->data + 1);
            const char* idOffset = (const char*)rData->data + 3;

            if (type < PDReadType_Count) {
                if (type == PDReadType_Array) {
                    // need to handle array here, now just grab the correct size and idOffset

                    size = getU32(rData->data + 1);
                    idOffset = (const char*)rData->data + 5;
                }

                log_info("  %s : (%s - %d)\n", idOffset, typeTable[type], size);
            }

            rData->data += size;
        }

        log_info("}\n");
    }

    PDBinaryReader_reset(reader);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryReader_init(PDReader* reader) {
    reader->read_get_event = read_get_event;
    reader->read_next_entry = read_next_entry;
    reader->read_find_s8 = read_find_s8;
    reader->read_find_u8 = read_find_u8;
    reader->read_find_s16 = read_find_s16;
    reader->read_find_u16 = read_find_u16;
    reader->read_find_s32 = read_find_s32;
    reader->read_find_u32 = read_find_u32;
    reader->read_find_s64 = read_find_s64;
    reader->read_find_u64 = read_find_u64;
    reader->read_find_float = read_find_float;
    reader->read_find_double = read_find_double;
    reader->read_find_string = read_find_string;
    reader->read_find_data = read_find_data;
    reader->read_find_array = read_find_array;
    reader->read_dump_data = read_dump_data;

    reader->data = malloc(sizeof(ReaderData));
    memset(reader->data, 0, sizeof(ReaderData));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryReader_initStream(PDReader* reader, uint8_t* data, unsigned int size) {
    ReaderData* readerData = (ReaderData*)reader->data;
    readerData->data = readerData->dataStart = data + 4;    // top 4 bytes for size + 2 bits for info
    readerData->dataEnd = (uint8_t*)data + size;
    readerData->nextEvent = 0;
    pda_log_set_level(LOG_INFO);
    log_debug("InitStream %p - size %d\n", data, size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryReader_reset(PDReader* reader) {
    ReaderData* readerData = (ReaderData*)reader->data;
    readerData->data = readerData->dataStart;
    readerData->nextEvent = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PDBinaryReader_destroy(PDReader* reader) {
    ReaderData* readerData = (ReaderData*)reader->data;
    free(readerData);
}

