#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "ui/ui_dock.h"
#include "core/math.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void create_docking(void**)
{
	Rect rect = { 0, 0, 0, 0 }; 
	UIDockingGrid* grid = UIDock_createGrid(rect);
	assert_non_null(grid);
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

