#pragma once

#include "core/math.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ViewPluginInstance;

struct UIDockView
{
	ViewPluginInstance* viewInstance;
	Rect rect; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockSizerDir
{
	UIDockSizerDir_Horz,
	UIDockSizerDir_Vert,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDockSizer
{
	UIDockView* side0;	// up/left
	UIDockView* side1;	// down/right
	UIDockSizerDir dir;
	Rect rect; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDock
{
	struct UIDockSizer* topSizer;
	struct UIDockSizer* bottomSizer;
	struct UIDockSizer* rightSizer;
	struct UIDockSizer* leftSizer;
	struct UIDockView currentView;
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
	UIDock* docks;
	UIDockSizer* sizers;
	Rect rect;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_createGrid(Rect rect);

UIDock* UIDock_addView(UIDockingGrid* grid, ViewPluginInstance* view);

void UIDock_dockLeft(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);

//void UIDock_dockLeft(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockRight(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockBottom(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockTop(UIDock* dock, ViewPluginInstance* instance);

//void UIDock_splitHorzUp(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitHorzDow(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitVertRight(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitVertLeft(UIDock* dock, ViewPluginInstance* instance);


