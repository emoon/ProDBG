#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <jansson.h>

#include "pd_view.h"
#include "core/core.h"
#include "session/session.h"
#include "core/plugin_io.h"
#include "api/plugin_instance.h"
#include "core/plugin_handler.h"
#include "ui/plugin.h"
#include "ui/bgfx/bgfx_plugin_ui.h"
#include <foundation/array.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* dummyCreateInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc) {
    (void)uiFuncs;
    (void)serviceFunc;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummyDestroyInstance(void* userData) {
    (void)userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int dummyUpdate(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents) {
    (void)userData;
    (void)uiFuncs;
    (void)inEvents;
    (void)outEvents;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int dummySaveState(void* userData, struct PDSaveState* saveState) {
    (void)userData;

    PDIO_write_int(saveState, -1231);
    PDIO_write_int(saveState, 1);
    PDIO_write_int(saveState, 1231);
    PDIO_write_double(saveState, 3.1415);
    PDIO_write_double(saveState, 8.0);
    PDIO_write_string(saveState, "stoehus");
    PDIO_write_string(saveState, "temp0");
    PDIO_write_string(saveState, "semp1");

    PDIO_write_string(saveState, "longlongseothuseothuseothstuhsntoehusnteohustnoehunstoehusneothusneothsohustoehus");

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int dummyLoadState(void* userData, struct PDLoadState* load_state) {
    int64_t v0;
    int64_t v1;
    double v2;
    double pi;
    int64_t v3;
    int64_t dummy;
    char buffer0[256];
    char buffer1[9];
    char buffer2[1];
    char buffer3[1];
    double v4;

    (void)userData;

    assert_int_equal(load_state->read_int(load_state->priv_data, &v0), PDLoadStatus_Ok);
    assert_int_equal(v0, -1231);

    assert_int_equal(load_state->read_int(load_state->priv_data, &v1), PDLoadStatus_Ok);
    assert_int_equal(v1, 1);

    assert_int_equal(load_state->read_double(load_state->priv_data, &v2), PDLoadStatus_Converted);
    assert_int_equal((int)v2, 1231);

    assert_int_equal(load_state->read_double(load_state->priv_data, &pi), PDLoadStatus_Ok);
    assert_true(fabs(pi - 3.1415) < 0.0001);

    assert_int_equal(load_state->read_int(load_state->priv_data, &v3), PDLoadStatus_Converted);
    assert_int_equal((int)v3, 8);

    assert_int_equal(load_state->read_string(load_state->priv_data, buffer0, sizeof(buffer0)), PDLoadStatus_Ok);
    assert_string_equal(buffer0, "stoehus");

    assert_int_equal(load_state->read_string(load_state->priv_data, buffer3, 0), PDLoadStatus_Fail);
    assert_int_equal(load_state->read_string(load_state->priv_data, buffer3, 1), PDLoadStatus_Truncated);
    assert_int_equal(buffer3[0], 0);

    assert_int_equal(load_state->read_string(load_state->priv_data, buffer1, sizeof(buffer1)), PDLoadStatus_Truncated);
    assert_string_equal(buffer1, "longlong");

    assert_int_equal(load_state->read_int(load_state->priv_data, &dummy), PDLoadStatus_OutOfData);
    assert_int_equal(load_state->read_double(load_state->priv_data, &v4), PDLoadStatus_OutOfData);
    assert_int_equal(load_state->read_string(load_state->priv_data, buffer2, sizeof(buffer2)), PDLoadStatus_OutOfData);

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin s_dummyPlugin =
{
    "DummyPlugin",
    dummyCreateInstance,
    dummyDestroyInstance,
    dummyUpdate,
    dummySaveState,
    dummyLoadState,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData s_pluginData = { &s_dummyPlugin, PD_VIEW_API_VERSION, "", 0 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* createSession() {
    struct Session* session = Session_create();

    assert_non_null(session);

    return session;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void create_null_session(void**) {
    struct Session* session = createSession();

    Session** sessions = Session_getSessions();

    assert_int_equal(array_size(sessions), 1);

    Session_destroy(session);

    assert_int_equal(array_size(sessions), 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void session_add_plugins(void**) {
    int pluginCount = 0;

    struct Session* session = createSession();

    struct ViewPluginInstance* i0 = g_pluginUI->createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i1 = g_pluginUI->createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i2 = g_pluginUI->createViewPlugin(&s_pluginData);

    Session_addViewPlugin(session, i0);
    Session_addViewPlugin(session, i1);
    Session_addViewPlugin(session, i2);

    struct ViewPluginInstance** instances = Session_getViewPlugins(session, &pluginCount);

    assert_true(pluginCount == 3);

    assert_true(instances[0] == i0);
    assert_true(instances[1] == i1);
    assert_true(instances[2] == i2);

    Session_destroy(session);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void session_delete_plugins(void**) {
    int pluginCount = 0;

    struct Session* session = createSession();

    struct ViewPluginInstance* i0 = g_pluginUI->createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i1 = g_pluginUI->createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i2 = g_pluginUI->createViewPlugin(&s_pluginData);

    Session_addViewPlugin(session, i0);
    Session_addViewPlugin(session, i1);
    Session_addViewPlugin(session, i2);

    // Delete one of the plugins

    assert_true(Session_removeViewPlugin(session, i1) == true);
    assert_true(Session_removeViewPlugin(session, i1) == false);

    struct ViewPluginInstance** instances = Session_getViewPlugins(session, &pluginCount);

    assert_true(pluginCount == 2);
    assert_true(instances[0] == i0);
    assert_true(instances[1] == i2);

    Session_destroy(session);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void session_test_many(void**) {
    struct Session* s0 = createSession();
    struct Session* s1 = createSession();
    struct Session* s2 = createSession();

    Session** sessions = Session_getSessions();

    assert_int_equal(array_size(sessions), 3);

    assert_true(sessions[0] == s0);
    assert_true(sessions[1] == s1);
    assert_true(sessions[2] == s2);

    Session_destroy(s1);

    assert_int_equal(array_size(sessions), 2);
    assert_true(sessions[0] == s0);
    assert_true(sessions[1] == s2);

    Session_globalDestroy();

    assert_int_equal(array_size(sessions), 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void session_save_plugin_state(void**) {
    const char* filename = "t2-output/test.json";

    {
        PDSaveState saveFuncs;

        json_t* root = json_object();
        json_t* array = json_array();
        json_object_set_new(root, "plugin_data", array);

        PluginIO_initSaveJson(&saveFuncs);

        saveFuncs.priv_data = array;

        s_dummyPlugin.save_state(0, &saveFuncs);

        json_dump_file(root, filename, JSON_COMPACT | JSON_INDENT(4) | JSON_PRESERVE_ORDER);
    }

    {
        PDLoadState loadFuncs;

        json_error_t error;

        json_t* root = json_load_file(filename, 0, &error);

        if (!root || !json_is_object(root)) {
            printf("JSON: Unable to open %s for read\n", filename);
            return;
        }

        json_t* pluginData = json_object_get(root, "plugin_data");
        assert_true(json_typeof(pluginData) == JSON_ARRAY);

        SessionLoadState load_state = { pluginData, (int)json_array_size(pluginData), 0 };

        PluginIO_initLoadJson(&loadFuncs);
        loadFuncs.priv_data = &load_state;

        s_dummyPlugin.load_state(0, &loadFuncs);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    Core_init();

    g_pluginUI = new BgfxPluginUI;

    const UnitTest tests[] =
    {
        unit_test(create_null_session),
        unit_test(session_add_plugins),
        unit_test(session_delete_plugins),
        unit_test(session_test_many),
        unit_test(session_save_plugin_state),
    };

    return run_tests(tests);
}
