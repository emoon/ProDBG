#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "core/core.h"
#include "core/session.h"
#include "api/plugin_instance.h"

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

    struct ViewPluginInstance* i0 = PluginInstance_createViewPlugin();
    struct ViewPluginInstance* i1 = PluginInstance_createViewPlugin();
    struct ViewPluginInstance* i2 = PluginInstance_createViewPlugin();

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

    struct ViewPluginInstance* i0 = PluginInstance_createViewPlugin();
    struct ViewPluginInstance* i1 = PluginInstance_createViewPlugin();
    struct ViewPluginInstance* i2 = PluginInstance_createViewPlugin();

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
