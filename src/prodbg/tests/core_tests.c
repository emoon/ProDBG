#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "core/core.h"
#include "core/session.h"
#include "api/plugin_instance.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_null_session(void** state)
{
    (void)state;

    int pluginCount = 0;

    struct Session* session = Session_create();

    assert_non_null(session);

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

	// Delete one of the plugins

	assert_true(Session_removeViewPlugin(session, i1) == true);
	assert_true(Session_removeViewPlugin(session, i1) == false);

    instances = Session_getViewPlugins(session, &pluginCount);

    assert_true(pluginCount == 2);
    assert_true(instances[0] == i0);
    assert_true(instances[1] == i2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    const UnitTest tests[] =
    {
        unit_test(test_null_session),
    };

    return run_tests(tests);
}
