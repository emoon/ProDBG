#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <math.h>

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

bool cmp_float(float a, float b)
{
	if (fabs(a - b) < 0.0001f)
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_layout_load(void** state)
{
    UILayout layout;
    (void)state;

    assert_true(UILayout_loadLayout(&layout, "t2-output/temp_layout.yaml"));

    assert_int_equal(layout.layoutItemCount, s_layout.layoutItemCount);
    assert_int_equal(layout.basePathCount, s_layout.basePathCount);

    for (int i = 0; i < layout.basePathCount; ++i)
	{
		assert_non_null(layout.pluginBasePaths[i]);
		assert_string_equal(layout.pluginBasePaths[i], s_layout.pluginBasePaths[i]);
	}

	for (int i = 0; i < layout.layoutItemCount; ++i)
	{
		const LayoutItem* a = &layout.layoutItems[i];
		const LayoutItem* b = &s_layout.layoutItems[i];

		assert_non_null(a->pluginFile);
		assert_non_null(a->pluginName);

		assert_string_equal(a->pluginFile, b->pluginFile);
		assert_string_equal(a->pluginName, b->pluginName);

		assert_true(cmp_float(a->x, b->x));
		assert_true(cmp_float(a->y, b->y));
		assert_true(cmp_float(a->width, b->width));
		assert_true(cmp_float(a->height, b->height));
	}
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

