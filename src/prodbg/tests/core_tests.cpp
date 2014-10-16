#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "core/plugin_handler.h"
#include "core/log.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_null_base_path(void** state)
{
    (void)state;
    assert_false(PluginHandler_addPlugin(0, "dummy"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_null_plugin(void** state)
{
    (void)state;
    assert_false(PluginHandler_addPlugin("dummyPath", 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_dummy_paths(void** state)
{
    (void)state;
    assert_false(PluginHandler_addPlugin("dummyPath", "dummy"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_add_plugin(void** state)
{
    (void)state;
    assert_false(PluginHandler_addPlugin("dummyPath", "dummy"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_add_plugin_true(void** state)
{
    int count = 0;

    (void)state;

    assert_true(PluginHandler_addPlugin(OBJECT_DIR, "sourcecode_plugin"));
    assert_true(PluginHandler_addPlugin(OBJECT_DIR, "registers_plugin"));

    PluginData** plugins = PluginHandler_getPlugins(&count);

    assert_int_equal(count, 2);

    assert_true(PluginHandler_getPluginData(plugins[0]->plugin) == plugins[0]);
    assert_true(PluginHandler_getPluginData(plugins[1]->plugin) == plugins[1]);

    PluginHandler_unloadAllPlugins();

    plugins = PluginHandler_getPlugins(&count);

    assert_int_equal(count, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_find_plugin(void** state)
{
    (void)state;

    assert_null(PluginHandler_findPlugin(0, "dummyFile", "dummyName", false));
    assert_null(PluginHandler_findPlugin(0, "dummyFile", "dummyName", true));
    assert_null(PluginHandler_findPlugin(0, "sourcecode_plugin", "Source Code View", false));
    assert_non_null(PluginHandler_findPlugin(0, "sourcecode_plugin", "Source Code View", true));
    assert_non_null(PluginHandler_findPlugin(0, "sourcecode_plugin", "Source Code View", false));

    assert_true(PluginHandler_addPlugin(OBJECT_DIR, "registers_plugin"));

    assert_non_null(PluginHandler_findPlugin(0, "registers_plugin", "Registers View", true));

    PluginHandler_unloadAllPlugins();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    log_set_level(LOG_NONE);

    const UnitTest tests[] =
    {
        unit_test(plugin_handler_null_base_path),
        unit_test(plugin_handler_null_plugin),
        unit_test(plugin_handler_dummy_paths),
        unit_test(plugin_handler_add_plugin),
        unit_test(plugin_handler_add_plugin_true),
        unit_test(plugin_handler_find_plugin),
    };

    return run_tests(tests);
}


