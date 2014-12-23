#include "ui_dock.h"
#include "core/alloc.h"
#include "api/plugin_instance.h"
#include <vector> // TODO: replace with custom arrays

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockSizerDir
{
	UIDockSizerDir_Horz,
	UIDockSizerDir_Vert,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockType
{
	UIDockType_Docked,
	UIDockType_Floating,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDockSizer
{
	ViewPluginInstance* side0;	// up/left
	ViewPluginInstance* side1;	// down/right
	UIDockSizerDir dir;
	Rect rect; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDock
{
	UIDockSizer* topSizer;
	UIDockSizer* bottomSizer;
	UIDockSizer* rightSizer;
	UIDockSizer* leftSizer;
	ViewPluginInstance* view;

	UIDockType type;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockStatus
{
	UIDockStatus_ok,
	UIDockStatus_fail,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDockingGrid
{
	std::vector<UIDock*> docks;
	std::vector<UIDockSizer*> sizers;
	Rect rect;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int g_sizerSize = 4; // TODO: Move to settings
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
	UIDock* dock = (UIDock*)alloc_zero(sizeof(UIDock)); 

	dock->view = view;

	// If this is the first we we just set it as maximized

	if (grid->docks.size() == 0)
		dock->view->rect = grid->rect;

	// TODO: How should we really handle this case?

	grid->docks.push_back(dock);

	return dock;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Rect calcVerticalSizerSize(const Rect* rect)
{
	Rect sizerRect;

	sizerRect.x = rect->x; 
	sizerRect.y = rect->y; 
	sizerRect.width = g_sizerSize;
	sizerRect.height = rect->height;

	return sizerRect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_dockLeft(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* instance)
{
	UIDock* newDock = (UIDock*)alloc_zero(sizeof(UIDock));

	newDock->view = instance;
	Rect rect = dock->view->rect;

	// Push current window forward 

	dock->view->rect.y += rect.width;
	dock->view->rect.width = rect.width;

	// Setup the rect for the new dock

	rect.width = int_min(rect.width - g_sizerSize, 0);
	newDock->view->rect = rect;
	
	// Check if the window that we dock to has any sizer of the left size
	// and if it doesn't (already on the left edge of the grid) we need
	// to create a new one

	UIDockSizer* sizer = (UIDockSizer*)alloc_zero(sizeof(UIDockSizer));
	Rect sizerRect = calcVerticalSizerSize(&rect);

	sizer->side0 = newDock->view; 
	sizer->side1 = dock->view; 
	sizer->dir = UIDockSizerDir_Vert;
	sizer->rect = sizerRect;

	if (!dock->leftSizer)
	{
		// Attach the sizer

		dock->leftSizer = sizer;
		newDock->rightSizer = sizer;
	}
	else
	{
		// If there already is a sizer we are getting connected between two windows
		//
		// |------|------|
		// | Left | Old  |
		// | View | View |
		// |------|------|
		//
		// -->
		// -->
		//        os      ns
		// |------|-------|------|
		// | Left | New   | Old  |
		// | View | View  | View |
		// |------|-------|------|
		//
		// Is what the result should be and the old view is what gets split

		dock->leftSizer->side1 = instance;
	}

	grid->sizers.push_back(sizer);
	                                
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

