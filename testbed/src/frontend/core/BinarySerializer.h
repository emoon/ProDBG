#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDSerializeRead;
struct PDSerializeWrite;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

void BinarySerialize_beginEvent(struct PDSerializeWrite* writer);
void BinarySerialize_endEvent(struct PDSerializeWrite* writer);

void BinarySerializer_initWriter(struct PDSerializeWrite* writer);
void BinarySerializer_initReader(struct PDSerializeRead* reader, void* data);
void BinarySerializer_initReaderFromWriter(struct PDSerializeWrite* reader, struct PDSerializeWrite* writer);

void BinarySerializer_saveReadOffset(struct PDSerializeRead* reader);
void BinarySerializer_gotoNextOffset(struct PDSerializeRead* reader, int offset);

void BinarySerializer_destroyData(void* data);

}

