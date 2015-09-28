#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "core/log.h"

#include "api/include/pd_capstone.h"
#include "core/service.h"
#include "core/core.h"
#include "core/capstone_service.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_m68k(void**)
{
	csh handle = 0;
	cs_insn* insn = 0;

	cs_err err;

	#define M68K_CODE "\x4E\x71\x22\x00\x4E\x75"

	PDCapstoneFuncs* csFuncs = (PDCapstoneFuncs*)Service_getService(PDCAPSTONEFUNCS_GLOBAL);

	err = csFuncs->open(CS_ARCH_M68K, (cs_mode)(CS_MODE_BIG_ENDIAN | CS_MODE_M68K_000), &handle);

	assert_int_equal(err, CS_ERR_OK); 

	csFuncs->option(handle, CS_OPT_DETAIL, CS_OPT_ON);

	size_t count = csFuncs->disasm(handle, (const uint8_t*)M68K_CODE, sizeof(M68K_CODE), 0, 0, &insn);

	for (size_t j = 0; j < count; j++) {
		printf("0x%llx\t%s\t%s\n", insn[j].address, insn[j].mnemonic, insn[j].op_str);
		//print_insn_detail(&insn[j]);
	}

	assert_int_equal(count, 4);

	assert_string_equal(insn[0].mnemonic, "nop");
	assert_string_equal(insn[1].mnemonic, "move.l");
	assert_string_equal(insn[2].mnemonic, "rts");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    log_set_level(LOG_NONE);
    Core_init();
	CapstoneService_init();

    const UnitTest tests[] =
    {
		unit_test(test_m68k),
	};

    return run_tests(tests);
}
