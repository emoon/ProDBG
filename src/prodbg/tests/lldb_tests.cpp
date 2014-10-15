#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "session/session.h"
#include "session/session_private.h"
#include "core/plugin_handler.h"
#include "core/time.h"
#include "core/log.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_lldb(void** state)
{
	(void)state;

	// we only do the LLDB test on Mac for now

#ifdef __APPLE__

	PluginData* pluginData;
	Session* session;
	int count = 0;

	assert_true(PluginHandler_addPlugin(OBJECT_DIR, "lldb_plugin"));
	assert_non_null(pluginData = PluginHandler_getPlugins(&count)[0]);

	session = Session_createLocal((PDBackendPlugin*)pluginData->plugin, OBJECT_DIR "/crashing_native");
	assert_non_null(session);

	// I hate to do this but there is really no good way to deal with this otherwise (at least currently)
	Time_sleepMs(800);

	Session_update(session);
	Session_update(session);

	// Expect that we have a crash here and thus are in PDDebugState_stopException state

	assert_int_equal(session->state, PDDebugState_stopException);

#endif

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    const UnitTest tests[] =
    {
        unit_test(test_lldb),
    };

    return run_tests(tests);
}

