#ifndef _PRODBGAPI_IO_H_
#define _PRODBGAPI_IO_H_

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSaveState 
{
	void* privData;
	void (*write)(void* privData, void* data, int len);

	void (*writeI1)(void* privData, const int8_t* data, int n);
	void (*writeI2)(void* privData, const int16_t* data, int n);
	void (*writeI4)(void* privData, const int32_t* data, int n);
	void (*writeI8)(void* privData, const int64_t* data, int n);

	void (*writeU1)(void* privData, const uint8_t* data, int n);
	void (*writeU2)(void* privData, const uint16_t* data, int n);
	void (*writeU4)(void* privData, const uint32_t* data, int n);
	void (*writeU8)(void* privData, const uint64_t* data, int n);

	void (*writeFloat)(void* privdata, const float* data, int n);
	void (*writeDouble)(void* privData, const double* data, int n);

	void (*writeString)(void* privData, const char* str);
} PDSaveState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDIO_write(pdWriteFuncs, data, len) pdWriteFuncs->write(pdWriteFuncs->privData, data, len)
#define PDIO_writeI1(pdWriteFuncs, data, len) pdWriteFuncs->writeI1(pdWriteFuncs->privData, data, len)
#define PDIO_writeI2(pdWriteFuncs, data, len) pdWriteFuncs->writeI2(pdWriteFuncs->privData, data, len)
#define PDIO_writeI4(pdWriteFuncs, data, len) pdWriteFuncs->writeI4(pdWriteFuncs->privData, data, len)
#define PDIO_writeI8(pdWriteFuncs, data, len) pdWriteFuncs->writeI8(pdWriteFuncs->privData, data, len)

#define PDUO_writeU1(pdWriteFuncs, data, len) pdWriteFuncs->writeU1(pdWriteFuncs->privData, data, len)
#define PDUO_writeU2(pdWriteFuncs, data, len) pdWriteFuncs->writeU2(pdWriteFuncs->privData, data, len)
#define PDUO_writeU4(pdWriteFuncs, data, len) pdWriteFuncs->writeU4(pdWriteFuncs->privData, data, len)
#define PDUO_writeU8(pdWriteFuncs, data, len) pdWriteFuncs->writeU8(pdWriteFuncs->privData, data, len)

#define PDUO_writeFloat(pdWriteFuncs, data, len) pdWriteFuncs->writeFloat(pdWriteFuncs->privData, data, len)
#define PDUO_writeDouble(pdWriteFuncs, data, len) pdWriteFuncs->writeDouble(pdWriteFuncs->privData, data, len)

#define PDUO_writeString(pdWriteFuncs, str) pdWriteFuncs->writeString(pdWriteFuncs->privData, str)

#endif

