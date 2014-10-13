#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "core/core.h"
#include "session/session.h"
#include "api/plugin_instance.h"
#include "ui/plugin.h"
#include "ui/ui_layout.h"
#include <pd_ui.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static LayoutItem s_layoutItems[] = 
{
	{ "disassembly.so", "disassembly0", 0.1f, 0.1f, 0.2f, 0.2f }, 
	{ "disassembly.so", "disassembly1", 0.2f, 0.2f, 0.3f, 0.4f }, 
	{ "locals.so", "locals", 0.4f, 0.4f, 0.5f, 0.5f }, 
	{ "registers.so", "registers", 0.7f, 0.7f, 0.1f, 0.1f }, 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* s_basePaths[] = { "pluginPath/temp", "customPath/plugins" }; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static UILayout s_layout = 
{
	s_basePaths,
	s_layoutItems,
	sizeof_array(s_basePaths),
	sizeof_array(s_layoutItems),
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_layout_save(void** state)
{
	(void)state;
	assert_true(UILayout_saveLayout(&s_layout, "t2-output/temp_layout.yaml"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_layout_load(void** state)
{
	UILayout layout;
	(void)state;

	assert_true(UILayout_loadLayout(&layout, "t2-output/temp_layout.yaml"));

	assert_int_equal(layout.layoutItemCount, s_layout.layoutItemCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    const UnitTest tests[] =
    {
        unit_test(ui_layout_save),
		unit_test(ui_layout_load),
    };

    return run_tests(tests);
}

