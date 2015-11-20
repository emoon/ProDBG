#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "core/alloc.h"
#include "core/commands.h"
#include "core/core.h"
#include "core/file.h"
#include "core/file_monitor.h"
#include "core/log.h"
#include "core/plugin_handler.h"
#include "core/settings.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main() {
    log_set_level(LOG_NONE);
    Core_init();

    const UnitTest tests[] =
    {
        unit_test(plugin_handler_null_base_path),
        unit_test(plugin_handler_null_plugin),
        unit_test(plugin_handler_dummy_paths),
        unit_test(plugin_handler_add_plugin),
        unit_test(plugin_handler_add_plugin_true),
        unit_test(plugin_handler_find_plugin),
        unit_test(test_load_file_ok),
        unit_test(test_load_file_fail),
        unit_test(test_commands),
        unit_test(test_file_notification),
        unit_test(test_settings),
    };

    return run_tests(tests);
}




