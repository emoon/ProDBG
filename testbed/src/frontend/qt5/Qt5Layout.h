#pragma once

namespace prodbg
{

enum Qt5ViewType
{
	Qt5ViewType_Init = 0x0000,
	Qt5ViewType_Reset = 0x0001,
	Qt5ViewType_Dock = 0x0002,
	Qt5ViewType_Main = 0x0003,
	Qt5ViewType_Dynamic = 0x0004,

	Qt5ViewType_PluginStart = 0x0020,

	// TODO: Temp
	Qt5ViewType_CallStack,
	Qt5ViewType_Locals,
	Qt5ViewType_SourceCode,
	Qt5ViewType_HexEdit,
	Qt5ViewType_DebugOutput
};

class Qt5LayoutEntry
{
public:
    Qt5LayoutEntry()
	{
	};
};

class Qt5Layout
{
public:
	int m_numEntries;
	Qt5LayoutEntry* m_entries;
};

}
