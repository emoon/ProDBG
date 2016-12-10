#ifndef PD_ID_SERVICE_
#define PD_ID_SERVICE_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDIDFUNCS_GLOBAL "Id Service 1"

typedef struct PDIdFuncs {
	uint16_t (*register_id)(const char* id);
} PDIdFuncs;

#ifdef __cplusplus
}
#endif

#endif

