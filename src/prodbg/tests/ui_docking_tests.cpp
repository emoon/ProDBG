#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "ui/ui_dock.h"
#include "ui/ui_dock_private.h"
#include "core/math.h"
#include "api/plugin_instance.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void create_docking(void**)
{
    Rect rect = {{{ 0, 0, 800, 200 }}};
    UIDockingGrid* grid = UIDock_createGrid(&rect);

    assert_int_equal(grid->topSizer.rect.x, rect.x);
    assert_int_equal(grid->topSizer.rect.y, rect.y);
    assert_int_equal(grid->topSizer.rect.width, rect.width);
    assert_int_equal(grid->topSizer.rect.height, 0);

    assert_int_equal(grid->bottomSizer.rect.x, rect.x);
    assert_int_equal(grid->bottomSizer.rect.y, rect.height);
    assert_int_equal(grid->bottomSizer.rect.width, rect.width);
    assert_int_equal(grid->bottomSizer.rect.height, 0);

    assert_int_equal(grid->leftSizer.rect.x, rect.x);
    assert_int_equal(grid->leftSizer.rect.y, rect.y);
    assert_int_equal(grid->leftSizer.rect.width, 0);
    assert_int_equal(grid->leftSizer.rect.height, rect.height);

    assert_int_equal(grid->rightSizer.rect.x, rect.width);
    assert_int_equal(grid->rightSizer.rect.y, rect.y);
    assert_int_equal(grid->rightSizer.rect.width, 0);
    assert_int_equal(grid->rightSizer.rect.height, rect.height);

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

void validateSize(Rect r, int x, int y, int w, int h)
{
    assert_int_equal(r.x, x);
    assert_int_equal(r.y, y);
    assert_int_equal(r.width, w);
    assert_int_equal(r.height, h);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_left_attach(void**)
{
    Rect rect = {{{ 0, 0, 1000, 400 }}};

    UIDockingGrid* grid = UIDock_createGrid(&rect);

    ViewPluginInstance view0Inst = {};
    ViewPluginInstance view1Inst = {};
    ViewPluginInstance view2Inst = {};

    ViewPluginInstance* view0 = &view0Inst;
    ViewPluginInstance* view1 = &view1Inst;
    ViewPluginInstance* view2 = &view2Inst;

    // Validate grid

    validateRect(grid->rect, rect);
    assert_int_equal(grid->sizers.size(), 0);
    assert_int_equal(grid->docks.size(), 0);

    UIDock* dock = UIDock_addView(grid, view0);

    //validateSize(dock->view->rect, 0, 0, 1000, 500);

    assert_true(dock->topSizer == &grid->topSizer);
    assert_true(dock->bottomSizer == &grid->bottomSizer);
    assert_true(dock->rightSizer == &grid->rightSizer);
    assert_true(dock->leftSizer == &grid->leftSizer);
    assert_true(dock->view == view0);

    assert_int_equal((int)grid->sizers.size(), 0);
    assert_int_equal((int)grid->docks.size(), 1);

    UIDock_dockLeft(grid, dock, view1);

    // at this point we should have two views/docks split in the middle (vertically)
    // with one sizer so lets verify that

    assert_int_equal((int)grid->sizers.size(), 1);
    assert_int_equal((int)grid->docks.size(), 2);

    UIDock* dock0 = grid->docks[0];
    UIDock* dock1 = grid->docks[1];
    UIDockSizer* s0 = grid->sizers[0];

    assert_int_equal(s0->rect.x, rect.width / 2);
    assert_int_equal(s0->rect.y, 0);
    assert_int_equal(s0->rect.width, 0);
    assert_int_equal(s0->rect.height, rect.height);

    assert_int_equal(dock0->view->rect.x, rect.width / 2);
    assert_int_equal(dock0->view->rect.y, 0);
    assert_int_equal(dock0->view->rect.width, rect.width / 2);
    assert_int_equal(dock0->view->rect.height, rect.height);

    assert_int_equal(dock1->view->rect.x, 0);
    assert_int_equal(dock1->view->rect.y, 0);
    assert_int_equal(dock1->view->rect.width, rect.width / 2);
    assert_int_equal(dock1->view->rect.height, rect.height);

    assert_true(s0->dir == UIDockSizerDir_Vert);

    assert_true(dock0->topSizer == &grid->topSizer);
    assert_true(dock0->bottomSizer == &grid->bottomSizer);
    assert_true(dock0->leftSizer == s0);
    assert_true(dock0->rightSizer == &grid->rightSizer);

    assert_true(dock1->topSizer == &grid->topSizer);
    assert_true(dock1->bottomSizer == &grid->bottomSizer);
    assert_true(dock1->leftSizer == &grid->leftSizer);
    assert_true(dock1->rightSizer == s0);

    // The top and bottom sizers should now be connected to the two views

    assert_int_equal((int)grid->topSizer.cons.size(), 2);
    assert_int_equal((int)grid->bottomSizer.cons.size(), 2);

    UIDock_dockLeft(grid, dock0, view2);

    // at this point we should have tree views looking like this:
    //  ___s0___s1_____
    // |    |   |      |
    // | d1 |d2 |  d0  |
    // |    |   |      |
    // -----------------

    assert_int_equal((int)grid->sizers.size(), 2);
    assert_int_equal((int)grid->docks.size(), 3);

    assert_int_equal((int)grid->topSizer.cons.size(), 3);
    assert_int_equal((int)grid->bottomSizer.cons.size(), 3);

    UIDock* dock2 = grid->docks[2];
    UIDockSizer* s1 = grid->sizers[1];

    assert_int_equal((int)s1->cons.size(), 2);
    assert_true(s1->dir == UIDockSizerDir_Vert);

    // Make sure sizers are assigned correct

    assert_true(dock0->topSizer == &grid->topSizer);
    assert_true(dock0->bottomSizer == &grid->bottomSizer);
    assert_true(dock0->leftSizer == s1);
    assert_true(dock0->rightSizer == &grid->rightSizer);

    assert_true(dock1->topSizer == &grid->topSizer);
    assert_true(dock1->bottomSizer == &grid->bottomSizer);
    assert_true(dock1->leftSizer == &grid->leftSizer);
    assert_true(dock1->rightSizer == s0);

    assert_true(dock2->topSizer == &grid->topSizer);
    assert_true(dock2->bottomSizer == &grid->bottomSizer);
    assert_true(dock2->leftSizer == s0);
    assert_true(dock2->rightSizer == s1);

    // Validate the size of views (they should not go above the inital size)
    // with the sizer size taken into account

    int width = 0;

    for (int i = 0; i < 3; ++i)
        width += (int)grid->docks[(size_t)i]->view->rect.width;

    assert_int_equal(width, rect.width);

    // TODO: currently leaks here, will be fixed once added delete of views
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_misc(void**)
{
    Rect rect = {{{ 0, 0, 1000, 500 }}};

    UIDockingGrid* grid = UIDock_createGrid(&rect);

    ViewPluginInstance view0 = {};
    ViewPluginInstance view1 = {};
    ViewPluginInstance view2 = {};

    UIDock* dock = UIDock_addView(grid, &view0);
    UIDock_dockRight(grid, dock, &view1);
    UIDock_dockBottom(grid, dock, &view2);

    // Expected layout:
    //
    //     _____s0_______
    //    |      |      |
    //    |  d0  |      |
    //    |      |      |
    // s1 |------|  d1  |
    //    |      |      |
    //    |  d2  |      |
    //    |      |      |
    //    ---------------
    //

    assert_int_equal((int)grid->sizers.size(), 2);
    assert_int_equal((int)grid->docks.size(), 3);

    UIDockSizer* s0 = grid->sizers[0];
    UIDockSizer* s1 = grid->sizers[1];

    UIDock* d0 = grid->docks[0];
    UIDock* d1 = grid->docks[1];
    UIDock* d2 = grid->docks[2];

    assert_int_equal(d0->view->rect.x, 0);
    assert_int_equal(d0->view->rect.y, 0);
    assert_int_equal(d0->view->rect.width, rect.width / 2);
    assert_int_equal(d0->view->rect.height, rect.height / 2);

    assert_int_equal(d1->view->rect.x, rect.width / 2);
    assert_int_equal(d1->view->rect.y, 0);
    assert_int_equal(d1->view->rect.width, rect.width / 2);
    assert_int_equal(d1->view->rect.height, rect.height);

    assert_int_equal(d2->view->rect.x, 0);
    assert_int_equal(d2->view->rect.y, rect.height / 2);
    assert_int_equal(d2->view->rect.width, rect.width / 2);
    assert_int_equal(d2->view->rect.height, rect.height / 2);

    assert_true(d0->topSizer == &grid->topSizer);
    assert_true(d0->bottomSizer == s1);
    assert_true(d0->rightSizer == s0);
    assert_true(d0->leftSizer == &grid->leftSizer);

    assert_true(d1->topSizer == &grid->topSizer);
    assert_true(d1->bottomSizer == &grid->bottomSizer);
    assert_true(d1->rightSizer == &grid->rightSizer);
    assert_true(d1->leftSizer == s0);

    assert_true(d2->topSizer == s1);
    assert_true(d2->bottomSizer == &grid->bottomSizer);
    assert_true(d2->rightSizer == s0);
    assert_true(d2->leftSizer == &grid->leftSizer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_sizer_hovering(void**)
{
    Rect rect = {{{ 0, 0, 1000, 500 }}};
    Vec2 pos;

    UIDockingGrid* grid = UIDock_createGrid(&rect);

    UIDockSizer* s0 = new UIDockSizer;
    s0->rect.x = 0;
    s0->rect.y = 0;
    s0->rect.width = 10;
    s0->rect.height = g_sizerSize;
    s0->dir = UIDockSizerDir_Horz;

    UIDockSizer* s1 = new UIDockSizer;
    s1->rect.x = 20;
    s1->rect.y = 20;
    s1->rect.width = g_sizerSize;
    s1->rect.height = 20;
    s1->dir = UIDockSizerDir_Vert;

    grid->sizers.push_back(s0);
    grid->sizers.push_back(s1);

    pos.x = -10.0f;
    pos.y = -10.0f;

    assert_true(UIDock_isHoveringSizer(grid, &pos) == UIDockSizerDir_None);

    pos.x = 120.0f;
    pos.y = 20.0f;

    assert_true(UIDock_isHoveringSizer(grid, &pos) == UIDockSizerDir_None);

    pos.x = 0.0f;
    pos.y = 0.0f;

    assert_true(UIDock_isHoveringSizer(grid, &pos) == UIDockSizerDir_Horz);

    pos.x = 6.0f;
    pos.y = 6.0f;

    assert_true(UIDock_isHoveringSizer(grid, &pos) == UIDockSizerDir_Horz);

    pos.x = 20.0f;
    pos.y = 20.0f;

    assert_true(UIDock_isHoveringSizer(grid, &pos) == UIDockSizerDir_Vert);

    pos.x = 18.0f;
    pos.y = 30.0f;

    assert_true(UIDock_isHoveringSizer(grid, &pos) == UIDockSizerDir_Vert);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static UIDockingGrid* createFourViews(Rect rect)
{
    UIDockingGrid* grid = UIDock_createGrid(&rect);

    static ViewPluginInstance view0 = {};
    static ViewPluginInstance view1 = {};
    static ViewPluginInstance view2 = {};
    static ViewPluginInstance view3 = {};

    UIDock* dock = UIDock_addView(grid, &view0);
    UIDock_dockRight(grid, dock, &view1);
    UIDock_dockBottom(grid, dock, &view2);
    UIDock_dockBottom(grid, grid->docks[1], &view3);

    // Expected layout:
    //
    //     _____s0_______
    //    |      |      |
    //    |  d0  |  d1  |
    //    |      |      |
    // s1 |------|------| s1
    //    |      |      |
    //    |  d2  |  d3  |
    //    |      |      |
    //    ---------------
    //
    // Notice: as we merge sizers there will be no s2 sizer here because
    // s1 will be resized to include the the sizing on the other size

    assert_int_equal((int)grid->sizers.size(), 2);
    assert_int_equal((int)grid->docks.size(), 4);

    UIDockSizer* s0 = grid->sizers[0];
    UIDockSizer* s1 = grid->sizers[1];

    UIDock* d0 = grid->docks[0];
    UIDock* d1 = grid->docks[1];
    UIDock* d2 = grid->docks[2];
    UIDock* d3 = grid->docks[3];

    assert_true(d0->topSizer == &grid->topSizer);
    assert_true(d0->bottomSizer == s1);
    assert_true(d0->rightSizer == s0);
    assert_true(d0->leftSizer == &grid->leftSizer);

    assert_true(d1->topSizer == &grid->topSizer);
    assert_true(d1->bottomSizer == s1);
    assert_true(d1->rightSizer == &grid->rightSizer);
    assert_true(d1->leftSizer == s0);

    assert_true(d2->topSizer == s1);
    assert_true(d2->bottomSizer == &grid->bottomSizer);
    assert_true(d2->rightSizer == s0);
    assert_true(d2->leftSizer == &grid->leftSizer);

    assert_true(d3->topSizer == s1);
    assert_true(d3->bottomSizer == &grid->bottomSizer);
    assert_true(d3->rightSizer == &grid->rightSizer);
    assert_true(d3->leftSizer == s0);

    return grid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_dock_split_horizontal(void**)
{
    Rect rect = {{{ 0, 0, 1000, 500 }}};

    UIDockingGrid* grid = createFourViews(rect);

    UIDockSizer* s1 = grid->sizers[1];

    UIDock_splitSizer(grid, s1, 10, rect.height / 2);

    //     _____s0_______
    //    |      |      |
    //    |  d0  |  d1  |
    //    |      |      |
    // s2 |------|------| s1
    //    |      |      |
    //    |  d2  |  d3  |
    //    |      |      |
    //    ---------------

    // s2 should be the new split added

    assert_int_equal((int)grid->sizers.size(), 3);
    assert_int_equal((int)s1->cons.size(), 2);

    UIDockSizer* s2 = grid->sizers[2];
    assert_int_equal((int)s2->cons.size(), 2);

    UIDock* d0 = grid->docks[0];
    UIDock* d2 = grid->docks[2];

    // Verify that the sizers has change to correct sizes

    assert_int_equal(s1->rect.x, rect.width / 2);
    assert_int_equal(s1->rect.y, rect.height / 2);
    assert_int_equal(s1->rect.width, rect.width / 2);
    assert_int_equal(s1->rect.height, 0);

    assert_int_equal(s2->rect.x, 0);
    assert_int_equal(s2->rect.y, rect.height / 2);
    assert_int_equal(s2->rect.width, rect.width / 2);
    assert_int_equal(s2->rect.height, 0);

    // Verify that the docks has got the new sizers assigned

    assert_true(d0->bottomSizer == s2);
    assert_true(d2->topSizer == s2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_dock_split_vertical(void**)
{
    Rect rect = {{{ 0, 0, 800, 200 }}};

    UIDockingGrid* grid = createFourViews(rect);

    UIDockSizer* s0 = grid->sizers[0];

    UIDock_splitSizer(grid, s0, rect.x / 2, rect.height - 10);

    assert_int_equal((int)grid->sizers.size(), 3);
    assert_int_equal((int)s0->cons.size(), 2);

    UIDockSizer* s2 = grid->sizers[2];
    assert_int_equal((int)s2->cons.size(), 2);

    UIDock* d2 = grid->docks[2];
    UIDock* d3 = grid->docks[3];

    // Verify that the sizers has change to correct sizes

    assert_int_equal(s0->rect.x, rect.width / 2);
    assert_int_equal(s0->rect.y, rect.y);
    assert_int_equal(s0->rect.width, 0);
    assert_int_equal(s0->rect.height, rect.height / 2);

    assert_int_equal(s2->rect.x, rect.width / 2);
    assert_int_equal(s2->rect.y, (rect.height / 2));
    assert_int_equal(s2->rect.width, 0);
    assert_int_equal(s2->rect.height, (rect.height / 2));

    // Verify that the docks has got the new sizers assigned

    assert_true(d2->rightSizer == s2);
    assert_true(d3->leftSizer == s2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_delete_docks_right_left(void**)
{
    Rect rect = {{{ 0, 0, 1000, 500 }}};

    UIDockingGrid* grid = UIDock_createGrid(&rect);

    ViewPluginInstance view0 = {};
    ViewPluginInstance view1 = {};
    ViewPluginInstance view2 = {};

    UIDock* dock = UIDock_addView(grid, &view0);
    UIDock_dockRight(grid, dock, &view1);

    {
        // at this point we should have tree views looking like this:
        //  ____s0___
        // |    |   |
        // | d0 |d1 |
        // |    |   |
        // ----------

        assert_int_equal((int)grid->sizers.size(), 1);
        assert_int_equal((int)grid->docks.size(), 2);

        UIDockSizer* s = grid->sizers[0];

        assert_int_equal((int)grid->topSizer.cons.size(), 2);
        assert_int_equal((int)grid->rightSizer.cons.size(), 1);
        assert_int_equal((int)grid->bottomSizer.cons.size(), 2);
        assert_int_equal((int)grid->leftSizer.cons.size(), 1);

        assert_int_equal((int)s->cons.size(), 2);

        UIDock_dockTop(grid, dock, &view2);

        // Expected layout:
        //
        //     _____s0_______
        //    |      |      |
        //    |  d2  |      |
        //    |      |      |
        // s1 |------|  d1  |
        //    |      |      |
        //    |  d0  |      |
        //    |      |      |
        //    ---------------
        //
        //

        assert_int_equal((int)grid->sizers.size(), 2);
        assert_int_equal((int)grid->docks.size(), 3);

        UIDockSizer* s0 = grid->sizers[0];
        UIDockSizer* s1 = grid->sizers[1];

        UIDock* d0 = grid->docks[0];
        UIDock* d1 = grid->docks[1];
        UIDock* d2 = grid->docks[2];

        assert_true(d0->topSizer == s1);
        assert_true(d0->bottomSizer == &grid->bottomSizer);
        assert_true(d0->rightSizer == s0);
        assert_true(d0->leftSizer == &grid->leftSizer);

        assert_true(d1->topSizer == &grid->topSizer);
        assert_true(d1->bottomSizer == &grid->bottomSizer);
        assert_true(d1->rightSizer == &grid->rightSizer);
        assert_true(d1->leftSizer == s0);

        assert_true(d2->topSizer == &grid->topSizer);
        assert_true(d2->bottomSizer == s1);
        assert_true(d2->rightSizer == s0);
        assert_true(d2->leftSizer == &grid->leftSizer);

        // Verify that the sizers has all the viwes connecetd

        assert_int_equal((int)grid->topSizer.cons.size(), 2);
        assert_int_equal((int)grid->rightSizer.cons.size(), 1);
        assert_int_equal((int)grid->bottomSizer.cons.size(), 2);
        assert_int_equal((int)grid->leftSizer.cons.size(), 2);

        assert_int_equal((int)s0->cons.size(), 3);
        assert_int_equal((int)s1->cons.size(), 2);
    }


    {
        UIDock_deleteView(grid, grid->docks[1]->view);

        // Expected layout:
        //
        //     _____s0_______
        //    |             |
        //    |     d1      |
        //    |             |
        // s1 |-------------|
        //    |             |
        //    |     d0      |
        //    |             |
        //    ---------------
        //
        //

        assert_int_equal((int)grid->sizers.size(), 1);
        assert_int_equal((int)grid->docks.size(), 2);

        UIDockSizer* s0 = grid->sizers[0];
        UIDock* d0 = grid->docks[0];
        UIDock* d1 = grid->docks[1];

        assert_true(d0->topSizer == s0);
        assert_true(d0->bottomSizer == &grid->bottomSizer);
        assert_true(d0->rightSizer == &grid->rightSizer);
        assert_true(d0->leftSizer == &grid->leftSizer);

        assert_true(d1->topSizer == &grid->topSizer);
        assert_true(d1->bottomSizer == s0);
        assert_true(d1->rightSizer == &grid->rightSizer);
        assert_true(d1->leftSizer == &grid->leftSizer);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_delete_docks_left_right(void**)
{
    Rect rect = {{{ 0, 0, 1000, 500 }}};

    UIDockingGrid* grid = UIDock_createGrid(&rect);

    ViewPluginInstance view0 = {};
    ViewPluginInstance view1 = {};
    ViewPluginInstance view2 = {};

    UIDock* dock = UIDock_addView(grid, &view0);
    UIDock_dockRight(grid, dock, &view1);
    UIDock_dockBottom(grid, grid->docks[1], &view2);

	// Expected layout:
	//
	//     _____s0_______
	//    |      |      |
	//    |      |  d1  |
	//    |      |      |
	// s1 |  d0  |------|
	//    |      |      |
	//    |      |  d2  |
	//    |      |      |
	//    ---------------
	//

	UIDock_deleteView(grid, grid->docks[0]->view);

	// Expected layout:
	//
	//     _____s0_______
	//    |             |
	//    |     d0      |
	//    |             |
	// s1 |-------------|
	//    |             |
	//    |     d1      |
	//    |             |
	//    ---------------
	//

	assert_int_equal((int)grid->sizers.size(), 1);
	assert_int_equal((int)grid->docks.size(), 2);

	UIDockSizer* s0 = grid->sizers[0];
	UIDock* d0 = grid->docks[0];
	UIDock* d1 = grid->docks[1];

	assert_true(d0->topSizer == &grid->topSizer);
	assert_true(d0->bottomSizer == s0);
	assert_true(d0->rightSizer == &grid->rightSizer);
	assert_true(d0->leftSizer == &grid->leftSizer);

	assert_true(d1->topSizer == s0);
	assert_true(d1->bottomSizer == &grid->bottomSizer);
	assert_true(d1->rightSizer == &grid->rightSizer);
	assert_true(d1->leftSizer == &grid->leftSizer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_delete_docks_up_down(void**)
{
    Rect rect = {{{ 0, 0, 1000, 500 }}};

    UIDockingGrid* grid = UIDock_createGrid(&rect);

    ViewPluginInstance view0 = {};
    ViewPluginInstance view1 = {};
    ViewPluginInstance view2 = {};

    UIDock* dock = UIDock_addView(grid, &view0);
    UIDock_dockRight(grid, dock, &view1);
    UIDock_dockBottom(grid, grid->docks[1], &view2);

	// Expected layout:
	//
	//     _____s0_______
	//    |      |      |
	//    |      |  d1  |
	//    |      |      |
	// s1 |  d0  |------|
	//    |      |      |
	//    |      |  d2  |
	//    |      |      |
	//    ---------------
	//

	UIDock_deleteView(grid, grid->docks[2]->view);

	// Expected layout:
	//
	//     _____s0_______
	//    |      |      | 
	//    |      |      |
	//    |      |      |
	//    | d0   |  d1  |
	//    |      |      |
	//    |      |      |
	//    |      |      |
	//    ---------------
	//

	assert_int_equal((int)grid->sizers.size(), 1);
	assert_int_equal((int)grid->docks.size(), 2);

	UIDockSizer* s0 = grid->sizers[0];
	UIDock* d0 = grid->docks[0];
	UIDock* d1 = grid->docks[1];

	assert_true(d0->topSizer == &grid->topSizer);
	assert_true(d0->bottomSizer == &grid->bottomSizer);
	assert_true(d0->rightSizer == s0);
	assert_true(d0->leftSizer == &grid->leftSizer);

	assert_true(d1->topSizer == &grid->topSizer);
	assert_true(d1->bottomSizer == &grid->bottomSizer);
	assert_true(d1->rightSizer == &grid->rightSizer);
	assert_true(d1->leftSizer == s0);

	UIDock_deleteView(grid, grid->docks[0]->view);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_delete_docks_down_up(void**)
{
    Rect rect = {{{ 0, 0, 1000, 500 }}};

    UIDockingGrid* grid = UIDock_createGrid(&rect);

    ViewPluginInstance view0 = {};
    ViewPluginInstance view1 = {};
    ViewPluginInstance view2 = {};

    UIDock* dock = UIDock_addView(grid, &view0);
    UIDock_dockRight(grid, dock, &view1);
    UIDock_dockBottom(grid, grid->docks[1], &view2);

	// Expected layout:
	//
	//     _____s0_______
	//    |      |      |
	//    |      |  d1  |
	//    |      |      |
	// s1 |  d0  |------|
	//    |      |      |
	//    |      |  d2  |
	//    |      |      |
	//    ---------------
	//

	UIDock_deleteView(grid, grid->docks[1]->view);

	// Expected layout:
	//
	//     _____s0_______
	//    |      |      | 
	//    |      |      |
	//    |      |      |
	//    | d0   |  d2  |
	//    |      |      |
	//    |      |      |
	//    |      |      |
	//    ---------------
	//

	assert_int_equal((int)grid->sizers.size(), 1);
	assert_int_equal((int)grid->docks.size(), 2);

	UIDockSizer* s0 = grid->sizers[0];
	UIDock* d0 = grid->docks[0];
	UIDock* d1 = grid->docks[1];

	assert_true(d0->topSizer == &grid->topSizer);
	assert_true(d0->bottomSizer == &grid->bottomSizer);
	assert_true(d0->rightSizer == s0);
	assert_true(d0->leftSizer == &grid->leftSizer);

	assert_true(d1->topSizer == &grid->topSizer);
	assert_true(d1->bottomSizer == &grid->bottomSizer);
	assert_true(d1->rightSizer == &grid->rightSizer);
	assert_true(d1->leftSizer == s0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_drag_vertical(void**)
{
    Vec2 dragDelta = { 10.0f, 10.f };
    Rect rect = {{{ 0, 0, 1000, 400 }}};

    UIDockingGrid* grid = UIDock_createGrid(&rect);

    ViewPluginInstance view0Inst = {};
    ViewPluginInstance view1Inst = {};

    ViewPluginInstance* view0 = &view0Inst;
    ViewPluginInstance* view1 = &view1Inst;

    UIDock* dock = UIDock_addView(grid, view0);
    UIDock_dockLeft(grid, dock, view1);

    assert_int_equal((int)grid->sizers.size(), 1);
    assert_int_equal((int)grid->docks.size(), 2);

    UIDockSizer* s0 = grid->sizers[0];
    UIDock* d0 = grid->docks[0];
    UIDock* d1 = grid->docks[1];

    assert_int_equal(s0->rect.x, rect.width / 2);
    assert_int_equal(s0->rect.y, 0);
    assert_int_equal(s0->rect.width, 0);
    assert_int_equal(s0->rect.height, rect.height);

    assert_int_equal(d0->view->rect.x, rect.width / 2);
    assert_int_equal(d0->view->rect.y, 0);
    assert_int_equal(d0->view->rect.width, rect.width / 2);
    assert_int_equal(d0->view->rect.height, rect.height);

    assert_int_equal(d1->view->rect.x, 0);
    assert_int_equal(d1->view->rect.y, 0);
    assert_int_equal(d1->view->rect.width, rect.width / 2);
    assert_int_equal(d1->view->rect.height, rect.height);

    UIDock_dragSizer(grid, s0, &dragDelta);

    assert_int_equal(s0->rect.x, (rect.width / 2) + (int)dragDelta.x);
    assert_int_equal(s0->rect.y, 0);
    assert_int_equal(s0->rect.width, 0);
    assert_int_equal(s0->rect.height, rect.height);

    assert_int_equal(d0->view->rect.x, (rect.width / 2) + (int)dragDelta.x);
    assert_int_equal(d0->view->rect.y, 0);
    assert_int_equal(d0->view->rect.width, (rect.width / 2) - (int)dragDelta.x);
    assert_int_equal(d0->view->rect.height, rect.height);

    assert_int_equal(d1->view->rect.x, 0);
    assert_int_equal(d1->view->rect.y, 0);
    assert_int_equal(d1->view->rect.width, (rect.width / 2) + (int)dragDelta.x);
    assert_int_equal(d1->view->rect.height, rect.height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    const UnitTest tests[] =
    {
    	/*
        unit_test(create_docking),
        unit_test(test_left_attach),
        unit_test(test_misc),
        unit_test(test_sizer_hovering),
        unit_test(test_dock_split_horizontal),
        unit_test(test_dock_split_vertical),
        unit_test(test_drag_vertical),
        unit_test(test_delete_docks_right_left),
        unit_test(test_delete_docks_left_right),
        */
		unit_test(test_delete_docks_up_down),
		//unit_test(test_delete_docks_down_up),
    };

    return run_tests(tests);
}

