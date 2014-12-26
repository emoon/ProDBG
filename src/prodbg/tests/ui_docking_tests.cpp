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
	ViewPluginInstance view2 = {};

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

	// at this point we should have two views/docks split in the middle
	// with one sizer so lets verify that

	assert_int_equal((int)grid->sizers.size(), 1);
	assert_int_equal((int)grid->docks.size(), 2);

	UIDock* dock0 = grid->docks[0];
	UIDock* dock1 = grid->docks[1];
	UIDockSizer* s0 = grid->sizers[0]; 

	assert_null(dock0->topSizer);	
	assert_null(dock0->bottomSizer);	
	assert_null(dock0->leftSizer);	
	assert_true(dock0->rightSizer == s0);	

	assert_null(dock1->topSizer);	
	assert_null(dock1->bottomSizer);	
	assert_null(dock1->rightSizer);	
	assert_true(dock1->leftSizer == s0);	

	assert_true(s0->side0->view = &view0);
	assert_true(s0->side1->view = &view1);

	UIDock_dockLeft(grid, dock1, &view2);

	// at this point we should have tree views looking like this:
	//  ______s0___s1__
	// |       |   |   |
	// |   d0  |d2 |d1 |
	// |       |   |   |
	// -----------------

	assert_int_equal((int)grid->sizers.size(), 2);
	assert_int_equal((int)grid->docks.size(), 3);

	UIDock* dock2 = grid->docks[2];
	UIDockSizer* s1 = grid->sizers[1]; 

	// Make sure sizers are assigned correct

	assert_null(dock0->topSizer);	
	assert_null(dock0->bottomSizer);	
	assert_null(dock0->leftSizer);	
	assert_true(dock0->rightSizer == s0);	

	assert_null(dock2->topSizer);	
	assert_null(dock2->bottomSizer);	
	assert_true(dock2->leftSizer == s0);	
	assert_true(dock2->rightSizer == s1);	

	assert_null(dock1->topSizer);	
	assert_null(dock1->bottomSizer);	
	assert_true(dock1->leftSizer == s1);	
	assert_null(dock1->rightSizer);	

	// Validate the size of views (they should not go above the inital size)
	// with the sizer size taken into account

	int width = 0;

	for (int i = 0; i < 3; ++i)
		width += grid->docks[i]->view->rect.width;

	width += g_sizerSize * 2;

	assert_int_equal(width, rect.width);
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

