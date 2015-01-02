#include "ui_dock.h"
#include "core/alloc.h"
#include "core/math.h"
#include "ui_dock_private.h"
#include <stddef.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//const float g_splitPercentage = 0.5;	// TODO: Move to settings. Default split in middle (50/50)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_createGrid(Rect* rect)
{
	UIDockingGrid* grid = new UIDockingGrid; 
	grid->rect = *rect;

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

/*

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void calcVerticalSizerSize(UIDockSizer* sizer, const Rect* rect)
{
	sizer->rect = *rect;

	sizer->rect.width = g_sizerSize;
	sizer->rect.height = rect->height;
	sizer->dir = UIDockSizerDir_Vert;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void calcHorizonalSizerSize(UIDockSizer* sizer, const Rect* rect)
{
	sizer->rect = *rect;

	sizer->rect.width = rect->width;
	sizer->rect.height = g_sizerSize;
	sizer->dir = UIDockSizerDir_Horz;
}

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool removeDockSide(UIDockSizer* sizer, UIDock* dock)
{
	for (auto i = sizer->cons.begin(), end = sizer->cons.end(); i != end; ++i)
	{
		if (*i == dock)
		{
			sizer->cons.erase(i);
			return true;
		}
	}

	return false;
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
	UIDock* newDock = new UIDock(instance); 
	UIDockSizer* sizer = new UIDockSizer; 
	UIDockSizerDir sizerDir = UIDockSizerDir_Vert;

	Rect rect = dock->view->rect;

	int widthOrHeight = Rect::W;
	int xOry = Rect::X;
	int side0 = UIDock::Left;
	int side1 = UIDock::Right;
	int side2 = UIDock::Top;
	int side3 = UIDock::Bottom;

	if (side == UIDockSide_Top || side == UIDockSide_Bottom)
	{
		widthOrHeight = Rect::H;
		xOry = Rect::Y;
		side0 = UIDock::Top;
		side1 = UIDock::Bottom;
		side2 = UIDock::Left;
		side3 = UIDock::Right;
		sizerDir = UIDockSizerDir_Horz;
	}

	rect.data[widthOrHeight] /= 2;

	dock->view->rect.data[widthOrHeight] = rect.data[widthOrHeight];

	if (side == UIDockSide_Top || side == UIDockSide_Left)
		dock->view->rect.data[xOry] += rect.data[widthOrHeight];
	else
		rect.data[xOry] += rect.data[widthOrHeight];

	Rect sizerRect = rect;

	if (side == UIDockSide_Top || side == UIDockSide_Bottom)
	{
		sizerRect.width = rect.width;
		sizerRect.height = 0;
	}
	else
	{
		sizerRect.height = rect.width;
		sizerRect.width = 0;
	}

	sizer = createOrResizeSizer(grid, sizerRect, sizerDir);

	newDock->sizers[side2] = dock->sizers[side2];	// left or top 
	newDock->sizers[side3] = dock->sizers[side3];	// right or bottom 

	newDock->sizers[side2]->addDock(newDock);
	newDock->sizers[side3]->addDock(newDock);

	if (side == UIDockSide_Top || side == UIDockSide_Left)
	{
		removeDockSide(dock->sizers[side0], dock);	// top or left
		newDock->sizers[side0] = dock->sizers[side0];
		newDock->sizers[side1] = sizer;
		dock->sizers[side0] = sizer;

		newDock->sizers[side0]->addDock(newDock);
	}
	else
	{
		removeDockSide(dock->sizers[side1], dock);	// bottom or right
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
	std::vector<UIDock*> topLeft;	// up/left
	std::vector<UIDock*> bottomRight;	// bellow/right
	std::vector<UIDock*> insideDocks;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void findSurroundingViewsY(NeighborDocks* docks, UIDockSizer* sizer, const UIDock* currentDock) 
{
	const int ty = currentDock->view->rect.y;
	const int tb = ty + currentDock->view->rect.height;

	for (UIDock* dock : sizer->cons)
	{
		if (dock == currentDock)
			continue;

		// Y bounds for the view

		const int cty = dock->view->rect.y;
		const int ctb = cty + dock->view->rect.height;

		if (cty >= ty && ctb <= tb)
			docks->insideDocks.push_back(dock);
		else if (tb < cty)
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

	// Remove the dock from the sizer

	for (auto i = oldSizer->cons.begin(), end = oldSizer->cons.end(); i != end; ++i)
	{
		if (*i == dock)
		{
			oldSizer->cons.erase(i);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void replaceSizers(std::vector<UIDock*>& docks, UIDockSizer* oldSizer, UIDockSizer* newSizer)
{
	for (UIDock* dock : docks)
		replaceSizer(dock, oldSizer, newSizer);
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

bool canSplitSizerY(const UIDock* dock)
{
	const int sx = dock->topSizer->rect.x;
	const int sw = dock->topSizer->rect.width;

	const int dx = dock->view->rect.x;
	const int dw = dock->view->rect.width; 

	if (sx < dx || sw > dw)
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

	(void)grid;

	if (!(dock = findDock(sizer, x, y)))
		return;

	if (sizer->dir == UIDockSizerDir_Vert)
	{
		NeighborDocks closeDocks;

		if (!canSplitSizerY(dock))
			return;

		findSurroundingViewsY(&closeDocks, sizer, dock);

		// The new sizer should be the same height (and y start) as the dock we are splitting it for

		UIDockSizer* newSizer = new UIDockSizer;
		newSizer->rect = sizer->rect;
		newSizer->rect.y = dock->view->rect.y;
		newSizer->rect.height = dock->view->rect.height;
		newSizer->dir = sizer->dir;
		
		// Replace the old sizer with the new sizer for the split

		replaceSizers(closeDocks.insideDocks, sizer, newSizer); 
		replaceSizer(dock, sizer, newSizer); 

		// if topDocks or bottomDocks has no entries it means that we only need to create one 
		// new sizer and just resize the old one. If that is not the case it means that the split
		// was done in the middle of some views and another split is required

		const size_t topDocksCount = closeDocks.topLeft.size(); 
		const size_t bottomDocksCount = closeDocks.bottomRight.size(); 

		if (topDocksCount == 0 || bottomDocksCount == 0)
		{
			if (topDocksCount == 0)
			{
				// If no top docks it mean that the split was made at the top and we need to move the sizer down 
				// of the old sizer and we are done
				
				sizer->rect.y += newSizer->rect.height;
				sizer->rect.height -= newSizer->rect.height;
			}
			else
			{
				// If no bottom docks it mean that the split was made at the bottom and we only need to set a new height
				// of the old sizer and we are done
				
				sizer->rect.height -= newSizer->rect.height;
			}
		}
		else
		{
			// TODO: Implement
		}

		grid->sizers.push_back(newSizer);
	}
	else
	{
	}
}


/*



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The way the delting of a view works is that the code first needs to try to figure out how the surronding views
// should be resized to close the hole the deleted view will casuse.
//
// The priority is this: (Using the same as ProDG)
//
// 1. Try to resize the connecting views left -> right to fill the gap 
// 2. Try to resize the connecting views right -> left to fill the gap 
// 3. Try to resize the connecting views from top -> down to fill the gap
// 4. Try to resize the connecting views from down -> up to fill the gap
//
// In order to resize to the left -> right the bottom and top sizers on the left of the window
// the top and bottom sizers *must* stretch to the left of the current view size
//
// ts _________
//   |   |     |    This is a case where the above delete will work 
//   |---| dv  |
//   |---|     |
// bs|___|_____|
//

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void deleteDock(UIDockingGrid* grid, UIDock* dock)
{
	std::vector<UIDocks*> connectedDocks;

	Rect viewRect = dock->view->rect;

	// We prefer to split resize left -> right (as described above we start with detecting that

	int tx = dock->topSizer.rect.x;
	int bx = dock->bottomRect.rect.x;

	if (tx < viewRect.x && bx < bx)
	{
		// ok we can delete and resize to the left. This means that the views that are connected to the left sizer
		// needs to be set to use the right sizer of the current view
		
		findViewsWithinYLimits(connectedDocks, dock, dock->topSizer.rect.y, dock->bottomSizer.rect.y);

		// Loop over the docks and set the 

		for (UIDock* dock : connectedDocks)
		{


		}


		return;
	}




}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

void UIDock_deleteView(UIDockingGrid* grid, ViewPluginInstance* view)
{
	(void)grid;
	(void)view;
	/*
	for (UIDock* dock : grid->docks)
	{
		if (dock->view != view)
		{
			deleteDock(grid, dock);
			return;
		}
	}
	*/
}


/*

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockStatus UIDock_dockRight(UIDock* dock, ViewPluginInstance* instance)
{

	return UIDockStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockStatus UIDock_dockBottom(UIDock* dock, ViewPluginInstance* instance)
{

	return UIDockStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockStatus UIDock_dockTop(UIDock* dock, ViewPluginInstance* instance)
{

	return UIDockStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_splitHorzUp(UIDock* dock, ViewPluginInstance* instance)
{



}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockStatus UIDock_splitHorzDow(UIDock* dock, ViewPluginInstance* instance)
{

	return UIDockStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockStatus UIDock_splitVertRight(UIDock* dock, ViewPluginInstance* instance)
{

	return UIDockStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockStatus UIDock_splitVertLeft(UIDock* dock, ViewPluginInstance* instance)
{

	return UIDockStatus_ok;
}

*/

