#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "ui/ui_dock.h"
#include "ui/ui_dock_private.h"
#include "core/math.h"
#include "api/plugin_instance.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void create_docking(void**)
{
	Rect rect = { 0, 0, 0, 0 }; 
	UIDockingGrid* grid = UIDock_createGrid(&rect);
	assert_non_null(grid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void validateRect(Rect r0, Rect r1)
{
	assert_int_equal(r0.x, r1.x);
	assert_int_equal(r0.y, r1.x);
	assert_int_equal(r0.width, r1.width);
	assert_int_equal(r0.height, r1.height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_left_attach(void**)
{
	Rect rect = { 0, 0, 1000, 1000 }; 
	UIDockingGrid* grid = UIDock_createGrid(&rect);

	ViewPluginInstance view0 = {};
	ViewPluginInstance view1 = {};

	// Validate grid

	validateRect(grid->rect, rect);
	assert_int_equal(grid->sizers.size(), 0);
	assert_int_equal(grid->docks.size(), 0);

	UIDock* dock = UIDock_addView(grid, &view0);

	assert_null(dock->topSizer);
	assert_null(dock->bottomSizer);
	assert_null(dock->rightSizer);
	assert_null(dock->leftSizer);
	assert_true(dock->view == &view0);

	assert_int_equal((int)grid->sizers.size(), 0);
	assert_int_equal((int)grid->docks.size(), 1);

	UIDock_dockLeft(grid, dock, &view1);

	assert_int_equal((int)grid->sizers.size(), 1);
	assert_int_equal((int)grid->docks.size(), 2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main()
{
    const UnitTest tests[] =
    {
        unit_test(create_docking),
        unit_test(test_left_attach),
    };

    return run_tests(tests);
}

