#ifndef _PRODBGAPI_IO_H_
#define _PRODBGAPI_IO_H_

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSaveState {
	void* priv_data;
	void (*write_int)(void* priv_data, const int64_t v);
	void (*write_double)(void* priv_data, const double v);
	void (*write_string)(void* priv_data, const char* str);
} PDSaveState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDLoadStatus {
	PDLoadStatus_Ok,
	PDLoadStatus_Fail,
	PDLoadStatus_Converted,
	PDLoadStatus_Truncated,
	PDLoadStatus_OutOfData,
} PDLoadStatus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDLoadState {
	void* priv_data;
	PDLoadStatus (*read_int)(void* priv_data, int64_t* dest);
	PDLoadStatus (*read_double)(void* priv_data, double* dest);
	PDLoadStatus (*read_string)(void* priv_data, char*, int maxLen);
} PDLoadState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDIO_write_int(funcs, v) funcs->write_int(funcs->priv_data, v)
#define PDIO_write_double(funcs, v) funcs->write_double(funcs->priv_data, v)
#define PDIO_write_string(funcs, v) funcs->write_string(funcs->priv_data, v)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDIO_read_int(funcs, dest) funcs->read_int(funcs->priv_data, dest)
#define PDIO_read_real(funcs, data) funcs->read_double(funcs->priv_data, dest)
#define PDIO_read_string(funcs, str, maxLen) funcs->read_string(funcs->priv_data, str, maxLen)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

