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

	Qt5ViewType_CodeEditor = 0x0005
};

}
