#pragma once

#include "api/plugin_instance.h"

#include <vector> // TODO: replace with custom arrays

const int g_sizerSize = 4; // TODO: Move to settings
const int g_sizerSnapSize = 8; // TODO: Move to settings

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockType
{
    UIDockType_Docked,
    UIDockType_Floating,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockSide
{
    UIDockSide_Top,
    UIDockSide_Bottom,
    UIDockSide_Right,
    UIDockSide_Left,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDock;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDockSizer
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    inline void addDock(UIDock* dock)
    {
        cons.push_back(dock);
    }

    std::vector<UIDock*> cons;  // connected docks
    std::vector<int> dockIds; // TODO: used during loading, move?
    UIDockSizerDir dir;
    FloatRect rect;
    int id;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDock
{
    inline UIDock(ViewPluginInstance* inView) :
        topSizer(0), bottomSizer(0), rightSizer(0), leftSizer(0), view(inView), type(UIDockType_Docked), id(-1)
    {
    }

    enum
    {
        Top,
        Bottom,
        Right,
        Left,
        Sizers_Count,
    };

    union
    {
        struct
        {
            UIDockSizer* topSizer;
            UIDockSizer* bottomSizer;
            UIDockSizer* rightSizer;
            UIDockSizer* leftSizer;
        };

        UIDockSizer* sizers[Sizers_Count];
    };

    ViewPluginInstance* view;

    UIDockType type;
    int id;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UIDockState
{
    UIDockState_None,
    UIDockState_HoverSizer,
    UIDockState_DragSizer,
    UIDockState_BeginDragView,
    UIDockState_DraggingView,
    UIDockState_EndDragView,
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
    UIDockingGrid() : state(UIDockState_None)
    {
    	prevDragPos = { 0.0f, 0.0f };
    }

    std::vector<UIDock*> docks;
    std::vector<UIDockSizer*> sizers;
    UIDockSizer topSizer;
    UIDockSizer bottomSizer;
    UIDockSizer rightSizer;
    UIDockSizer leftSizer;
    FloatRect rect;

    UIDockState state;
    Vec2 prevDragPos;
    std::vector<UIDockSizer*> hoverSizers;
};



