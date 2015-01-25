#include "ui_dock.h"
#include "core/alloc.h"
#include "core/math.h"
#include "ui_dock_private.h"
#include "ui_render.h"
#include "plugin.h"
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <bgfx.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_createGrid(FloatRect* rect)
{
    UIDockingGrid* grid = new UIDockingGrid;
    grid->rect = *rect;

    grid->topSizer.rect = {{{ rect->x, rect->y, rect->width, 0 }}};
    grid->bottomSizer.rect = {{{ rect->x, rect->height, rect->width, 0 }}};
    grid->leftSizer.rect = {{{ rect->x, rect->y, 0, rect->height }}};
    grid->rightSizer.rect = {{{ rect->width, rect->y, 0, rect->height }}};

    grid->topSizer.dir = UIDockSizerDir_Horz;
    grid->bottomSizer.dir = UIDockSizerDir_Horz;
    grid->leftSizer.dir = UIDockSizerDir_Vert;
    grid->rightSizer.dir = UIDockSizerDir_Vert;

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
            sizer->cons.erase(i);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This code will look for a sizer that can be resized if adjacent sizer is present

static UIDockSizer* createOrResizeSizer(UIDockingGrid* grid, FloatRect rect, UIDockSizerDir dir)
{
    // TODO: Doing this for all sizers is a bit wasteful perf wise but really shouldn't be much of an issue
    // but something to think about.
    //

    const int sx0 = (int)rect.x;
    const int sy0 = (int)rect.y;
    const int sx1 = sx0 + (int)rect.width;
    const int sy1 = sy0 + (int)rect.height;

    for (UIDockSizer* sizer : grid->sizers)
    {
        // Can only resize a sizer with the same direction
        // TODO: Have a separate array for sizers depending on direction?

        if (sizer->dir != dir)
            continue;

        const int cx0 = (int)sizer->rect.x;
        const int cy0 = (int)sizer->rect.y;
        const int cx1 = cx0 + (int)sizer->rect.width;
        const int cy1 = cy0 + (int)sizer->rect.height;

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

            sizer->rect.width += sizer->rect.x - rect.x;
            sizer->rect.height += sizer->rect.y - rect.y;

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

    FloatRect rect = dock->view->rect;

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

    const float startWidthOrRect = rect.data[sizerWidthOrHeight];

    rect.data[widthOrHeight] /= 2;

    dock->view->rect.data[widthOrHeight] = rect.data[widthOrHeight];
    FloatRect sizerRect = rect;

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
    const int txy = (int)currentDock->view->rect.data[xOry];
    const int thw = txy + (int)currentDock->view->rect.data[widthOrHeight];

    for (UIDock* dock : sizer->cons)
    {
        if (dock == currentDock)
            continue;

        // bounds for the view

        const int ctxy = (int)dock->view->rect.data[xOry];
        const int cthw = ctxy + (int)dock->view->rect.data[widthOrHeight];

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
            const int y0 = (int)dock->view->rect.y;
            const int y1 = y0 + (int)dock->view->rect.height;

            if (y >= y0 && y < y1)
                return dock;
        }
    }
    else
    {
        for (UIDock* dock : sizer->cons)
        {
            const int x0 = (int)dock->view->rect.x;
            const int x1 = x0 + (int)dock->view->rect.width;

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
		float ty = dock->topSizer->rect.y;

		float rx = dock->rightSizer->rect.x;
		float lx = dock->leftSizer->rect.x;

		float by = dock->bottomSizer->rect.y;

		dock->view->rect.x = lx;
		dock->view->rect.y = ty;
		dock->view->rect.width = rx - lx;
		dock->view->rect.height = by - ty;

		assert(dock->view->rect.width > 0);
		assert(dock->view->rect.height > 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void deleteSizer(UIDockingGrid* grid, UIDockSizer* sizer)
{
    for (auto i = grid->sizers.begin(), end = grid->sizers.end(); i != end; ++i)
	{
		if (*i == sizer)
		{
			grid->sizers.erase(i);
			delete sizer;
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void removeDockFromSizer(UIDockSizer* sizer, UIDock* dock)
{
	for (auto i = sizer->cons.begin(), end = sizer->cons.end(); i != end; ++i)
	{
		if (*i == dock)
		{
			sizer->cons.erase(i);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool deleteDockMem(UIDockingGrid* grid, UIDock* dock)
{
	// remove the dock from all the sizers

	for (UIDockSizer* sizer : grid->sizers)
		removeDockFromSizer(sizer, dock);

	removeDockFromSizer(&grid->topSizer, dock);
	removeDockFromSizer(&grid->bottomSizer, dock);
	removeDockFromSizer(&grid->leftSizer, dock);
	removeDockFromSizer(&grid->rightSizer, dock);

    for (auto i = grid->docks.begin(), end = grid->docks.end(); i != end; ++i)
	{
		if (*i == dock)
		{
			grid->docks.erase(i);
			delete dock;
			break;
		}
	}

	return true;
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
    const int sxy = (int)dock->sizers[sideSizer]->rect.data[xOry];
    const int swh = (int)dock->sizers[sideSizer]->rect.data[widthOrHeight];

    const int dxy = (int)dock->view->rect.data[xOry];
    const int dwh = (int)dock->view->rect.data[widthOrHeight];

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
        const float resizeValue = newSizer->rect.data[widthOrHeight];

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

static bool deleteDockSide(UIDockingGrid* grid, UIDock* dock, UIDockSizer* remSizer, UIDockSizer* repSizer, int wOrh, int xOry)
{
	NeighborDocks closeDocks;

	// We can delete and resize to the left. This means that the views that are connected to the left sizer
	// needs to be set to use the right sizer of the current view

	findSurroundingViews(&closeDocks, remSizer, dock, wOrh, xOry);

    const size_t topLeftDocksCount = closeDocks.topLeft.size();
    const size_t bottomRightDocksCount = closeDocks.bottomRight.size();
    const float resizeValue = dock->view->rect.data[wOrh];

	// Replace the old sizer with the right sizer of the dock  

	replaceSizers(closeDocks.insideDocks, remSizer, repSizer);

    // if we have no surrounding sizers we can just go ahead and delete and replace the sizer

    if (topLeftDocksCount == 0 && bottomRightDocksCount == 0)
	{
		// Safe to delete the sizer here

		deleteSizer(grid, remSizer);
	}
	else if (topLeftDocksCount == 0 || bottomRightDocksCount == 0)
	{
		// if one of the surrounding docks is zero when can just keep the sizers but change the size of it

		if (topLeftDocksCount == 0)
		{
            remSizer->rect.data[xOry] += resizeValue;
            remSizer->rect.data[wOrh] -= resizeValue;
		}
		else
		{
			remSizer->rect.data[wOrh] -= resizeValue;
		}
	}
	else
	{
		int topLeft = UIDock::Top;
		int bottomRight = UIDock::Bottom;

		if (wOrh != Rect::H)
		{
			topLeft = UIDock::Left;
			bottomRight = UIDock::Right;
		}

		// if we have a sizers on both sizes we:
		// 1. keep the upper one but change the size of it 
		// 2. create a new sizer and move it according to width/height of the dock
		// 3. reassign all the bottom/right docks with the new sizer 

		const float dxy0 = dock->sizers[bottomRight]->rect.data[xOry];
		const float rmxy0 = remSizer->rect.data[xOry]; 
		const float rmxy1 = rmxy0 + remSizer->rect.data[wOrh]; 

		UIDockSizer* newSizer = new UIDockSizer;
		newSizer->rect = remSizer->rect;
		newSizer->rect.data[xOry] = dock->sizers[bottomRight]->rect.data[xOry];
		newSizer->rect.data[wOrh] = rmxy1 - dxy0;
		newSizer->dir = repSizer->dir;

		// Adjust the size of the old sizer

		remSizer->rect.data[wOrh] = dock->sizers[topLeft]->rect.data[xOry] - remSizer->rect.data[xOry];

    	replaceSizers(closeDocks.bottomRight, remSizer, newSizer);

		grid->sizers.push_back(newSizer);
	}

    refitDocks(closeDocks.insideDocks);

    // Here is what this code needs to handle: When a dock (in this example) d0 need to be deleted s1 needs 
    // to be resized as it follows 'along' with dock d1/d2 dock
    //
    //
	//     _____s0_______
	//    |      |      |
	//    |      |  d1  |
	//    |  <-  |      |
	// s1 |  d0  |------|
	//    |  <-  |      |
	//    |      |  d2  |
	//    |      |      |
	//    ---------------

    // TODO: Can like do this better
	
	if (wOrh == Rect::H)
	{
		for (UIDock* cDock : closeDocks.insideDocks)
		{
			const float dx0 = cDock->view->rect.x;
			const float dx1 = cDock->view->rect.width + dx0;

			const float tx0 = cDock->topSizer->rect.x;
			const float tx1 = cDock->topSizer->rect.width + tx0;

			const float bx0 = cDock->bottomSizer->rect.x;
			const float bx1 = cDock->bottomSizer->rect.width + bx0;

			if ((int)dx0 < (int)tx0)
				cDock->topSizer->rect.x = dx0;

			if ((int)dx1 > (int)tx1)
				cDock->topSizer->rect.width = dx1 - cDock->topSizer->rect.x;
			else
				cDock->topSizer->rect.width = tx1 - cDock->topSizer->rect.x;

			if ((int)dx0 < (int)bx0)
				cDock->bottomSizer->rect.x = dx0;

			if ((int)dx1 > (int)bx1)
				cDock->bottomSizer->rect.width = dx1 - cDock->bottomSizer->rect.x;
			else
				cDock->bottomSizer->rect.width = bx1 - cDock->bottomSizer->rect.x;
		}
	}
	else
	{
		for (UIDock* cDock : closeDocks.insideDocks)
		{
			const float dy0 = cDock->view->rect.y;
			const float dy1 = cDock->view->rect.height + dy0;

			const float ty0 = cDock->leftSizer->rect.y;
			const float ty1 = cDock->leftSizer->rect.height + ty0;

			const float by0 = cDock->rightSizer->rect.y;
			const float by1 = cDock->rightSizer->rect.height + by0;

			if ((int)dy0 < (int)ty0)
				cDock->leftSizer->rect.y = dy0;

			if ((int)dy1 > (int)ty1)
				cDock->leftSizer->rect.height = dy1 - cDock->leftSizer->rect.y;
			else
				cDock->leftSizer->rect.height = ty1 - cDock->leftSizer->rect.y;

			if ((int)dy0 < (int)by0)
				cDock->rightSizer->rect.y = dy0;

			if ((int)dy1 > (int)by1)
				cDock->rightSizer->rect.height = dy1 - cDock->rightSizer->rect.y;
			else
				cDock->rightSizer->rect.height = by1 - cDock->rightSizer->rect.y;

		}
	}

	return deleteDockMem(grid, dock);
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
   
static bool deleteDock(UIDockingGrid* grid, UIDock* dock)
{
	FloatRect viewRect = dock->view->rect;

	// This is a special case if we only have one dock left we just delete it without testing anything

	if (grid->docks.size() == 1)
	{
		return deleteDockMem(grid, dock);
	}

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

	const int tx0 = (int)dock->topSizer->rect.x;
	const int tx1 = (int)dock->topSizer->rect.width + tx0;
	const int bx0 = (int)dock->bottomSizer->rect.x;
	const int bx1 = (int)dock->bottomSizer->rect.width + bx0;

	const int ly0 = (int)dock->leftSizer->rect.y;
	const int ly1 = (int)dock->leftSizer->rect.height + ly0;
	const int ry0 = (int)dock->rightSizer->rect.y;
	const int ry1 = (int)dock->rightSizer->rect.height + ry0;

	const int vx0 = (int)viewRect.x;
	const int vy0 = (int)viewRect.y;
	const int vx1 = (int)viewRect.width + vx0;
	const int vy1 = (int)viewRect.height + vy0;

	if (tx0 < vx0 && bx0 < vx0)
	{
		// Case 0 this will do resize right -> left

		return deleteDockSide(grid, dock, dock->leftSizer, dock->rightSizer, Rect::H, Rect::Y);
	}
	else if (tx1 > vx1 && bx1 > vx1)
	{
		// Case 1 this will do resize left <- right

		return deleteDockSide(grid, dock, dock->rightSizer, dock->leftSizer, Rect::H, Rect::Y);
	}
	else if (ly0 < vy0 && ry0 < vy0)
	{
		// Case 2 resize top -> down

		return deleteDockSide(grid, dock, dock->topSizer, dock->bottomSizer, Rect::W, Rect::X);
	}
	else if (ly1 > vy1 && ry1 > vy1)
	{
		// Case 3 size down -> top

		return deleteDockSide(grid, dock, dock->bottomSizer, dock->topSizer, Rect::W, Rect::X);
	}

	// this case may actually happen if we try to delete something that looks like this
	//
	// _______________
	// |_________|   |
	// |    |    |___|
	// |    |  x |   |
	// |    |____|___| 
	// |____|________|
	//
	// X in this case has no way split and thus we can't delete this view (Same behavior as ProDG)

	return false;
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
	else if (sizer->dir == UIDockSizerDir_Horz)
	{
        std::vector<UIDock*> topDocks;
        std::vector<UIDock*> bottomDocks;

        int move = (int)deltaMove->y;

        for (UIDock* dock : sizer->cons)
        {
            if (dock->topSizer == sizer)
                topDocks.push_back(dock);
            if (dock->bottomSizer == sizer)
                bottomDocks.push_back(dock);
        }

        // TODO: Add limits of the resizing

        sizer->rect.y += move;

        for (UIDock* dock : topDocks)
        {
            dock->view->rect.y += move;
            dock->view->rect.height -= move;
        }

        for (UIDock* dock : bottomDocks)
            dock->view->rect.height += move;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIDock_deleteView(UIDockingGrid* grid, ViewPluginInstance* view)
{
	for (UIDock* dock : grid->docks)
	{
		if (dock->view != view)
			continue;

		return deleteDock(grid, dock);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDock* UIDock_getDockAt(UIDockingGrid* grid, int x, int y)
{
    for (UIDock* dock : grid->docks)
    {
        FloatRect rect = dock->view->rect;

        const int x0 = (int)rect.x;
        const int y0 = (int)rect.y;
        const int x1 = (int)rect.width + x0;
        const int y1 = (int)rect.height + y0;

        if ((x >= x0 && x < x1) && (y >= y0 && y < y1))
            return dock;
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_updateSize(UIDockingGrid* grid, int width, int height)
{
	Vec2 deltaMove; 

	deltaMove.x = (float)(width - grid->rect.width);
	deltaMove.y = (float)(height - grid->rect.height);

	grid->rect.width = width;
	grid->rect.height = height;

	UIDock_dragSizer(grid, &grid->rightSizer, &deltaMove);
	UIDock_dragSizer(grid, &grid->bottomSizer, &deltaMove);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_update(UIDockingGrid* grid, const InputState* inputState)
{
	(void)inputState;

	switch (grid->state)
	{
		case UIDockState_None:
		{
			break;
		}

		case UIDockState_DragSizer:
		{
			break;
		}

		case UIDockState_BeginDragView:
		{
			break;
		}

		case UIDockState_DraggingView:
		{

			break;
		}

		case UIDockState_EndDragView:
	 	{
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_renderSizers(UIDockingGrid* grid)
{
	bgfx::TransientVertexBuffer tvb;

	const uint32_t vertexCount = (uint32_t)grid->sizers.size() * 6;

	UIRender_allocPosColorTb(&tvb, vertexCount); 
	PosColorVertex* verts = (PosColorVertex*)tvb.data;

	// TODO: Use settings for colors

	const uint32_t color = (0x40 << 16) | (0x40 << 8) | 0x40;

	for (UIDockSizer* sizer : grid->sizers)
	{
		FloatRect rect = sizer->rect;

		if (sizer->dir == UIDockSizerDir_Horz)
		{
			rect.y -= g_sizerSize / 2;
			rect.height = g_sizerSize;
		}
		else if (sizer->dir == UIDockSizerDir_Vert)
		{
			rect.x -= g_sizerSize / 2;
			rect.width = g_sizerSize;
		}
		else
		{
			assert(false);
		}

		const float x0 = rect.x;
		const float y0 = rect.y;
		const float x1 = rect.width + x0;
		const float y1 = rect.height + y0;

		// First triangle

		verts[0].x = x0;
		verts[0].y = y0;
		verts[0].color = color;

		verts[1].x = x1;
		verts[1].y = y0;
		verts[1].color = color;

		verts[2].x = x1;
		verts[2].y = y1;
		verts[2].color = color;

		// Second triangle

		verts[3].x = x0;
		verts[3].y = y0;
		verts[3].color = color;

		verts[4].x = x1;
		verts[4].y = y1;
		verts[4].color = color;

		verts[5].x = x0;
		verts[5].y = y1;
		verts[5].color = color;

		verts += 6;
	}

	bgfx::setState(0
					| BGFX_STATE_RGB_WRITE
					| BGFX_STATE_ALPHA_WRITE
					| BGFX_STATE_MSAA);

	UIRender_posColor(&tvb, 0, vertexCount);
}





