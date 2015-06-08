#ifndef _PRODBGAPI_IO_H_
#define _PRODBGAPI_IO_H_

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSaveState 
{
	void* privData;

	void (*writeInt)(void* privData, const int64_t v);
	void (*writeIntArray)(void* privData, const int64_t* data, int n);

	void (*writeReal)(void* privData, const double v);
	void (*writeRealArray)(void* privData, const double* data, int n);

	void (*writeString)(void* privData, const char* str);

} PDSaveState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDLoadState 
{
	void* privData;

	int (*readInt)(void* privData);
	int* (*readIntArray)(void* privData, int64_t* data, int n);

	double (*readReal)(void* privData);
	double* (*readRealArray)(void* privData, double* data, int n);

	char* (*readString)(void* privData, char* str, int len);

} PDLoadState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDIO_writeInt(funcs, v) funcs->writeInt(funcs->privData, v)
#define PDIO_writeIntArray(funcs, v, len) funcs->writeIntArray(pdWriteFuncs->privData, v, len)

#define PDIO_writeReal(funcs, v) funcs->writeIReal(funcs->privData, v)
#define PDIO_writeRealArray(funcs, v, len) funcs->writeRealArray(funcs->privData, v, len)

#define PDIO_writeString(funcs, v) funcs->writeString(funcs->privData, v)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDIO_readInt(funcs, data) funcs->readI1(funcs->privData)
#define PDIO_readIntArray(funcs, data, dest, len) funcs->readI1(funcs->privData, dest, len)

#define PDIO_readReal(funcs, data) pdReadFuncs->readReal(funcs->privData)
#define PDIO_readRealArray(funcs, data, dest, len) funcs->readRealArray(funcs->privData, dest, len)

#define PDIO_readString(pdReadFuncs, str) pdReadFuncs->readString(pdReadFuncs->privData, str)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

