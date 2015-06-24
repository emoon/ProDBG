#ifndef _PRODBGAPI_IO_H_
#define _PRODBGAPI_IO_H_

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSaveState 
{
	void* privData;

	void (*writeInt)(void* privData, const int64_t v);
	void (*writeDouble)(void* privData, const double v);
	void (*writeString)(void* privData, const char* str);

} PDSaveState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDLoadStatus
{
	PDLoadStatus_ok,
	PDLoadStatus_fail,
	PDLoadStatus_converted,
	PDLoadStatus_truncated,
	PDLoadStatus_outOfData,
} PDLoadStatus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDLoadState 
{
	void* privData;

	PDLoadStatus (*readInt)(void* privData, int64_t* dest);
	PDLoadStatus (*readDouble)(void* privData, double* dest);
	PDLoadStatus (*readString)(void* privData, char*, int maxLen);

} PDLoadState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDIO_writeInt(funcs, v) funcs->writeInt(funcs->privData, v)
#define PDIO_writeDouble(funcs, v) funcs->writeDouble(funcs->privData, v)
#define PDIO_writeString(funcs, v) funcs->writeString(funcs->privData, v)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDIO_readInt(funcs, dest) funcs->readInt(funcs->privData, dest)
#define PDIO_readReal(funcs, data) funcs->readDouble(funcs->privData, dest)
#define PDIO_readString(funcs, str, maxLen) funcs->readString(funcs->privData, str, maxLen)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

