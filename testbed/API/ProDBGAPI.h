/* ********** NOTICE : THIS IS JUST A DRAFT ***********************/

#ifndef _PRODBGAPI_H_
#define _PRODBGAPI_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef _cplusplus
extern "C" {
#endif

/*! \fn void* ServiceFunc(const char* serviceName)
    \breif Service Function. Provides services for the plugin to use.
    Example:
     ProDBGUI* ui = serviceFunc(PRODBG_UI_SERVICE);
     ProDBServerInfo* serverInfo = serviceFunc(PRODBG_SERVERINFO_SERVICE);
     It's ok for the plugin to hold a pointer to the requested service during its life time.
    \param serviceName The name of the requested service. It's *highly* recommended to use the defines for the wanted service.
*/

typedef void* ServiceFunc(const char* serviceName);
typedef void RegisterPlugin(int type, void* data);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    PD_API_VERSION = 1
};

typedef uint64_t PDToken;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDebugState
{
    PDDebugState_noTarget,         // nothing is running 
    PDDebugState_running,          // target is being executed 
    PDDebugState_stopBreakpoint,   // Stop on Breakpoint 
    PDDebugState_stopException,    // Stop on Exception 
    PDDebugState_stepping,         // code is currently being stepped/traced/etc 
    PDDebugState_custom = 0x1000   // Start of custom ids 
} PDDebugState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDAction
{
    PDAction_none,
    PDAction_break,
    PDAction_run,
    PDAction_step,
    PDAction_stepOut,
    PDAction_stepOver,
    PDAction_custom = 0x1000
} PDAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDEventType
{
	// get events

    PDEventType_getLocals,
    PDEventType_getCallStack,
    PDEventType_getWatch,
    PDEventType_getRegisters,
    PDEventType_getMemory,
    PDEventType_getTty,
    PDEventType_getExceptionLocation,
    PDEventType_getDisassembly,

    // set events

    PDEventType_setBreakpointAddress = 0x500,
    PDEventType_setBreakpointSourceLine,
    PDEventType_start,
    PDEventType_setExecutable,
    PDEventType_attachToProcess,
    PDEventType_attachToRemoteSession,

	// custom

    PDEventType_custom = 0x1000

} PDEventType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSerializeWrite
{
	// First Argument to all functions
	void* writeData;

	// Write 32-bit signed integer
    void (*writeInt)(void* writeData, int);

	// Write unsigned signed short (16-bit)
    void (*writeU16)(void* writeData, unsigned short);

	// Write unsigned char (8-bit) 
    void (*writeU8)(void* writeData, unsigned char);

    // Write null-teminated string
    void (*writeString)(void* writeData, const char* string);

} PDSerializeWrite;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSerializeRead
{
	// First argument to all functions
	void* readData;

	// Read a 32-bit int from the stream
	// TODO: better with int32/uint32/I32/I16, etc and maybe add readI4Array(...) with count
    int (*readInt)(void* readData);

    // TODO: Extend even more
    unsigned short (*readU16)(void* readData);

    // TODO: Extend even more
    unsigned char (*readU8)(void* readData);

    // Read zero terminated string (caller needs to take a copy if reqired)
    const char* (*readString)(void* readData);

    // Checks how many bytes that there are left in the stream
	int (*bytesLeft)(void* readData);

	// Skip bytes to be read (useful if there is a unknown section with known size)
	void (*skipBytes)(void* readData, int count);

} PDSerializeRead;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper macros 

#define PDREAD_INT(reader) reader->readInt(reader->readData)
#define PDREAD_U16(reader) reader->readU16(reader->readData)
#define PDREAD_STRING(reader) reader->readString(reader->readData)
#define PDREAD_SKIP_BYTES(reader, n) reader->skipBytes(reader->readData, n)
#define PDREAD_BYTES_LEFT(reader) reader->bytesLeft(reader->readData)

#define PDWRITE_EVENT_BEGIN(writer, event, id) writer->eventBegin(writer->writeData, event, id)
#define PDWRITE_EVENT_END(writer, event, id) writer->eventEnd(writer->writeData)
#define PDWRITE_INT(writer, value) writer->writeInt(writer->writeData, value)
#define PDWRITE_U16(writer, value) writer->writeU16(writer->writeData, value)
#define PDWRITE_U8(writer, value) writer->writeU8(writer->writeData, value)
#define PDWRITE_STRING(writer, value) writer->writeString(writer->writeData, value)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDBackendPlugin
{
    int version;
    const char* name;

    // Create and destroy instance of the plugin
    
    void* (*createInstance)(ServiceFunc* serviceFunc);
    void (*destroyInstance)(void* userData);

    // Updates and Returns the current state of the plugin.

    PDDebugState (*update)(void* userData);

    // Various actions can be send to the plugin. like
    // Break, continue, exit, detach etc.
    bool (*action)(void* userData, PDAction action);

    // Get some data state  
    int (*getState)(void* userData, PDEventType eventType, int eventId, PDSerializeRead* reader, PDSerializeWrite* writer);

    // set/get some dat
    int (*setState)(void* userData, PDEventType event, int eventId, PDSerializeRead* reader, PDSerializeWrite* writer);

} PDBackendPlugin;

#ifdef _cplusplus
}
#endif

#endif

