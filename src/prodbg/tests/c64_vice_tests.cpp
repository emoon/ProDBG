#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "api/include/pd_readwrite.h"
#include "api/src/remote/pd_readwrite_private.h"
#include "core/core.h"
#include "core/plugin_handler.h"
#include "core/process.h"
#include "core/time.h"
#include "core/math.h"
#include "session/session.h"
#include "session/session_private.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CPUState
{
    uint16_t pc;
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    CPUState_maskA = 1 << 0,
    CPUState_maskX = 1 << 1,
    CPUState_maskY = 1 << 2,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Session* s_session;
static ProcessHandle s_viceHandle;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_c64_vice_init(void**)
{
    int count = 0;

    assert_true(PluginHandler_addPlugin(OBJECT_DIR, "c64_vice_plugin"));
    assert_non_null(PluginHandler_getBackendPlugins(&count)[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_c64_vice_fail_connect(void**)
{
    int count = 0;

    PluginData* pluginData;

    pluginData = PluginHandler_getBackendPlugins(&count)[0];

    s_session = Session_createLocal((PDBackendPlugin*)pluginData->plugin, 0);

    Session_update(s_session);

    // We haven't setup vice at this point so no connect

    assert_int_equal(s_session->state, PDDebugState_noTarget);

    Session_destroy(s_session);

    s_session = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool getMemory(void* dest, int* len, uint16_t inAddress, int readLength)
{
    PDWriter* writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_getMemory);
    PDWrite_u64(writer, "address_start", inAddress);
    PDWrite_u64(writer, "size", (uint32_t)readLength);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    Session_update(s_session);

    PDReader* reader = s_session->reader;
    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(s_session->currentWriter), PDBinaryWriter_getSize(s_session->currentWriter));

	uint32_t event;

	while ((event = PDRead_getEvent(reader)) != 0)
	{
		uint8_t* data;
		uint64_t dataSize;
		uint64_t address;

		if (event != PDEventType_setMemory)
			continue;

		assert_true(PDRead_findU64(reader, &address, "address", 0) & PDReadStatus_ok);
		assert_true((PDRead_findData(reader, (void**)&data, &dataSize, "data", 0) & PDReadStatus_typeMask) == PDReadType_data);

		memcpy(dest, data, dataSize);

		*len = (int)dataSize;

		return true;
	}

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateRegisters(CPUState* cpuState, PDReader* reader)
{
    PDReaderIterator it;

    if (PDRead_findArray(reader, &it, "registers", 0) == PDReadStatus_notFound)
        return;

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* name = "";
        uint16_t regValue;

        PDRead_findString(reader, &name, "name", it);
        PDRead_findU16(reader, &regValue, "register", it);

        if (!strcmp(name, "pc"))
            cpuState->pc = (uint16_t)regValue;
        else if (!strcmp(name, "sp"))
            cpuState->sp = (uint8_t)regValue;
        else if (!strcmp(name, "a"))
            cpuState->a = (uint8_t)regValue;
        else if (!strcmp(name, "x"))
            cpuState->x = (uint8_t)regValue;
        else if (!strcmp(name, "y"))
            cpuState->y = (uint8_t)regValue;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool handleEvents(CPUState* cpuState, Session* session)
{
	uint32_t event = 0;

	PDReader* reader = session->reader;

	PDBinaryReader_initStream(reader, PDBinaryWriter_getData(session->currentWriter), PDBinaryWriter_getSize(session->currentWriter));

	while ((event = PDRead_getEvent(reader)) != 0)
	{
		if (event == PDEventType_setRegisters)
		{
			updateRegisters(cpuState, reader);
			return true;
		}
	}

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printCPUState(CPUState* state)
{
    printf("pc %04x - a %02x - x - %02x - y %02x - sp %02x\n", state->pc, state->a, state->x, state->y, state->sp);
}

//////////////////////////////////////////////hndle////////////////////////////////////////////////////////////////////////

static void test_c64_vice_connect(void**)
{
    int count = 0;

    PluginData* pluginData;

    pluginData = PluginHandler_getBackendPlugins(&count)[0];

    // Lanuch C64 VICE
    // TODO: Fix hardcoded path

#ifdef PRODBG_MAC
    //const char* viceLaunchPath = "../../vice/x64.app/Contents/MacOS/x64";
    const char* viceLaunchPath = "/Applications/VICE/x64.app/Contents/MacOS/x64";
#elif PRODBG_WIN
    const char* viceLaunchPath = "..\\..\\vice\\x64.exe";
#else
    // Not supported on Linux yet
    const char* viceLaunchPath = 0;
#endif
    assert_non_null(viceLaunchPath);

    //const char* argv[] = { viceLaunchPath, "-remotemonitor", "-console", "examples/c64_vice/test.prg", 0 };
    const char* argv[] = { viceLaunchPath, "-remotemonitor", "-console", 0 };

    s_viceHandle = Process_spawn(viceLaunchPath, argv);

    assert_non_null(s_viceHandle);

    // Wait 3 sec for VICE to launch

    Time_sleepMs(3000);

    // TODO: Non hard-coded path

    s_session = Session_createLocal((PDBackendPlugin*)pluginData->plugin, 0);

    // make sure we attach to VICE

    PDWriter* writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_menuEvent);
    PDWrite_u32(writer, "menu_id", 0);
    PDWrite_eventEnd(writer);

    Session_update(s_session);

    // We haven't setup vice at this point so no connect

    assert_int_not_equal(s_session->state, PDDebugState_noTarget);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_c64_vice_start_executable(void**)
{
	const char* prgFile = "/Users/danielcollin/code/ProDBG/examples/c64_vice/test.prg";

    PDWriter* writer = s_session->currentWriter;
    PDWrite_eventBegin(writer, PDEventType_setExecutable);
    PDWrite_string(writer, "filename", prgFile); 
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    Session_update(s_session);

    // Annoying to da anything about this as VICE doesn't reply back anything
    // when doing <g $xxx>

    Time_sleepMs(200);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_c64_vice_get_registers(void**)
{
    CPUState state;

    PDWriter* writer = s_session->currentWriter;
    PDWrite_eventBegin(writer, PDEventType_getRegisters);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    Session_update(s_session);

    assert_true(handleEvents(&state, s_session));
    assert_true(state.pc >= 0x80e && state.pc <= 0x81a);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_c64_vice_step_cpu(void**)
{
    CPUState state;

    Session_action(s_session, PDAction_step);
    assert_true(handleEvents(&state, s_session));

    assert_true(state.pc >= 0x80e && state.pc <= 0x81a);
    assert_int_equal(state.a, 0x22);
    assert_int_equal(state.x, 0x32);

    Session_action(s_session, PDAction_step);
    assert_true(handleEvents(&state, s_session));

    assert_true(state.pc >= 0x80e && state.pc <= 0x81a);

    Session_action(s_session, PDAction_step);
    assert_true(handleEvents(&state, s_session));

    assert_true(state.pc >= 0x80e && state.pc <= 0x81a);

    // Get registers after some stepping

    PDWriter* writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_getRegisters);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    assert_true(handleEvents(&state, s_session));

    assert_true(state.pc >= 0x80e && state.pc <= 0x81a);

    writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_getRegisters);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    assert_true(handleEvents(&state, s_session));

    assert_true(state.pc >= 0x80e && state.pc <= 0x81a);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Assembly
{
    uint16_t address;
    const char* text;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_c64_vice_get_disassembly(void**)
{
    static Assembly assembly[] =
    {
        { 0x080e, "A9 22       LDA #$22" },
        { 0x0810, "A2 32       LDX #$32" },
        { 0x0812, "C8          INY"      },
        { 0x0813, "EE 20 D0    INC $D020" },
        { 0x0816, "EE 21 D0    INC $D021" },
        { 0x0819, "4C 0E 08    JMP $080E" },
        { 0, 0 },
    };

    PDWriter* writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_getDisassembly);
    PDWrite_u64(writer, "address_start", 0x80e);
    PDWrite_u32(writer, "instruction_count", (uint32_t)4);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    Session_update(s_session);

    PDReader* reader = s_session->reader;

    PDBinaryReader_initStream(reader, PDBinaryWriter_getData(s_session->currentWriter), PDBinaryWriter_getSize(s_session->currentWriter));

    uint32_t event;

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        if (event != PDEventType_setDisassembly)
            continue;

        PDReaderIterator it;

        assert_false(PDRead_findArray(reader, &it, "disassembly", 0) == PDReadStatus_notFound);

        int i = 0;

        while (PDRead_getNextEntry(reader, &it))
        {
            uint64_t address;
            const char* text;

            PDRead_findU64(reader, &address, "address", it);
            PDRead_findString(reader, &text, "line", it);

            assert_non_null(assembly[i].text);
            assert_int_equal((int)assembly[i].address, (int)address);
            assert_string_equal(assembly[i].text, text);

            i++;
        }

        return;

    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_c64_vice_get_memory(void**)
{
    const uint8_t read_memory[] = { 0xa9, 0x22, 0xa2, 0x32, 0xc8, 0xee, 0x20, 0xd0, 0xee, 0x21, 0xd0, 0x4c, 0x0e, 0x08 };
    uint8_t dest[sizeof(read_memory) + 1];
    int dataSize = 0;

	assert_true(getMemory(dest, &dataSize, 0x080e, 14)); 

	assert_true(dataSize >= 14);

	assert_memory_equal(dest, read_memory, sizeof_array(read_memory));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool getExceptionLocation(PDReader* reader, uint64_t* address, CPUState* cpuState)
{
    uint32_t event;
    bool foundException = false;

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_setRegisters:
            {
                if (cpuState)
                    updateRegisters(cpuState, reader);

                break;
            }

            case PDEventType_setExceptionLocation:
            {
                assert_true(PDRead_findU64(reader, address, "address", 0) & PDReadStatus_ok);
                foundException = true;

                break;
            }
        }
    }

    return foundException;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void stepToPC(uint64_t pc)
{
    // first we step the CPU se we have the PC at known position, we try to step 10 times
    // and if we don't get the correct PC with in that time we fail the test

    for (int i = 0; i < 10; ++i)
    {
        CPUState state;

        Session_action(s_session, PDAction_step);
        //Session_update(s_session);
        assert_true(handleEvents(&state, s_session));

        if (state.pc == pc)
            break;

        if (i == 9)
            fail();

        Time_sleepMs(1);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void waitForBreak(uint64_t breakAddress, const CPUState* cpuState, uint64_t checkMask)
{
    CPUState outState;

    //printf("wait for break $%llx\n", breakAddress);

    for (int i = 0; i < 12; ++i)
    {
        uint64_t address = 0;

        PDReader* reader = s_session->reader;

        PDBinaryReader_initStream(reader, PDBinaryWriter_getData(s_session->currentWriter), PDBinaryWriter_getSize(s_session->currentWriter));

        if (getExceptionLocation(reader, &address, &outState))
        {
            if (checkMask & CPUState_maskA)
                assert_true(cpuState->a == outState.a);
            if (checkMask & CPUState_maskX)
                assert_true(cpuState->x == outState.x);
            if (checkMask & CPUState_maskY)
                assert_int_equal(cpuState->y, outState.y);

            assert_int_equal((int)address, (int)breakAddress);

            return;
        }

        Time_sleepMs(1);

        Session_update(s_session);
    }

    fail();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_c64_vice_basic_breakpoint(void**)
{
    Session_action(s_session, PDAction_step);

    uint64_t breakAddress = 0x0813;

    stepToPC(0x080e);

    // Add a breakpoint at 0x0814

    PDWriter* writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
    PDWrite_u64(writer, "address", breakAddress);
    PDWrite_eventEnd(writer);

    PDBinaryWriter_finalize(writer);

    Session_update(s_session);
    Session_action(s_session, PDAction_run);

    waitForBreak(breakAddress, 0, 0);

    stepToPC(0x080e);

    // Update the breakpoint to different address

    breakAddress = 0x0816;

    writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
    PDWrite_u64(writer, "address", breakAddress);
    PDWrite_u64(writer, "id", 1);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    Session_update(s_session);
    Session_action(s_session, PDAction_run);

    waitForBreak(breakAddress, 0, 0);

    // Delete the breakpoint

    stepToPC(0x080e);

    writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_deleteBreakpoint);
    PDWrite_u32(writer, "id", 2);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    Session_update(s_session);
    Session_action(s_session, PDAction_run);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_c64_vice_breakpoint_cond(void**)
{
    CPUState state = { 0 };

    Session_action(s_session, PDAction_step);

    const uint64_t breakAddress = 0x0816;

    stepToPC(0x080e);

    // Add a breakpoint at 0x0814

    PDWriter* writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
    PDWrite_u64(writer, "address", breakAddress);
    PDWrite_string(writer, "condition", ".y == 0");
    PDWrite_eventEnd(writer);

    PDBinaryWriter_finalize(writer);

    Session_update(s_session);
    Session_action(s_session, PDAction_run);

    waitForBreak(breakAddress, &state, CPUState_maskY);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_c64_vice_callstack(void**)
{
	uint32_t event;
    PDReaderIterator it;

	uint16_t refCallstack[] =
	{
		0xe112 + 2, // (2) e112
		0xa562 + 4, // (4) a562
		0xa483 + 6, // (6) a483
		0xa677 + 8, // (8) a677
		0xe39a + 10, // (10) e39a
	};

    PDWriter* writer = s_session->currentWriter;

    PDWrite_eventBegin(writer, PDEventType_getCallstack);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    Session_update(s_session);

	PDReader* reader = s_session->reader;

	PDBinaryReader_initStream(reader, PDBinaryWriter_getData(s_session->currentWriter), PDBinaryWriter_getSize(s_session->currentWriter));

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_setCallstack:
            {
				if (PDRead_findArray(reader, &it, "callstack", 0) == PDReadStatus_notFound)
					return;

				int callstackSize = sizeof_array(refCallstack);
				int count = 0;

				while (PDRead_getNextEntry(reader, &it))
				{
					uint16_t address;

        			PDRead_findU16(reader, &address, "address", it);

        			assert_true(count < callstackSize);
        			assert_int_equal(refCallstack[count], address);

					count++;
				}

				return;
            }
        }
    }

    fail();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static void test_c64_vice_set_memory(void**)
{
	uint8_t data[16];
	uint8_t dataWrite[16];
	bool dataEqual = true;

	getMemory

	assert_true(getMemory(data, 0x1000, sizeof_array(data)));

	for (int i = 0; i < sizeof_array(data); ++i)
	{
		dataWrite[i] = (uint8_t)i;
		if (dataWrite[i] != data[i])
			dataEqual = false;
	}

	// make sure the data we read is not the same as the one we are going to write. This is a bit ugly as
	// we assume the C64 memory at 0x1000 doesn't contain the current pattern but should be fine for our use

	assert_false(dataEqual);

	// write down the data

	assert_true(writeMemory(0x01000, data, sizeof_array(data)));
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	Core_init();

    const UnitTest tests[] =
    {
        unit_test(test_c64_vice_init),
        unit_test(test_c64_vice_fail_connect),
        unit_test(test_c64_vice_connect),
        unit_test(test_c64_vice_start_executable),
        unit_test(test_c64_vice_get_registers),
        unit_test(test_c64_vice_step_cpu),
        unit_test(test_c64_vice_get_disassembly),
        unit_test(test_c64_vice_get_memory),
        unit_test(test_c64_vice_basic_breakpoint),
        unit_test(test_c64_vice_breakpoint_cond),
        unit_test(test_c64_vice_callstack),
        //unit_test(test_c64_vice_set_memory),
    };

    int test = run_tests(tests);

    if (s_viceHandle)
        Process_kill(s_viceHandle);

    return test;
}

