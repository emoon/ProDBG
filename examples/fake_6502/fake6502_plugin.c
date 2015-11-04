#include <pd_backend.h> 
#include "debugger6502.h"
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

    g_debugger->runState = PDDebugState_Running;

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
    PDWrite_array_entry_begin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "size", size);

    if (readOnly)
        PDWrite_u8(writer, "read_only", 1);

    if (size == 2)
    	PDWrite_u16(writer, "register", reg);
	else
    	PDWrite_u8(writer, "register", (uint8_t)reg);

    PDWrite_entry_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setRegisters(PDWriter* writer)
{
    PDWrite_event_begin(writer, PDEventType_SetRegisters);
    PDWrite_array_begin(writer, "registers");

    writeRegister(writer, "pc", 2, pc, 1);
    writeRegister(writer, "sp", 1, sp, 0);
    writeRegister(writer, "a", 1, a, 0);
    writeRegister(writer, "x", 1, x, 0);
    writeRegister(writer, "y", 1, y, 0);
    writeRegister(writer, "status", 1, status, 1);

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExceptionLocation(PDWriter* writer)
{
    PDWrite_event_begin(writer,PDEventType_SetExceptionLocation); 
    PDWrite_u16(writer, "address", pc);
    PDWrite_u8(writer, "address_size", 2);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setDisassembly(PDWriter* writer, int start, int instCount)
{
    char temp[65536];

    disassembleToBuffer(temp, &start, &instCount);

    PDWrite_event_begin(writer, PDEventType_SetDisassembly);
    PDWrite_u16(writer, "address_start", (uint16_t)start);
    PDWrite_u16(writer, "instruction_count", (uint16_t)instCount);
    PDWrite_string(writer, "string_buffer", temp);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static void getDisassembly(PDReader* reader, PDWriter* writer)
{
    uint16_t start = 0;
    uint32_t instCount = 1;

    PDRead_find_u16(reader, &start, "address_start", 0);
    PDRead_find_u32(reader, &instCount, "instruction_count", 0);

    setDisassembly(writer, start, instCount);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sendState(PDWriter* writer)
{
    setExceptionLocation(writer);
    setRegisters(writer);
	setDisassembly(writer, 0, 10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void doAction(Debugger6502* debugger, PDAction action, PDWriter* writer)
{
    int t = (int)action;

    switch (t)
    {
        case PDAction_Break : 
        {
            // On this target we can anways break so just set that we have stopped on breakpoint
            
            printf("Fake6502Debugger: break\n");
            debugger->runState = PDDebugState_StopException;
            sendState(writer);
            break;
        }

        case PDAction_Run : 
        {
            // on this target we can always start running directly again
            printf("Fake6502Debugger: run\n");
            debugger->runState = PDDebugState_Running;
            break;
        }

        case PDAction_Step : 
        {
            // on this target we can always stepp 
            printf("Fake6502Debugger: step\n");
            debugger->runState = PDDebugState_Trace;
            sendState(writer);
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
    //int event = 0;

    (void)reader;

    Debugger6502* debugger = (Debugger6502*)userData;

    doAction(debugger, action, writer);

	/*
    while ((event = PDRead_get_event(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_getDisassembly : getDisassembly(reader, writer); break;
            case PDEventType_getRegisters : setRegisters(writer); break;
        }
    }
    */

    return debugger->runState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin s_debuggerPlugin =
{
    "Fake6502",
    createInstance,
    destroyInstance,
    0,
    update,
};

