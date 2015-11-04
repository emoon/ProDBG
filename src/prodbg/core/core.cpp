#include "core.h"

#ifdef PRODBG_MAC
#include <foundation/apple.h>
#endif

#include <foundation/foundation.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Core_init() {
    static application_t application;

    application.name = "ProDBG";
    application.short_name = "ProDBG";
    application.config_dir = "ProDBG";
    application.version = foundation_version();
    application.flags = APPLICATION_UTILITY;
    application.dump_callback = 0;

    foundation_initialize(memory_system_malloc(), application);
}

