#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "core/time.h"
#include "core/process.h"
#include "session/session.h"

struct Session* Session_createRemote(const char* target, int port);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_remote_session(void**) {
    Session* session = Session_createRemote("127.0.0.1", 1340);

    assert_true(Session_isConnected(session));

    Session_destroy(session);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    int ret = 0;

    const UnitTest tests[] =
    {
        unit_test(test_remote_session),
    };

    static const char* fake_exe = OBJECT_DIR "/fake6502";
    static const char* argv[] = {fake_exe, "examples/fake_6502/test.bin", 0};

    ProcessHandle fakeHandle = Process_spawn(fake_exe, argv);

    if (!fakeHandle)
        return -1;

    // Wait 1 sec for the remote process to setup socket, get everything running, etc
    // Not really nice but this is more of simulating how it would work in a real life
    // test when connecting with the debugger to a remote process

    Time_sleepMs(1000);

    ret = run_tests(tests);

    Process_kill(fakeHandle);

    return ret;
}
