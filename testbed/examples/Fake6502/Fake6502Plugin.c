#include "../../API/ProDBGAPI.h" 
#include "Debugger6502.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Debugger6502* g_debugger;
extern uint16_t pc;
extern uint8_t sp, a, x, y, status;
extern int disassembleToBuffer(char* dest, int* address, int* instCount);

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


/*
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
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeRegister(PDWriter* writer, const char* name, uint8_t size, uint16_t reg, bool readOnly)
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
	PDWrite_u16(writer, "address_start", start);
	PDWrite_u16(writer, "instruction_count", instCount);
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
			debugger->runState = PDDebugState_stopException;
			sendState(writer);
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

	while ((event = PDRead_getEvent(reader)))
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

