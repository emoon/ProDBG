#pragma once

#include "api/plugin_instance.h"
#include <vector> // TODO: replace with custom arrays

const int g_sizerSize = 4; // TODO: Move to settings

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

struct UIDockSizerGroup;
struct UIDock;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIDockSizer
{
	UIDock* side0;	// up/left
	UIDock* side1;	// down/right
	UIDockSizerGroup* group; // if this is included in a group and being moved we should move the group
	UIDockSizerDir dir;
	Rect rect; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SizerGroups handle the case when we have severnal sizers along the same line connected to more than one view:
//
//
//         s0
// |       |   v1
// |       |--------- s1
// |       |   v2
// |  v0   |--------- s2
// |       |   v3
// |       |--------- s3
//
// In this case we have s0 which are connected to v0,v1,v2,v3 but s0 is actually several sizers becase if a split is
// made horizontally on s1 it's possible to move "s0" independently. Like this:
//
//         s0
// | v4  |     v1
// |-------|--------- s1
// |       |   v2
// |  v0   |--------- s2
// |       |   v3
// |       |--------- s3
//

struct UIDockSizerGroup
{
	std::vector<UIDockSizer*> sizers;
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



