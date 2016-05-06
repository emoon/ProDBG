#include "capstone/capstone.h"
#include "api/include/pd_capstone.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDCapstoneFuncs s_funcs[] = {
	cs_version,
	cs_support,

	cs_open,
	cs_close,

	cs_option,
	cs_errno,

	cs_disasm,
	cs_free,

	cs_disasm_iter,

	cs_reg_name,
	cs_insn_name,
	cs_group_name,

	cs_reg_read,
	cs_op_count,

	cs_op_index,

	cs_regs_access
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* get_capstone_service_1() {
	return s_funcs;
}

