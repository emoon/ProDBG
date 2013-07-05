#pragma once

#include <ProDBGAPI.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDSerializeRead;
struct PDSerializeWrite;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinarySerialize_beginEvent(struct PDSerializeWrite* writer, PDEventType eventType, int eventId);
void BinarySerialize_endEvent(struct PDSerializeWrite* writer);

void BinarySerializer_initWriter(struct PDSerializeWrite* writer);
void BinarySerializer_initReader(struct PDSerializeRead* reader, void* data);
void BinarySerializer_initReaderFromStream(struct PDSerializeRead* reader, void* data, int size); 

void BinarySerializer_saveReadOffset(struct PDSerializeRead* reader);
void BinarySerializer_gotoNextOffset(struct PDSerializeRead* reader, int offset);

void BinarySerializer_destroyData(void* data);
int  BinarySerializer_writeSize(struct PDSerializeWrite* writer);

void* BinarySerializer_getStartData(struct PDSerializeWrite* writer);

#ifdef __cplusplus
}
#endif

