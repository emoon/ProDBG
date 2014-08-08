#include <PDBackend.h> 
#include "Debugger6502.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Debugger6502* g_debugger;
extern uint16_t pc;
extern uint8_t sp, a, x, y, status;
extern int disassembleToBuffer(char* dest, int* address, int* instCount);
extern struct PDBackendPlugin s_debuggerPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc)
{
    (void)serviceFunc;

    g_debugger = malloc(sizeof(Debugger6502));    // this is a bit ugly but for this plugin we only have one instance
    memset(g_debugger, 0, sizeof(Debugger6502));

    g_debugger->runState = PDDebugState_running;

    return g_debugger;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
    g_debugger = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeRegister(PDWriter* writer, const char* name, uint8_t size, uint16_t reg, uint8_t readOnly)
{
    PDWrite_arrayEntryBegin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "size", size);

    if (readOnly)
        PDWrite_u8(writer, "read_only", 1);

    PDWrite_u16(writer, "register", reg);

    PDWrite_arrayEntryEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setRegisters(PDWriter* writer)
{
    PDWrite_eventBegin(writer, PDEventType_setRegisters);
    PDWrite_arrayBegin(writer, "registers");

    writeRegister(writer, "pc", 2, pc, 1);
    writeRegister(writer, "sp", 1, sp, 0);
    writeRegister(writer, "a", 1, x, 0);
    writeRegister(writer, "x", 1, x, 0);
    writeRegister(writer, "y", 1, y, 0);
    writeRegister(writer, "status", 1, y, 1);

    PDWrite_arrayEnd(writer);
    PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExceptionLocation(PDWriter* writer)
{
    PDWrite_eventBegin(writer,PDEventType_setExceptionLocation); 
    PDWrite_u16(writer, "address", pc);
    PDWrite_u8(writer, "address_size", 2);
    PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setDisassembly(PDWriter* writer, int start, int instCount)
{
    char temp[65536];

    disassembleToBuffer(temp, &start, &instCount);

    PDWrite_eventBegin(writer, PDEventType_setDisassembly);
    PDWrite_u16(writer, "address_start", (uint16_t)start);
    PDWrite_u16(writer, "instruction_count", (uint16_t)instCount);
    PDWrite_string(writer, "string_buffer", temp);
    PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getDisassembly(PDReader* reader, PDWriter* writer)
{
    uint16_t start = 0;
    uint32_t instCount = 1;

    PDRead_findU16(reader, &start, "address_start", 0);
    PDRead_findU32(reader, &instCount, "instruction_count", 0);

    setDisassembly(writer, start, instCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sendState(PDWriter* writer)
{
    setExceptionLocation(writer);
    setRegisters(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void doAction(Debugger6502* debugger, PDAction action, PDWriter* writer)
{
    int t = (int)action;

    switch (t)
    {
        case PDAction_break : 
        {
            // On this target we can anways break so just set that we have stopped on breakpoint
            
            printf("Fake6502Debugger: break\n");
            debugger->runState = PDDebugState_stopException;
            sendState(writer);
            break;
        }

        case PDAction_run : 
        {
            // on this target we can always start running directly again
            printf("Fake6502Debugger: run\n");
            debugger->runState = PDDebugState_running;
            break;
        }

        case PDAction_step : 
        {
            // on this target we can always stepp 
            printf("Fake6502Debugger: step\n");
            debugger->runState = PDDebugState_trace;
            sendState(writer);
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
    int event = 0;

    Debugger6502* debugger = (Debugger6502*)userData;

    doAction(debugger, action, writer);

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_getDisassembly : getDisassembly(reader, writer); break;
        }
    }

    return debugger->runState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin s_debuggerPlugin =
{
    1,
    "Fake6502",
    createInstance,
    destroyInstance,
    update,
};

