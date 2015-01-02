#pragma once

struct ViewPluginInstance;
struct UIDockSizer;
struct UIDockingGrid;
struct UIDock;
struct Rect;
struct Vec2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockSizerDir
{
	UIDockSizerDir_None,
	UIDockSizerDir_Horz,
	UIDockSizerDir_Vert,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_createGrid(Rect* rect);

UIDock* UIDock_addView(UIDockingGrid* grid, ViewPluginInstance* view);
void UIDock_deleteView(UIDockingGrid* grid, ViewPluginInstance* view);

void UIDock_dockTop(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);
void UIDock_dockBottom(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);
void UIDock_dockLeft(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);
void UIDock_dockRight(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);


UIDockSizerDir UIDock_isHoveringSizer(UIDockingGrid* grid, Vec2* pos);

void UIDock_splitSizer(UIDockingGrid* grid, UIDockSizer* sizer, int x, int y);
void UIDock_mergeSizers(UIDockingGrid* grid, UIDockSizer* sizer);


//void UIDock_dockLeft(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockRight(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockBottom(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockTop(UIDock* dock, ViewPluginInstance* instance);

//void UIDock_splitHorzUp(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitHorzDow(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitVertRight(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitVertLeft(UIDock* dock, ViewPluginInstance* instance);


