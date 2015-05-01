#include "core.h"

#ifdef PRODBG_MAC
#include <foundation/apple.h>
#endif

#include <foundation/foundation.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//extern "C" void foundation_hack_environment_main_args(int argc, const char* const* argv);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Core_init()
{
	static application_t application;

	application.name = "ProDBG";
	application.short_name = "ProDBG";
	application.config_dir = "ProDBG";
	application.version = foundation_version();
	application.flags = APPLICATION_UTILITY;
	application.dump_callback = 0;

	// static const char* const temp[] = { "temp" };

	//foundation_hack_environment_main_args(1, temp);

	foundation_initialize(memory_system_malloc(), application);
}
 
