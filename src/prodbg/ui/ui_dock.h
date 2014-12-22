#pragma once

struct ViewPluginInstance;
struct UIDockingGrid;
struct UIDock;
struct Rect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_createGrid(Rect* rect);

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


