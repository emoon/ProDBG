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

    struct Session* session = Session_createNullSession();

    assert_non_null(session);

    struct ViewPluginInstance* i0 = PluginInstance_createViewPlugin();
    struct ViewPluginInstance* i1 = PluginInstance_createViewPlugin();

    Session_addViewPlugin(session, i0);
    Session_addViewPlugin(session, i1);

    struct ViewPluginInstance** instances = Session_getViewPlugins(session, &pluginCount);

    assert_true(pluginCount == 2);

    assert_true(instances[0] == i0);
    assert_true(instances[1] == i1);
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
