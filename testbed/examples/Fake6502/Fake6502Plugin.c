#include "../../API/ProDBGAPI.h" 
#include "Debugger6502.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Debugger6502* g_debugger;
extern uint16_t pc;
extern uint8_t sp, a, x, y, status;
extern int disassembleToBuffer(char* dest, int address, int instCount);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc)
{
	(void)serviceFunc;
	g_debugger = malloc(sizeof(Debugger6502));	// this is a bit ugly but for this plugin we only have one instance
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

static PDDebugState update(void* userData)
{
	Debugger6502* debugger = (Debugger6502*)userData;
	return debugger->runState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool action(void* userData, PDAction action)
{
	int t = (int)action;
	Debugger6502* debugger = (Debugger6502*)userData;

	switch (t)
	{
		case PDAction_break : 
		{
			// On this target we can anways break so just set that we have stopped on breakpoint
			debugger->runState = PDDebugState_stopException;
			break;
		}

		case PDAction_run : 
		{
			// on this target we can always start running directly again
			debugger->runState = PDDebugState_running;
			break;
		}

		case PDAction_step : 
		{
			// on this target we can always stepp 
			debugger->runState = PDDebugState_stepping;
			break;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getRegisters(PDSerializeWrite* writer)
{
	PDWRITE_INT(writer, 7);				// 7 registers;

	PDWRITE_STRING(writer, "pc");		// register name
	PDWRITE_U16(writer, 2);				// number of bytes in register
	PDWRITE_U16(writer, pc);			// value 

	PDWRITE_STRING(writer, "sp");		// register name
	PDWRITE_U16(writer, 1);				// number of bytes in register
	PDWRITE_U16(writer, sp);			// value 

	PDWRITE_STRING(writer, "a");		// register name
	PDWRITE_U16(writer, 1);				// number of bytes in register
	PDWRITE_U8(writer, a);				// value 

	PDWRITE_STRING(writer, "x");		// register name
	PDWRITE_U16(writer, 1);				// number of bytes in register
	PDWRITE_U8(writer, x);				// value 

	PDWRITE_STRING(writer, "y");		// register name
	PDWRITE_U16(writer, 1);				// number of bytes in register
	PDWRITE_U8(writer, y);				// value 

	PDWRITE_STRING(writer, "status");	// register name
	PDWRITE_U16(writer, 1);				// number of bytes in register
	PDWRITE_U8(writer, status);			// value 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getExceptionLocation(PDSerializeWrite* writer)
{
	PDWRITE_STRING(writer, "address");
	PDWRITE_U16(writer, 2);				// size of address in bytes 
	PDWRITE_U16(writer, pc);			// location of pc 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getDisassembly(PDSerializeWrite* writer, int start, int instCount)
{
	char temp[65536];
	disassembleToBuffer(temp, start, instCount);
	PDWRITE_STRING(writer, temp);
	printf("getDisassembly %s\n", temp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getState(void* userData, PDEventType eventType, int eventId, PDSerializeRead* reader, PDSerializeWrite* writer)
{
	int et = (int)eventType;
	//Debugger6502* debugger = (Debugger6502*)userData;

	(void)userData;
	(void)eventType;
	(void)eventId;
	(void)writer;
	(void)reader;

	switch (et)
	{
		case PDEventType_getRegisters :
		{
			getRegisters(writer);
			return 1;
		}

		case PDEventType_getExceptionLocation :
		{
			printf("getExceptionLocation\n");
			getExceptionLocation(writer);
			return 1;
		}

		case PDEventType_getDisassembly :
		{
			int start = PDREAD_INT(reader);
			int count = PDREAD_INT(reader);
			printf("getDisassembly %d %d\n", start, count);
			getDisassembly(writer, start, count);
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int setState(void* userData, PDEventType event, int eventId, PDSerializeRead* reader, PDSerializeWrite* writer)
{
	(void)userData;
	(void)event;
	(void)eventId;
	(void)reader;
	(void)writer;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin s_debuggerPlugin =
{
	1,
	"Fake6502",
    createInstance,
    destroyInstance,
    update,
    action,
    getState,
    setState,
};

