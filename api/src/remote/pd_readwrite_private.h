#ifndef PDREADWRITE_PRIVATE_H_
#define PDREADWRITE_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct PDReader;
struct PDWriter;

// This is a private header. Not to to be used by plugins directly

void pd_binary_reader_init(struct PDReader* reader);
void pd_binary_reader_init_stream(struct PDReader* reader, unsigned char* data, unsigned int size);
void pd_binary_reader_reset(struct PDReader* reader);
void pd_binary_reader_destroy(struct PDReader* reader);

void pd_binary_writer_init(struct PDWriter* writer);
void pd_binary_writer_destroy(struct PDWriter* writer);
void pd_binary_writer_finalize(struct PDWriter* writer);
void pd_binary_writer_reset(struct PDWriter* writer);

unsigned int pd_binary_writer_get_size(struct PDWriter* writer);
unsigned char* pd_binary_writer_get_data(struct PDWriter* writer);

#ifdef __cplusplus
}
#endif

#endif

