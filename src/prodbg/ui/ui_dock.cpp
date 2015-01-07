#include "ui_dock.h"
#include "core/alloc.h"
#include "core/math.h"
#include "ui_dock_private.h"
#include "plugin.h"

#include <stddef.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_createGrid(Rect* rect)
{
    UIDockingGrid* grid = new UIDockingGrid;
    grid->rect = *rect;

    grid->topSizer.rect = {{{ rect->x, rect->y, rect->width, 0 }}};
    grid->bottomSizer.rect = {{{ rect->x, rect->height, rect->width, 0 }}};
    grid->leftSizer.rect = {{{ rect->x, rect->y, 0, rect->height }}};
    grid->rightSizer.rect = {{{ rect->width, rect->y, 0, rect->height }}};

    return grid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDock* UIDock_addView(UIDockingGrid* grid, ViewPluginInstance* view)
{
    UIDock* dock = new UIDock(view);

    dock->topSizer = &grid->topSizer;
    dock->bottomSizer = &grid->bottomSizer;
    dock->rightSizer = &grid->rightSizer;
    dock->leftSizer = &grid->leftSizer;

    // Add the dock to the sizers

    grid->topSizer.addDock(dock);
    grid->bottomSizer.addDock(dock);
    grid->rightSizer.addDock(dock);
    grid->leftSizer.addDock(dock);

    // If this is the first we we just set it as maximized

    if (grid->docks.size() == 0)
        dock->view->rect = grid->rect;

    // TODO: How should we really handle this case?

    grid->docks.push_back(dock);

    return dock;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void removeDockSide(UIDockSizer* sizer, UIDock* dock)
{
    for (auto i = sizer->cons.begin(), end = sizer->cons.end(); i != end; ++i)
    {
        if (*i == dock)
        {
            sizer->cons.erase(i);
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This code will look for a sizer that can be resized if adjacent sizer is present

static UIDockSizer* createOrResizeSizer(UIDockingGrid* grid, Rect rect, UIDockSizerDir dir)
{
    // TODO: Doing this for all sizers is a bit wasteful perf wise but really shouldn't be much of an issue
    // but something to think about.
    //

    const int sx0 = rect.x;
    const int sy0 = rect.y;
    const int sx1 = sx0 + rect.width;
    const int sy1 = sy0 + rect.height;

    for (UIDockSizer* sizer : grid->sizers)
    {
        // Can only resize a sizer with the same direction
        // TODO: Have a separate array for sizers depending on direction?

        if (sizer->dir != dir)
            continue;

        const int cx0 = sizer->rect.x;
        const int cy0 = sizer->rect.y;
        const int cx1 = cx0 + sizer->rect.width;
        const int cy1 = cy0 + sizer->rect.height;

        // Check if the new sizer is connected so we can resize the current sizer

        if (cx1 == sx0 && cy1 == sy0)
        {
            // we can resize it. Just calculate the new size

            sizer->rect.width += rect.width;
            sizer->rect.height += rect.height;

            return sizer;
        }
        else if (cx0 == sx1 && cy0 == sy1)
        {
            // move the position of the sizer to where the new sizer was supposed to start

            sizer->rect.x = rect.x;
            sizer->rect.y = rect.y;

            return sizer;
        }
    }

    // Need to create a new sizer as we didn't find any to resize

    UIDockSizer* sizer = new UIDockSizer;
    sizer->rect = rect;
    sizer->dir = dir;

    grid->sizers.push_back(sizer);

    return sizer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dockSide(UIDockSide side, UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* instance)
{
    if (grid->docks.size() == 0)
    {
        UIDock_addView(grid, instance);
        return;
    }

    UIDock* newDock = new UIDock(instance);
    UIDockSizer* sizer = new UIDockSizer;
    UIDockSizerDir sizerDir = UIDockSizerDir_Vert;

    Rect rect = dock->view->rect;

    int sizerWidthOrHeight = Rect::H;
    int widthOrHeight = Rect::W;
    int xOry = Rect::X;
    int side0 = UIDock::Left;
    int side1 = UIDock::Right;
    int side2 = UIDock::Top;
    int side3 = UIDock::Bottom;

    if (side == UIDockSide_Top || side == UIDockSide_Bottom)
    {
        sizerWidthOrHeight = Rect::W;
        widthOrHeight = Rect::H;
        xOry = Rect::Y;
        side0 = UIDock::Top;
        side1 = UIDock::Bottom;
        side2 = UIDock::Left;
        side3 = UIDock::Right;
        sizerDir = UIDockSizerDir_Horz;
    }

    const int startWidthOrRect = rect.data[sizerWidthOrHeight];

    rect.data[widthOrHeight] /= 2;

    dock->view->rect.data[widthOrHeight] = rect.data[widthOrHeight];
    Rect sizerRect = rect;

    if (side == UIDockSide_Top || side == UIDockSide_Left)
        dock->view->rect.data[xOry] += rect.data[widthOrHeight];
    else
        rect.data[xOry] += rect.data[widthOrHeight];

    sizerRect.data[xOry] += rect.data[widthOrHeight];

    if (side == UIDockSide_Top || side == UIDockSide_Bottom)
    {
        sizerRect.width = startWidthOrRect;
        sizerRect.height = 0;
    }
    else
    {
        sizerRect.height = startWidthOrRect;
        sizerRect.width = 0;
    }

    sizer = createOrResizeSizer(grid, sizerRect, sizerDir);

    newDock->sizers[side2] = dock->sizers[side2];   // left or top
    newDock->sizers[side3] = dock->sizers[side3];   // right or bottom

    newDock->sizers[side2]->addDock(newDock);
    newDock->sizers[side3]->addDock(newDock);

    if (side == UIDockSide_Top || side == UIDockSide_Left)
    {
        removeDockSide(dock->sizers[side0], dock);  // top or left
        newDock->sizers[side0] = dock->sizers[side0];
        newDock->sizers[side1] = sizer;
        dock->sizers[side0] = sizer;

        newDock->sizers[side0]->addDock(newDock);
    }
    else
    {
        removeDockSide(dock->sizers[side1], dock);  // bottom or right
        newDock->sizers[side1] = dock->sizers[side1];
        newDock->sizers[side0] = sizer;
        dock->sizers[side1] = sizer;

        newDock->sizers[side1]->addDock(newDock);
    }

    sizer->addDock(dock);
    sizer->addDock(newDock);

    newDock->view->rect = rect;

    grid->docks.push_back(newDock);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline UIDockSizerDir isHoveringSizer(UIDockSizer* sizer, Vec2* size)
{
    UIDockSizerDir dir = sizer->dir;

    float x = (float)sizer->rect.x;
    float y = (float)sizer->rect.y;
    float w = x + (float)sizer->rect.width;
    float h = y + (float)sizer->rect.height;

    // resize the sizes with the snap area

    if (dir == UIDockSizerDir_Horz)
    {
        y -= g_sizerSnapSize;
        h += g_sizerSnapSize;
    }
    else
    {
        x -= g_sizerSnapSize;
        w += g_sizerSnapSize;
    }

    if ((size->x >= x && size->x < w) && (size->y >= y && size->y < h))
        return dir;

    return UIDockSizerDir_None;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_dockTop(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* instance)
{
    dockSide(UIDockSide_Top, grid, dock, instance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_dockBottom(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* instance)
{
    dockSide(UIDockSide_Bottom, grid, dock, instance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_dockLeft(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* instance)
{
    dockSide(UIDockSide_Left, grid, dock, instance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_dockRight(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* instance)
{
    dockSide(UIDockSide_Right, grid, dock, instance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_splitHorizontal(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* newInstance)
{
    UIDock_dockBottom(grid, dock, newInstance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_splitVertical(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* newInstance)
{
    UIDock_dockLeft(grid, dock, newInstance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockSizerDir UIDock_isHoveringSizer(UIDockingGrid* grid, Vec2* pos)
{
    for (UIDockSizer* sizer : grid->sizers)
    {
        UIDockSizerDir dir = isHoveringSizer(sizer, pos);

        if (dir != UIDockSizerDir_None)
            return dir;
    }

    return UIDockSizerDir_None;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct NeighborDocks
{
    std::vector<UIDock*> topLeft;   // up/left
    std::vector<UIDock*> bottomRight;   // bellow/right
    std::vector<UIDock*> insideDocks;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void findSurroundingViews(NeighborDocks* docks, UIDockSizer* sizer, const UIDock* currentDock, int widthOrHeight, int xOry)
{
    const int txy = currentDock->view->rect.data[xOry];
    const int thw = txy + currentDock->view->rect.data[widthOrHeight];

    for (UIDock* dock : sizer->cons)
    {
        if (dock == currentDock)
            continue;

        // bounds for the view

        const int ctxy = dock->view->rect.data[xOry];
        const int cthw = ctxy + dock->view->rect.data[widthOrHeight];

        if (ctxy >= txy && cthw <= thw)
            docks->insideDocks.push_back(dock);
        else if (thw < cthw)
            docks->bottomRight.push_back(dock);
        else
            docks->topLeft.push_back(dock);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Depending on where the sizer has been dragged we need to figure out which dock that has been moved (this can be used
// when spliting is ineeded) It really doesn't matter which side of the sizer we get.

UIDock* findDock(const UIDockSizer* sizer, int x, int y)
{
    const UIDockSizerDir dir = sizer->dir;

    if (dir == UIDockSizerDir_Vert)
    {
        for (UIDock* dock : sizer->cons)
        {
            const int y0 = dock->view->rect.y;
            const int y1 = y0 + dock->view->rect.height;

            if (y >= y0 && y < y1)
                return dock;
        }
    }
    else
    {
        for (UIDock* dock : sizer->cons)
        {
            const int x0 = dock->view->rect.x;
            const int x1 = x0 + dock->view->rect.width;

            if (x >= x0 && x < x1)
                return dock;
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void replaceSizer(UIDock* dock, UIDockSizer* oldSizer, UIDockSizer* newSizer)
{
    for (int i = 0; i < UIDock::Sizers_Count; ++i)
    {
        if (dock->sizers[i] != oldSizer)
            continue;

        dock->sizers[i] = newSizer;
        newSizer->addDock(dock);
    }

    removeDockSide(oldSizer, dock);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void replaceSizers(std::vector<UIDock*>& docks, UIDockSizer* oldSizer, UIDockSizer* newSizer)
{
    for (UIDock* dock : docks)
        replaceSizer(dock, oldSizer, newSizer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void refitDocks(std::vector<UIDock*>& docks)
{
    for (UIDock* dock : docks)
	{
		dock->view->rect.x = dock->leftSizer->rect.x;
		dock->view->rect.y = dock->topSizer->rect.y;
		dock->view->rect.width = dock->rightSizer->rect.x - dock->leftSizer->rect.x;
		dock->view->rect.height = dock->bottomSizer->rect.y - dock->topSizer->rect.y;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void deleteSizer(UIDockingGrid* grid, UIDockSizer* sizer)
{
    for (auto i = grid->sizers.begin(), end = grid->sizers.end(); i != end; ++i)
	{
		if (*i != sizer)
			continue;

		grid->sizers.erase(i);
		delete sizer;
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void deleteDockMem(UIDockingGrid* grid, UIDock* dock)
{
    for (auto i = grid->docks.begin(), end = grid->docks.end(); i != end; ++i)
	{
		if (*i != dock)
			continue;

		grid->docks.erase(i);
		delete dock;
		return;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// In order for the splitting to work (in vertical size) there has to be a horizontal top slider and bottom slider
// that the new split can move along. Otherwise there will be no split as it would move over and this code
// will return without doing any split.
//
// TODO: Verify that this really is correct:
//
// We check this looking at the top sizer and see if it's larger than the windown in both x and y directions.
// if that is the case we can do the split
//
// Notice that this code handes both vertical/horizontal checks given widthOrHeight/xOry params

bool canSplitSizer(const UIDock* dock, int sideSizer, int widthOrHeight, int xOry)
{
    const int sxy = dock->sizers[sideSizer]->rect.data[xOry];
    const int swh = dock->sizers[sideSizer]->rect.data[widthOrHeight];

    const int dxy = dock->view->rect.data[xOry];
    const int dwh = dock->view->rect.data[widthOrHeight];

    if (sxy < dxy || swh > dwh)
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Splitsizers. This is needed in order to to move a sizer independent of the previous sizer. This is needed if a sizer
// is to move like this:
//
//     _____s0_______
//    |      |      |
//    |  d0  |  d1  |
//    |      |      |
// s1 |------|------| s2
//    |      |      |
//    |  d2  |  d3  |
//    |      |      |
//    ---------------
//
//          |||
//          \|/
//
//     _____s0_______
//    |      |      |
//    |  d0  |  d1  |
//    |      |      |
// s1 |-------------| s2
//    |        |    |
//    |  d2    | d3 |
//    |        |    |
//    ---------------
//             s3
//
// In order for the splitting to work (in vertical size) there has to be a horizontal top slider and bottom slider
// that the new split can move along. Otherwise there will be no split as it would move over and this code
// will return without doing any split.
//
// The code will first after deciding that a split a possible collect the docks which are above and bellow the current
// splitter dock to be splited.
//
// The split that is above the view will keep the old splitter (but size will be changed) and new splitter will be created
// for the the current sizer and yet another one will be created for the views bellow the current view (if there are any)
//

void UIDock_splitSizer(UIDockingGrid* grid, UIDockSizer* sizer, int x, int y)
{
    UIDock* dock = nullptr;

    if (!(dock = findDock(sizer, x, y)))
        return;

    int canSplitPos = Rect::X;
    int canSplitSize = Rect::W;

    int xOry = Rect::Y;
    int widthOrHeight  = Rect::H;

    int sideSizer = UIDock::Top;

    if (sizer->dir == UIDockSizerDir_Horz)
    {
        canSplitPos = Rect::Y;
        canSplitSize = Rect::H;

        sideSizer = UIDock::Left;

        xOry = Rect::X;
        widthOrHeight = Rect::W;
    }

    NeighborDocks closeDocks;

    if (!canSplitSizer(dock, sideSizer, canSplitSize, canSplitPos))
        return;

    findSurroundingViews(&closeDocks, sizer, dock, widthOrHeight, xOry);

    // The new sizer should be the same height (and y start) as the dock we are splitting it for

    UIDockSizer* newSizer = new UIDockSizer;
    newSizer->rect = sizer->rect;
    newSizer->rect.data[xOry] = dock->view->rect.data[xOry];
    newSizer->rect.data[widthOrHeight] = dock->view->rect.data[widthOrHeight];
    newSizer->dir = sizer->dir;

    // Replace the old sizer with the new sizer for the split

    replaceSizers(closeDocks.insideDocks, sizer, newSizer);
    replaceSizer(dock, sizer, newSizer);

    // if topLeftDocks or bottomRightDocks has no entries it means that we only need to create one
    // new sizer and just resize the old one. If that is not the case it means that the split
    // was done in the middle of some views and another split is required

    const size_t topLeftDocksCount = closeDocks.topLeft.size();
    const size_t bottomRightDocksCount = closeDocks.bottomRight.size();

    if (topLeftDocksCount == 0 || bottomRightDocksCount == 0)
    {
        const int resizeValue = newSizer->rect.data[widthOrHeight];

        if (topLeftDocksCount == 0)
        {
            // If no top docks it mean that the split was made at the top and we need to move the sizer down
            // of the old sizer and we are done

            sizer->rect.data[xOry] += resizeValue;
            sizer->rect.data[widthOrHeight] -= resizeValue;
        }
        else
        {
            // If no bottom docks it mean that the split was made at the bottom and we only need to set a new height
            // of the old sizer and we are done

            sizer->rect.data[widthOrHeight] -= resizeValue;
        }
    }
    else
    {
        // TODO: Implement
    }

    grid->sizers.push_back(newSizer);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The way the deleting of a view works is that the code first needs to try to figure out how the surrounding views
// should be resized to close the hole the deleted view will cause.
//
// The priority is this: (Using the same as ProDG)
//
// 0. Try to resize the connecting views left -> right to fill the gap
// 1. Try to resize the connecting views right -> left to fill the gap
// 2. Try to resize the connecting views from top -> down to fill the gap
// 3. Try to resize the connecting views from down -> up to fill the gap
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
static void deleteDock(UIDockingGrid* grid, UIDock* dock)
{
	Rect viewRect = dock->view->rect;

	// TODO: Special case if we only have one dock left

	// Case 0 (preferred direction)
	// In order to resize to the left -> right the bottom and top sizers on the left of the window
	// the top and bottom sizers *must* stretch to the left of the current view size
	//
	// ts _________
	//   |   |     |  
	//   |---| dv  |
	//   |---|     |
	// bs|___|_____|

	// We prefer to split resize left -> right (as described above we start with detecting that

	const int tx = dock->topSizer->rect.x;
	const int bx = dock->bottomSizer->rect.x;
	const int ly = dock->leftSizer->rect.y;
	const int ry = dock->rightSizer->rect.y;

	if (tx < viewRect.x && bx < viewRect.x)
	{
    	NeighborDocks closeDocks;

		// We can delete and resize to the left. This means that the views that are connected to the left sizer
		// needs to be set to use the right sizer of the current view

    	findSurroundingViews(&closeDocks, dock->leftSizer, dock, Rect::H, Rect::Y);

		// Replace the old sizer with the right sizer of the dock  

		replaceSizers(closeDocks.insideDocks, dock->leftSizer, dock->rightSizer);

		assert(dock->leftSizer != &grid->leftSizer);

		// Make sure to move the docks into their new places

		refitDocks(closeDocks.insideDocks);

		deleteSizer(grid, dock->leftSizer);
		deleteDockMem(grid, dock);

		// TODO: Create splits here if needed for upper and lower docks

		return;
	}
	else if (tx > viewRect.x && bx > viewRect.x)
	{
		// Case 1 this will do resize left <- right

		return;

	}
	else if (ly < viewRect.y && ry < viewRect.y)
	{
		// Case 2 resize top -> down

		return;
	}
	else if (ly > viewRect.y && ry > viewRect.y)
	{
		// Case 3 size down -> top

		return;
	}

	// Should never get here.
	// TODO: Proper error message

	assert(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_dragSizer(UIDockingGrid* grid, void* handle, Vec2* deltaMove)
{
    UIDockSizer* sizer = (UIDockSizer*)handle;

    (void)grid;

    if (sizer->dir == UIDockSizerDir_Vert)
    {
        std::vector<UIDock*> leftDocks;
        std::vector<UIDock*> rightDocks;

        int move = (int)deltaMove->x;

        for (UIDock* dock : sizer->cons)
        {
            if (dock->rightSizer == sizer)
                rightDocks.push_back(dock);
            if (dock->leftSizer == sizer)
                leftDocks.push_back(dock);
        }

        // TODO: Add limits of the resizing

        sizer->rect.x += move;

        for (UIDock* dock : leftDocks)
        {
            dock->view->rect.x += move;
            dock->view->rect.width -= move;
        }

        for (UIDock* dock : rightDocks)
            dock->view->rect.width += move;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_deleteView(UIDockingGrid* grid, ViewPluginInstance* view)
{
	for (UIDock* dock : grid->docks)
	{
		if (dock->view != view)
			continue;

		deleteDock(grid, dock);
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDock* UIDock_getDockAt(UIDockingGrid* grid, int x, int y)
{
    for (UIDock* dock : grid->docks)
    {
        Rect rect = dock->view->rect;

        const int x0 = rect.x;
        const int y0 = rect.y;
        const int x1 = x0 + rect.width;
        const int y1 = y0 + rect.height;

        if ((x >= x0 && x < x1) && (y >= y0 && y < y1))
            return dock;
    }

    return nullptr;
}


