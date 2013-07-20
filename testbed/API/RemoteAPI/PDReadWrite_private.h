#ifndef _PDREADWRITE_PRIVATE_H_
#define _PDREADWRITE_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

// This is a private header. Not to to be used by plugins directly

void PDBinaryReader_init(PDReader* reader);
void PDBinaryReader_initStream(PDReader* reader, void* data, unsigned int size);
void PDBinaryReader_reset(PDReader* reader);
void PDBinaryReader_destroy(PDReader* reader);

void PDBinaryWriter_init(PDWriter* writer);
void PDBinaryWriter_destroy(PDWriter* writer);

unsigned int PDBinaryWriter_getSize(PDWriter* writer);
void* PDBinaryWriter_getData(PDWriter* writer);

#ifdef __cplusplus
}
#endif

#endif

