#ifndef PDREADWRITE_PRIVATE_H_
#define PDREADWRITE_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct PDReader;
struct PDWriter;

// This is a private header. Not to to be used by plugins directly

void PDBinaryReader_init(struct PDReader* reader);
void PDBinaryReader_initStream(struct PDReader* reader, unsigned char* data, unsigned int size);
void PDBinaryReader_reset(struct PDReader* reader);
void PDBinaryReader_destroy(struct PDReader* reader);

void PDBinaryWriter_init(struct PDWriter* writer);
void PDBinaryWriter_destroy(struct PDWriter* writer);
void PDBinaryWriter_finalize(struct PDWriter* writer);
void PDBinaryWriter_reset(struct PDWriter* writer);

unsigned int PDBinaryWriter_getSize(struct PDWriter* writer);
unsigned char* PDBinaryWriter_getData(struct PDWriter* writer);

#ifdef __cplusplus
}
#endif

#endif

