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

	dock->view = view;
	dock->topSizer = &grid->topSizer;
	dock->bottomSizer = &grid->bottomSizer;
	dock->rightSizer = &grid->rightSizer;
	dock->leftSizer = &grid->leftSizer;

	// Add the dock to the sizers

	grid->topSizer.side1.push_back(dock); 
	grid->bottomSizer.side0.push_back(dock); 
	grid->rightSizer.side0.push_back(dock); 
	grid->leftSizer.side1.push_back(dock); 

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

			if (side == UIDockSide_Top)
			{
				newDock->topSizer = dock->topSizer;
				newDock->bottomSizer = sizer;
				dock->topSizer = sizer;

				sizer->side0.push_back(newDock); 
				sizer->side1.push_back(dock); 
			}
			else
			{
				newDock->bottomSizer = dock->bottomSizer;
				newDock->topSizer = sizer;
				dock->bottomSizer = sizer;

				sizer->side0.push_back(dock); 
				sizer->side1.push_back(newDock); 
			}

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

			if (side == UIDockSide_Left)
			{
				newDock->leftSizer = dock->leftSizer;
				newDock->rightSizer = sizer;
				dock->leftSizer = sizer;

				sizer->side0.push_back(newDock); 
				sizer->side1.push_back(dock); 
			}
			else
			{
				newDock->rightSizer = dock->rightSizer;
				newDock->leftSizer = sizer;
				dock->rightSizer = sizer;

				sizer->side0.push_back(dock); 
				sizer->side1.push_back(newDock); 
			}

			// Add new dock to top/bottom sizer as both views now share it.

			dock->bottomSizer->side0.push_back(newDock);
			dock->topSizer->side1.push_back(newDock);

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

