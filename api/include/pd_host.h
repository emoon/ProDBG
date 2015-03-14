#ifndef _PD_HOST_H_
#define _PD_HOST_H_

#ifdef __cplusplus
extern "C" {
#endif

// 

#define PDMESSAGEFUNCS_GLOBAL "Info Messages 1"

typedef struct PDMessageFuncs
{
	void (*info)(const char* title, const char* message);
	void (*error)(const char* title, const char* message);
	void (*warning)(const char* title, const char* message);
} PDMessageFuncs;

#ifdef __cplusplus
}
#endif

#endif

