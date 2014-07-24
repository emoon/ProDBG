#include <stdlib.h>
#include <core/Core.h>
#include <core/PluginHandler.h>
#include <ui/UISystem.h>
#include <Arika/Arika.h>

#ifdef PRODBG_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* s_plugins[] =
{
	"Registers", 
	// "LLDBPlugin",
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	for (uint32_t i = 0; i < sizeof_array(s_plugins); ++i)
	{
		if (!PluginHandler_addPlugin(OBJECT_DIR, s_plugins[i]))
			return 0;
	}

	if (!UISystem_initArika(OBJECT_DIR, "arika-qt"))
		return 0;

	ARFuncs* arika = UISystem_getArFuncs();

	ARWidget* widget = arika->window_create_main();
	//arika->widget_set_title(widget, "ProDBG 0.1");
	arika->widget_set_width(widget, 800);
	arika->widget_set_height(widget, 600);

	for (;;)
	{
		if (!arika->update())
			break;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PRODBG_WIN

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return prodbg::main(__argc, __argv);
}

#else

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	return prodbg::main(argc, argv);
}

#endif
