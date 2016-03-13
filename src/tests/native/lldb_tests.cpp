#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "api/include/pd_readwrite.h"
#include "api/src/remote/pd_readwrite_private.h"
#include "session/session.h"
#include "session/session_private.h"
#include "core/plugin_handler.h"
#include "core/time.h"
#include "core/log.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_lldb(void** state) {
    (void)state;

    // we only do the LLDB test on Mac for now

#ifdef __APPLE__

    PluginData* pluginData;
    Session* session;
    int count = 0;

    assert_true(PluginHandler_addPlugin(OBJECT_DIR, "lldb_plugin"));
    assert_non_null(pluginData = PluginHandler_getBackendPlugins(&count)[0]);

    session = Session_createLocal((PDBackendPlugin*)pluginData->plugin, OBJECT_DIR "/crashing_native");
    assert_non_null(session);

    // I hate to do this but there is really no good way to deal with this otherwise (at least currently)
    Time_sleepMs(800);

    Session_update(session);
    Session_update(session);

    // Expect that we have a crash here and thus are in PDDebugState_stopException state

    assert_int_equal(session->state, PDDebugState_StopException);

    // Request locals location.

    PDWriter* writer = session->currentWriter;
    PDReader* reader = session->reader;

    PDWrite_event_begin(writer, PDEventType_GetCallstack);
    PDWrite_u8(writer, "dummy", 0);
    PDWrite_event_end(writer);

    Session_update(session);

    PDBinaryWriter_finalize(session->currentWriter);
    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(session->currentWriter), PDBinaryWriter_getSize(session->currentWriter));

    uint32_t event;
    bool foundCallstack = false;

    while ((event = PDRead_get_event(reader)) != 0) {
        switch (event) {
            case PDEventType_SetCallstack:
            {
                foundCallstack = true;
                break;
            }
        }
    }

    assert_true(foundCallstack);

#endif

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    const UnitTest tests[] =
    {
        unit_test(test_lldb),
    };

    return run_tests(tests);
}

