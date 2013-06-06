#pragma once

#include <ProDBGAPI.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDSerializeRead;
struct PDSerializeWrite;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

void BinarySerialize_beginEvent(struct PDSerializeWrite* writer, PDEventType eventType, int eventId);
void BinarySerialize_endEvent(struct PDSerializeWrite* writer);

void BinarySerializer_initWriter(struct PDSerializeWrite* writer);
void BinarySerializer_initReader(struct PDSerializeRead* reader, void* data);
void BinarySerializer_initReaderFromWriter(struct PDSerializeWrite* reader, struct PDSerializeWrite* writer);

void BinarySerializer_saveReadOffset(struct PDSerializeRead* reader);
void BinarySerializer_gotoNextOffset(struct PDSerializeRead* reader, int offset);

int  BinarySerializer_writeSize(struct PDSerializeWrite* writer);
void BinarySerializer_destroyData(void* data);

}

