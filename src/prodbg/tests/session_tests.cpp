#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <pd_view.h>

#include "core/core.h"
#include "session/session.h"
#include "api/plugin_instance.h"
#include "core/plugin_handler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* dummyCreateInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
	(void)uiFuncs;
	(void)serviceFunc;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummyDestroyInstance(void* userData)
{
	(void)userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int dummyUpdate(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
	(void)userData;
	(void)uiFuncs;
	(void)inEvents;
	(void)outEvents;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin s_dummyPlugin =
{
    "DummyPlugin",
    dummyCreateInstance,
    dummyDestroyInstance,
    dummyUpdate,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData s_pluginData = { &s_dummyPlugin , PD_VIEW_API_VERSION, "", 0 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* createSession()
{
    struct Session* session = Session_create();

    assert_non_null(session);

    return session;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void create_null_session(void** state)
{
    (void)state;

    struct Session* session = createSession();

    Session_destroy(session);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void session_add_plugins(void** state)
{
    (void)state;
    int pluginCount = 0;

    struct Session* session = createSession();

    struct ViewPluginInstance* i0 = PluginInstance_createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i1 = PluginInstance_createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i2 = PluginInstance_createViewPlugin(&s_pluginData);

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

static void session_delete_plugins(void** state)
{
    (void)state;
    int pluginCount = 0;

    struct Session* session = createSession();

    struct ViewPluginInstance* i0 = PluginInstance_createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i1 = PluginInstance_createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i2 = PluginInstance_createViewPlugin(&s_pluginData);

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

int main()
{
    const UnitTest tests[] =
    {
        unit_test(create_null_session),
        unit_test(session_add_plugins),
        unit_test(session_delete_plugins),
    };

    return run_tests(tests);
}
