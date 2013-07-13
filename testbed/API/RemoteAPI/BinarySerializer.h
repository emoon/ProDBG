#pragma once

#include <ProDBGAPI.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDReader;
struct PDWriter;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerialize_beginEvent(struct PDWriter* writer, PDEventType eventType, int eventId);
void BinarySerialize_endEvent(struct PDWriter* writer);

void BinarySerializer_initWriter(struct PDWriter* writer);
void BinarySerializer_initReader(struct PDReader* reader, void* data);
void BinarySerializer_initReaderFromStream(struct PDReader* reader, void* data, int size); 

void BinarySerializer_saveReadOffset(struct PDReader* reader);
void BinarySerializer_gotoNextOffset(struct PDReader* reader, int offset);

void BinarySerializer_destroyData(void* data);
int  BinarySerializer_writeSize(struct PDWriter* writer);

void* BinarySerializer_getStartData(struct PDWriter* writer);

#ifdef __cplusplus
}
#endif

