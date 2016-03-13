#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#if defined(PRODBG_MAC)
#define PRODBG_USING_DOCKING 1
//#else
//#define PRODBG_USING_DOCKING 0
//#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ViewPluginInstance;
struct UIDockSizer;
struct UIDockingGrid;
struct UIDock;
struct IntRect;
struct Vec2;
struct InputState;
struct json_t;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockSizerDir {
    UIDockSizerDir_None = 0,
    UIDockSizerDir_Horz = 1 << 0,
    UIDockSizerDir_Vert = 1 << 1,
    UIDockSizerDir_Both = UIDockSizerDir_Horz | UIDockSizerDir_Vert,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_createGrid(IntRect* rect);
void UIDock_destroyGrid(UIDockingGrid* grid);

UIDock* UIDock_addView(UIDockingGrid* grid, ViewPluginInstance* view);
bool UIDock_deleteView(UIDockingGrid* grid, ViewPluginInstance* view);

void UIDock_dockTop(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);
void UIDock_dockBottom(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);
void UIDock_dockLeft(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);
void UIDock_dockRight(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* view);

UIDockSizerDir UIDock_isHoveringSizer(UIDockingGrid* grid, const Vec2* pos);

void UIDock_splitSizer(UIDockingGrid* grid, UIDockSizer* sizer, int x, int y);
void UIDock_mergeSizers(UIDockingGrid* grid, UIDockSizer* sizer);

void UIDock_dragSizer(UIDockingGrid* grid, void* handle, Vec2* deltaMove);

void UIDock_update(UIDockingGrid* grid, const InputState* state);
void UIDock_updateSize(UIDockingGrid* grid, int width, int height);

UIDockSizerDir UIDock_getSizingState(UIDockingGrid* grid);

UIDock* UIDock_getDockAt(UIDockingGrid* grid, int x, int y);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_splitHorizontalAt(UIDockingGrid* grid, int x, int y, ViewPluginInstance* newInstance);
void UIDock_splitVerticalAt(UIDockingGrid* grid, int x, int y, ViewPluginInstance* newInstance);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIDock_splitHorizontal(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* newInstance);
void UIDock_splitVertical(UIDockingGrid* grid, UIDock* dock, ViewPluginInstance* newInstance);
void UIDock_render(UIDockingGrid* grid);

bool UIDock_saveLayout(UIDockingGrid* grid, const char* filename);
bool UIDock_saveLayoutJson(UIDockingGrid* grid, json_t* jsonObject);

UIDockingGrid* UIDock_loadLayout(const char* filename, int width, int height);
UIDockingGrid* UIDock_loadLayoutJson(json_t* jsonObject, int width, int height);


//void UIDock_dockLeft(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockRight(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockBottom(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_dockTop(UIDock* dock, ViewPluginInstance* instance);

//void UIDock_splitHorzUp(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitHorzDow(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitVertRight(UIDock* dock, ViewPluginInstance* instance);
//UIDockStatus UIDock_splitVertLeft(UIDock* dock, ViewPluginInstance* instance);


