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
#include <stdio.h>
#include <stdlib.h>
#include <foundation/types.h>
#include <foundation/fs.h>
#include <foundation/thread.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_null_base_path(void**)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_null_plugin(void**)
{
    assert_false(PluginHandler_addPlugin("dummyPath", 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_dummy_paths(void**)
{
    assert_false(PluginHandler_addPlugin("dummyPath", "dummy"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_add_plugin(void**)
{
    assert_false(PluginHandler_addPlugin("dummyPath", "dummy"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_add_plugin_true(void**)
{
    int count = 0;

    assert_true(PluginHandler_addPlugin(OBJECT_DIR, "sourcecode_plugin"));
    assert_true(PluginHandler_addPlugin(OBJECT_DIR, "registers_plugin"));

    PluginData** plugins = PluginHandler_getViewPlugins(&count);

    assert_int_equal(count, 2);

    assert_true(PluginHandler_getPluginData(plugins[0]->plugin) == plugins[0]);
    assert_true(PluginHandler_getPluginData(plugins[1]->plugin) == plugins[1]);

    PluginHandler_unloadAllPlugins();

    plugins = PluginHandler_getViewPlugins(&count);

    assert_int_equal(count, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plugin_handler_find_plugin(void**)
{
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

static void test_load_file_ok(void**)
{
    size_t size;

    void* ret = File_loadToMemory("examples/fake_6502/test.bin", &size, 0);

    assert_non_null(ret);
    assert_int_equal(size, 11);

    free(ret);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_load_file_fail(void**)
{
    size_t size;

    void* ret = File_loadToMemory("examples/fake_6502/test_dont_exist.bin", &size, 0);

    assert_null(ret);
    assert_int_equal(size, 0);
}

static int g_intValue = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct IntAddData
{
    int newValue;
    int oldValue;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void doAdd(int value)
{
    IntAddData* addData = (IntAddData*)alloc_zero(sizeof(IntAddData));

    addData->newValue = value;

    Commands_execute(
    {
        addData,

        [](void* userData)
        {
            IntAddData* data = (IntAddData*)userData;
            data->oldValue = g_intValue;
            g_intValue += data->newValue;
        },

        [](void* userData)
        {
            IntAddData* data = (IntAddData*)userData;
            g_intValue = data->oldValue;
        }
    });
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_commands(void**)
{
    Commands_init();

    g_intValue = 0;

    assert_int_equal(g_intValue, 0);

    doAdd(1);

    assert_int_equal(g_intValue, 1);

    doAdd(1);
    doAdd(1);

    assert_int_equal(g_intValue, 3);

    Commands_undo();

    assert_int_equal(g_intValue, 2);

    Commands_undo();
    Commands_undo();

    assert_int_equal(g_intValue, 0);

    Commands_redo();

    assert_int_equal(g_intValue, 1);

    Commands_redo();

    Commands_clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int s_checkPhase = 0;
static const char* s_filename = 0; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fileNotifaction(void* userData, const char* file, int type)
{
	(void)userData;

	if (s_checkPhase == 0)
	{
		assert_string_equal(file, s_filename); 
		assert_int_equal(type, FOUNDATIONEVENT_FILE_CREATED); 
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_file_notification(void**)
{
	const char* test_dir = "t2-output/test_dir"; 
	s_filename = "t2-output/test_dir/test_file";

	fs_remove_directory(test_dir); 
	fs_make_directory(test_dir);

	FileMonitor_addPath(test_dir, "*", fileNotifaction, 0);

	thread_sleep(100);

	FILE* t = fopen(s_filename, "wb");
	fclose(t);

	thread_sleep(500);

	FileMonitor_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
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
    #ifndef _WIN32
        unit_test(test_file_notification),
    #endif
    };

    return run_tests(tests);
}


