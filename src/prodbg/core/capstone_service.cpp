#include "api/include/pd_capstone.h"
#include "capstone_service.h"
#include "service.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDCapstoneFuncs g_capstoneService = 
{
	cs_version,

	cs_support,

	cs_open,
	cs_close,

	cs_option,
	cs_errno,

	cs_disasm,
	cs_disasm_iter,

	cs_reg_name,
	cs_insn_name,
	cs_group_name,

	cs_reg_read,
	cs_op_count,

	cs_op_index,

	cs_regs_access,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CapstoneService_init()
{
	Service_register(&g_capstoneService, PDCAPSTONEFUNCS_GLOBAL);
}


