#pragma once

#include <core/Core.h>
#include <qt5/Qt5Settings.h>

#include <QSplitter>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum Qt5ViewType
{
	Qt5ViewType_Init = 0x0000,
	Qt5ViewType_Reset = 0x0001,
	Qt5ViewType_Dock = 0x0002,
	Qt5ViewType_Main = 0x0003,
	Qt5ViewType_Dynamic = 0x0004,

	Qt5ViewType_PluginStart = 0x0020,

	// \todo: Temp
	Qt5ViewType_CallStack,
	Qt5ViewType_Locals,
	Qt5ViewType_SourceCode,
	Qt5ViewType_HexEdit,
	Qt5ViewType_DebugOutput,
	Qt5ViewType_Registers
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Qt5LayoutEntry
{
	///! Id number of layout entry
	int32 entryId;

	///! Global X position
	int32 positionX;

	///! Global Y position
	int32 positionY;

	///! Width of view
	int32 sizeX;

	///! Height of view
	int32 sizeY;

	///! If number of parent (0 if main window)
	int32 parentId;

	///! View tyoe of layout entry
	Qt5ViewType viewType;

	// Main Window Parameters
	
	///! Flag for whether or not the main window is maximized
	bool isMaximized;

	///! Binary blob for the main window state
	QByteArray* mainWindowState;

	// Dynamic View Parameters
	
	///! Splitter orientation
	Qt::Orientation splitDirection;
	
	///! Size of splitter region 1
	int32 splitRegion1Size;

	///! Size of splitter region 2
	int32 splitRegion2Size;

	///! Id number of child 1 (0 if none)
	int32 child1;

	///! Id number of child 2 (0 if none)
	int32 child2;

	///! Global X position of dock
	int32 dockPositionX;

	///! Global Y position of dock
	int32 dockPositionY;

	///! Width of dock
	int32 dockSizeX;

	///! Height of dock
	int32 dockSizeY;

	///! Is docked or floating
	bool isFloating;

	///! Whether dynamic view has a splitter
	bool hasSplitter;

	///! Whether the dynamic view is filling the main window (not a dock widget)
	bool fillMainWindow;

	///! Whether this is a top level dynamic view (main window as its parent)
	bool topLevel;

	// Miscellaneous
	
	///! View specific extended data
	Qt5SettingArgument extendedData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Qt5Layout
{
	int32 entryCount;
	Qt5LayoutEntry* entries;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
