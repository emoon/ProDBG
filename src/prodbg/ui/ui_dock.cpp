#include "ui_dock.h"
#include "core/alloc.h"
#include "core/math.h"
#include "ui_dock_private.h"

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

static void dockSide(UIDockSide side, UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* instance)
{
	UIDock* newDock = new UIDock(instance); 
	UIDockSizer* sizer = new UIDockSizer; 

	Rect rect = dock->view->rect;

	switch (side)
	{
		case UIDockSide_Top:
		case UIDockSide_Bottom:
		{
			rect.height /= 2;

			if (side == UIDockSide_Top)
				dock->view->rect.y += rect.height;
			else
				rect.y += rect.height;

			rect.height = int_min(rect.height - g_sizerSize, 0);
			calcHorizonalSizerSize(sizer, &rect);

			newDock->leftSizer = dock->leftSizer;
			newDock->rightSizer = dock->rightSizer;

			newDock->leftSizer->addDock(newDock);
			newDock->rightSizer->addDock(newDock);

			if (side == UIDockSide_Top)
			{
				bool r = removeDockSide(dock->topSizer, dock);
				printf("removed %d\n", r);

				newDock->topSizer = dock->topSizer;
				newDock->bottomSizer = sizer;
				dock->topSizer = sizer;

				newDock->topSizer->addDock(newDock);
			}
			else
			{
				bool r = removeDockSide(dock->bottomSizer, dock);
				printf("removed %d\n", r);

				newDock->bottomSizer = dock->bottomSizer;
				newDock->topSizer = sizer;
				dock->bottomSizer = sizer;

				newDock->bottomSizer->addDock(newDock);
			}

			sizer->addDock(dock); 
			sizer->addDock(newDock); 

			break;
		}

		case UIDockSide_Right:
		case UIDockSide_Left:
		{
			rect.width /= 2;

			// if we connect on the left side we need to move the current view to the right
			// otherwise we move the new view to the side

			dock->view->rect.width = rect.width;

			if (side == UIDockSide_Left)
			{
				dock->view->rect.x += rect.width;
				rect.width = int_min(rect.width - g_sizerSize, 0);
			}
			else
			{
				rect.x += rect.width;
				dock->view->rect.x = int_min(dock->view->rect.x - g_sizerSize, 0);
				rect.width = int_min(rect.width - g_sizerSize, 0);
			}

			calcVerticalSizerSize(sizer, &rect);

			newDock->topSizer = dock->topSizer;
			newDock->bottomSizer = dock->bottomSizer;

			newDock->topSizer->addDock(newDock);
			newDock->bottomSizer->addDock(newDock);

			if (side == UIDockSide_Left)
			{
				bool r = removeDockSide(dock->leftSizer, dock);
				printf("removed %d\n", r);

				newDock->leftSizer = dock->leftSizer;
				newDock->rightSizer = sizer;
				dock->leftSizer = sizer;

				newDock->leftSizer->addDock(newDock);
			}
			else
			{
				bool r = removeDockSide(dock->rightSizer, dock);
				printf("removed %d\n", r);

				newDock->rightSizer = dock->rightSizer;
				newDock->leftSizer = sizer;
				dock->rightSizer = sizer;

				newDock->rightSizer->addDock(newDock);
			}

			sizer->addDock(dock); 
			sizer->addDock(newDock); 

			break;
		}
	}

	newDock->view->rect = rect;

	grid->sizers.push_back(sizer);
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

static void deleteDock(UIDockingGrid* grid, UIDock* dock)
{
	// We prefer to split the whole horizontal

	const size_t topCount = dock->topSizer->cons.size();
	const size_t bottomCount = dock->bottomSizer->cons.size();
	const size_t rightCount = dock->rightSizer->cons.size();
	const size_t leftCount = dock->leftSizer->cons.size();

	(void)topCount;
	(void)bottomCount;
	(void)rightCount;
	(void)leftCount;

	(void)grid;
	(void)dock;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_deleteView(UIDockingGrid* grid, ViewPluginInstance* view)
{
	for (UIDock* dock : grid->docks)
	{
		if (dock->view != view)
		{
			deleteDock(grid, dock);
			return;
		}
	}
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

