#ifndef PD_CAPSTONE_SERVICE_
#define PD_CAPSTONE_SERVICE_

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-sign-overflow"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wduplicate-enum"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#endif

#include "capstone/capstone.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDCAPSTONEFUNCS_GLOBAL "Capstone Service 1"

typedef struct PDCapstoneFuncs
{
	// All of these functions matches the capstone API doc. Refer to that one to look on how to use this API

	unsigned int (*version)(int* major, int* minor);
	bool (*support)(int query);

	cs_err (*open)(cs_arch arch, cs_mode mode, csh* handle);
	cs_err (*close)(csh* handle);

	cs_err (*option)(csh handle, cs_opt_type type, size_t value);
	cs_err (*err)(csh handle);

	size_t (*disasm)(csh handle, const uint8_t* code, size_t code_size, uint64_t address, size_t count, cs_insn** insn);
	bool (*disasmIter)(csh handle, const uint8_t** code, size_t* size, uint64_t* address, cs_insn* insn);

	const char* (*regName)(csh handle, unsigned int regId);
	const char* (*insnName)(csh handle, unsigned int insnId);
	const char* (*groupName)(csh handle, unsigned int groupId);

	bool (*regRead)(csh handle, const cs_insn* insn, unsigned int regId);
	int (*opCount)(csh handle, const cs_insn *insn, unsigned int opType);

	int (*opIndex)(csh handle, const cs_insn* insn, unsigned int opType, unsigned int position);

	cs_err (*regsAccess)(csh handle, const cs_insn* insn, cs_regs regsRead, uint8_t* regsReadCount, cs_regs regsWrite, uint8_t* regsWriteCount);

} PDCapstoneFuncs;

#ifdef __cplusplus
}
#endif

#endif

