#include <stdlib.h>
#include <core/Core.h>
#include <core/PluginHandler.h>
//#include "ui/Application.h"

#ifdef PRODBG_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <bgfx.h>
#include <entry.h>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* s_plugins[] =
{
	"SourceCodePlugin", 
	"CallStackPlugin", 
	"Registers", 
	"Locals", 
	"Disassembly", 
#ifdef PRODBG_MAC
	"LLDBPlugin",
#endif
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int realMain(int argc, const char** argv)
{
	(void)argc;
	(void)argv;

	for (uint32_t i = 0; i < sizeof_array(s_plugins); ++i)
	{
		if (!PluginHandler_addPlugin(OBJECT_DIR, s_plugins[i]))
			return 0;
	}

	uint32_t width = 1280;
	uint32_t height = 720;
	uint32_t debug = BGFX_DEBUG_TEXT;
	uint32_t reset = BGFX_RESET_VSYNC;

	bgfx::init();
	bgfx::reset(width, height, reset);

	// Enable debug text.
	bgfx::setDebug(debug);

	// Set view 0 clear state.
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR_BIT|BGFX_CLEAR_DEPTH_BIT
		, 0x303030ff
		, 1.0f
		, 0
		);

	while (!entry::processEvents(width, height, debug, reset) )
	{
		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, (uint16_t)width, (uint16_t)height);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::submit(0);

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/00-helloworld");
		bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Initialization and debug text.");

		// Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
	}

	// Shutdown bgfx.
	bgfx::shutdown();
	

	return 0; //Application_init(argc, argv);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

extern "C"
{

int _main_(int argc, char** argv)
{
	return prodbg::realMain(argc, (const char**)argv);
}

}

/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PRODBG_WIN

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return prodbg::main(__argc, (const char**)__argv);
}

#else

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
	return prodbg::main(argc, argv);
}
*/

