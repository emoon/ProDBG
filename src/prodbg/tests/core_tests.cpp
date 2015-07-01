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
#include "pd_keys.h"
#include <stdio.h>
#include <stdlib.h>
#include <foundation/types.h>
#include <foundation/fs.h>
#include <foundation/thread.h>
#include <foundation/path.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void plugin_handler_null_base_path(void**)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void plugin_handler_null_plugin(void**)
{
    assert_false(PluginHandler_addPlugin("dummyPath", 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void plugin_handler_dummy_paths(void**)
{
    assert_false(PluginHandler_addPlugin("dummyPath", "dummy"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void plugin_handler_add_plugin(void**)
{
    assert_false(PluginHandler_addPlugin("dummyPath", "dummy"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void plugin_handler_add_plugin_true(void**)
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

void plugin_handler_find_plugin(void**)
{
    assert_null(PluginHandler_findPlugin(0, "dummyFile", "dummyName", false));
    assert_null(PluginHandler_findPlugin(0, "dummyFile", "dummyName", true));
    assert_null(PluginHandler_findPlugin(0, "sourcecode_plugin", "Source Code View", false));
    assert_non_null(PluginHandler_findPlugin(0, "sourcecode_plugin", "Source Code View", true));
    assert_non_null(PluginHandler_findPlugin(0, "sourcecode_plugin", "Source Code View", false));

    assert_true(PluginHandler_addPlugin(OBJECT_DIR, "registers_plugin"));

    assert_non_null(PluginHandler_findPlugin(0, "registers_plugin", "Registers View", true));

    assert_non_null(PluginHandler_findPluginByFilename("registers_plugin"));
    assert_null(PluginHandler_findPluginByFilename("plugin_that_isnt_here"));

    PluginHandler_unloadAllPlugins();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_load_file_ok(void**)
{
    size_t size;

    void* ret = File_loadToMemory("examples/fake_6502/test.bin", &size, 0);

    assert_non_null(ret);
    assert_int_equal(size, 11);

    free(ret);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_load_file_fail(void**)
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
static const char* s_filename_2 = 0;
static const char* s_filename_3 = 0;
static const char* s_filename_4 = 0;
static const char* s_filename_5 = 0;
static int s_userData_1 = 0;
static int s_userData_2 = 1;
static int s_userData_3 = 2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fileNotifaction(void* userData, const char* file, int type)
{
    assert_true(userData == &s_userData_1);

    if (s_checkPhase == 0)
    {
        assert_string_equal(file, s_filename);
        assert_int_equal(type, FOUNDATIONEVENT_FILE_CREATED);
        s_checkPhase = 1;
        return;
    }

    if (s_checkPhase == 1)
    {
        assert_string_equal(file, s_filename);
        assert_int_equal(type, FOUNDATIONEVENT_FILE_MODIFIED);
        s_checkPhase = 2;
        return;
    }

    s_checkPhase = 7;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fileNotifaction2(void* userData, const char* file, int type)
{
    assert_true(userData == &s_userData_2);

    if (s_checkPhase == 2)
    {
        assert_string_equal(file, s_filename_2);
        assert_int_equal(type, FOUNDATIONEVENT_FILE_CREATED);
        s_checkPhase = 3;
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fileNotifaction3(void* userData, const char* file, int type)
{
    assert_true(userData == &s_userData_3);

    if (s_checkPhase == 3 && strcmp(file, s_filename_3) == 0)
    {
        assert_int_equal(type, FOUNDATIONEVENT_FILE_CREATED);
        s_checkPhase = 4;
        return;
    }

    if (s_checkPhase == 4 && strcmp(file, s_filename_4) == 0)
    {
        assert_int_equal(type, FOUNDATIONEVENT_FILE_CREATED);
        s_checkPhase = 5;
        return;
    }

    if (s_checkPhase == 5  && strcmp(file, s_filename_5) == 0)
    {
        assert_int_equal(type, FOUNDATIONEVENT_FILE_CREATED);
        s_checkPhase = 6;
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void test_file_notification(void**)
{
    int temp = 1;
    const char* test_dir = "t2-output/test_dir";
    const char* test_dir_2 = "t2-output/2_test_dir";
    const char* test_dir_3 = "t2-output/3_test_dir";
    const char* filename_2 = "t2-output/test_dir/test_file_2";
    s_filename = "t2-output/test_dir/test_file.dll";
    s_filename_2 = "t2-output/2_test_dir/test_file.txt";

    s_filename_3 = "t2-output/3_test_dir/test_file.bin";
    s_filename_4 = "t2-output/3_test_dir/test_file.so";
    s_filename_5 = "t2-output/3_test_dir/test_file.exe";

    fs_remove_directory(test_dir);
    fs_make_directory(test_dir);

    fs_remove_directory(test_dir_2);
    fs_make_directory(test_dir_2);

    fs_remove_directory(test_dir_3);
    fs_make_directory(test_dir_3);

    FILE* t = fopen(filename_2, "wb");
    fwrite(&temp, 4, 1, t);
    fclose(t);

    FileMonitor_addPath(test_dir, "*", fileNotifaction, &s_userData_1);

    thread_sleep(200);

    // Test notification when writing one file

    t = fopen(s_filename, "wb");
    fclose(t);

    thread_sleep(1000);

    FileMonitor_update();

    thread_sleep(400);

    assert_int_equal(s_checkPhase, 1);

    fs_copy_file(filename_2, s_filename);

    thread_sleep(1000);

    FileMonitor_update();

    thread_sleep(400);

    assert_int_equal(s_checkPhase, 2);

    FileMonitor_addPath(test_dir_2, "txt", fileNotifaction2, &s_userData_2);

    thread_sleep(1000);

    fs_copy_file(filename_2, s_filename_2);

    thread_sleep(1200);

    FileMonitor_update();

    assert_int_equal(s_checkPhase, 3);


    FileMonitor_removePath(test_dir);

    // Except no notifactions for this

    fs_remove_file(s_filename);

    FileMonitor_update();

    thread_sleep(1000);

    FileMonitor_update();

    thread_sleep(400);

    assert_int_equal(s_checkPhase, 3);


    FileMonitor_addPath(test_dir_3, "bin;so", fileNotifaction3, &s_userData_3);

    thread_sleep(1000);

    fs_copy_file(filename_2, s_filename_3);

    thread_sleep(1200);

    FileMonitor_update();

    assert_int_equal(s_checkPhase, 4);

    thread_sleep(1000);

    fs_copy_file(filename_2, s_filename_4);

    thread_sleep(1200);

    FileMonitor_update();

    assert_int_equal(s_checkPhase, 5);

    thread_sleep(1000);

    fs_copy_file(filename_2, s_filename_5);

    thread_sleep(1200);

    FileMonitor_update();

    assert_int_equal(s_checkPhase, 5);

    FileMonitor_close();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_settings(void**)
{
    assert_true(Settings_loadSettings("test_data/settings.json"));

    assert_string_equal(Settings_getString("default_native_backend", "mac"), "LLDB");
    assert_string_equal(Settings_getString("default_native_backend", "windows"), "Microsoft Debugger Engine");
    assert_int_equal(Settings_getInt("window_size", "width"), 1024);
    assert_true((Settings_getReal("window_size", "scale") - 2.2) < 0.001f);

    assert_true(Settings_loadSettings("test_data/user_settings.json"));

    assert_string_equal(Settings_getString("default_native_backend", "mac"), "gdb");
    assert_string_equal(Settings_getString("default_native_backend", "windows"), "windbg");
    assert_int_equal(Settings_getInt("window_size", "width"), 1280);
    assert_true((Settings_getReal("window_size", "scale") - 4.4) < 0.001f);

    assert_int_equal(Settings_getInt("window_size", "not_existing_tag"), 0);
    assert_null(Settings_getString("window_size", "not_existing_tag"));
    assert_true(Settings_getReal("window_size", "not_existing_tag") < 0.0001f);

    assert_false(Settings_loadSettings("test_data/user_settings_2.json"));

    uint32_t key = Settings_getShortcut("Source Code View", "fast_open");
    assert_int_equal((key & 0xf), PDKEY_CTRL);
    assert_int_equal((key >> 4), 'o');

    key = Settings_getShortcut("Source Code View", "open_file");
#ifdef PRODBG_MAC
    assert_int_equal((key & 0xf), PDKEY_ALT | PDKEY_SUPER);
#else
    assert_int_equal((key & 0xf), PDKEY_CTRL | PDKEY_ALT);
#endif
    assert_int_equal((key >> 4), 'o');

    key = Settings_getShortcut("Source Code View", "funky");
#ifdef PRODBG_WIN
    assert_int_equal((key & 0xf), PDKEY_CTRL);
    assert_int_equal((key >> 4), 'w');
#else
    assert_int_equal((key & 0xf), PDKEY_CTRL);
    assert_int_equal((key >> 4), 'o');
#endif

    key = Settings_getShortcut("Source Code View", "multi_combo");
    assert_int_equal((key & 0xf), PDKEY_CTRL | PDKEY_ALT | PDKEY_SHIFT | PDKEY_SUPER);
    assert_int_equal((key >> 4), PDKEY_ESCAPE);

    key = Settings_getShortcut("Source Code View", "f_key");
    assert_int_equal((key & 0xf), 0); 
    assert_int_equal((key >> 4), PDKEY_F4);

    Settings_destroy();
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
        unit_test(test_file_notification),
        unit_test(test_settings),
    };

    return run_tests(tests);
}


