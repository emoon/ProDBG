#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "ui/ui_dock.h"
#include "core/math.h"
#include "api/plugin_instance.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void create_docking(void**)
{
	Rect rect = { 0, 0, 0, 0 }; 
	UIDockingGrid* grid = UIDock_createGrid(rect);
	assert_non_null(grid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_left_attach(void**)
{
	Rect rect = { 0, 0, 1000, 1000 }; 
	UIDockingGrid* grid = UIDock_createGrid(rect);

	ViewPluginInstance view0 = {};
	ViewPluginInstance view1 = {};

	UIDock* dock = UIDock_addView(grid, &view0);
	UIDock_dockLeft(grid, dock, &view1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main()
{
    const UnitTest tests[] =
    {
        unit_test(create_docking),
    };

    return run_tests(tests);
}

