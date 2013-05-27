/* ********** NOTICE : THIS IS JUST A DRAFT ***********************/

#ifndef _PRODBGAPI_H_
#define _PRODBGAPI_H_

#include <stdint.h>

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    PD_API_VERSION = 1,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    PD_API_REMOTE = 1,
    PD_API_ON_HOST = 1,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDBreakpointType
{
    PDBreakpointType_Address           = 0x01,   // Supports setting an breakpoint on an address 
    PDBreakpointType_SourceFileLine    = 0x02,   // Supports setting an breakpoint on source file + line 
    PDBreakpointType_Data              = 0x04,   // Supports databreak points 
    PDBreakpointType_Expression        = 0x08,   // If the breakpoint supports expression 
    PDBreakpointType_CustomUI          = 0x10,   // A custom UI is supplied for the breakpoints 
    PDBreakpointType_None              = 0x20,   // No Breakpoints for this target
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDCallStackType
{
    PDCallStackType_Address,                    // Address (disassembly style)
    PDCallStackType_AddressSourceFileLine,      // Standard source code based file/line callstack with address
    PDCallStackType_SourceFileLine,             // code based file/line callstack without address
    PDCallStackType_CustomUI,                   // A custom UI is supplied for the Callstack 
    PDCallStackType_None,                       // No callstack for this target 

} PBCallStackType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PVariableTypeSupport
{
    PDVariableTypeSupport_Supported,            // If we support standard local variables
    PDVariableTypeSupport_CustomUI,             // CustomUI for local variables 
    PDVariableTypeSupport_None,                 // No local function variables
} PDLocalVariableType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDVariableType
{
    PDVariableType_Unsigned,                    // unsigned type
    PDVariableType_Signed,                      // signed
    PDVariableType_Float,                       // float
    PDVariableType_Vector,                      // Vector
    PDVariableType_Custom,                      // Custom

} PDVariableType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDVariable
{
    const char* name;                           // Name of the variable
    const char* value;                          // Value
    size_t size;                                // Size of the variable
    PDVariableType type;                        // Type
    bool hasChildren;                           // If the type has childern (typical for structs)

} PDVariable;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDRegisterFlags
{
    PDRegisterFlags_ReadOnly      = 0x01,       // Read-only register
    PDRegisterFlags_Ip            = 0x02,       // Instruction pointer
    PDRegisterFlags_Sp            = 0x04,       // Stack Pointer
    PDRegisterFlags_Float         = 0x08,       // Float register
    PDRegisterFlags_Vector        = 0x10,       // Vector
    PDRegisterFlags_Address       = 0x20,       // Address register
    PDRegisterFlags_CustomFormat  = 0x40,       // Custom format
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDRegisterLayout
{
    const char* name;       // name of the register
    int registerSize;       // size of the register
    int flags;              // Mask of PDRegisterFlags  
    const char* fmt;        // If custom format is set in flags this is used to format the output (prinf style string)

} PDRegisterLayout;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDebugState
{
    PDDebugState_noTarget,  // nothing is running 
    PDDebugState_running,   // target is being executed 
    PDDebugState_paused,    // exception, breakpoint, etc 
    PDDebugState_stepping,  // code is currently being stepped/traced/etc 
} PDDebugState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDThreadSupport
{
    PDThreadSupport_Id        = 0x01,       // Just an ID for the thread
    PDThreadSupport_Name      = 0x02,       // We also have a name for the thread
    PDThreadSupport_CustomUI  = 0x04,       // CustomUI for the ThreadView 
    PDThreadSupport_None      = 0x08,       // No thread support 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDThread
{
    size_t id;             // id for the thread
    const char* name;      // name
    bool custom;           // customdata
} PDThread;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDDebugPlugin2
{
    int version,
    const char* name;

    // Create and destroy instance of the plugin (must be implemented)

    void* (*createInstance)(ServiceFunc* serviceFunc);
    void (*destroyInstance)(void* userData);

    // Updates and Returns the current state of the plugin. Must be implemented

    PDDebugState (*update)(void* userData);

    // Various actions can be send to the plugin. like
    // Break, continue, exit, detach etc.

    int (*action)(void* userData, PDAction action);

    //////////////////////
    // Breakpoint support

    int breakpointType;                 // Mask of PDBreakpointType 
    PDUICallback* breakpointUI;         // custom UI if the PDBreakpointType_CustomUI mask is used for type

    // Add breakpoint. breakpointData will needs to be casted to the correct struct
    // given the breakpoint type (only types supplied in the breakpointType flag will be called to this function) 
    int (*addBreakpoint)(void* userData, void* breakpointData, PDBreakpointType type);

    // Delete breakpoint given the ID
    int (*deleteBreakpoint(void* userData, int id);

    //////////////////////
    // CallStack support

    PDCallStackType callStackType;      // Type of callstack (see PDCallStackType for all supported types)
    PDUICallback* callStackUI;          // custom UI if callStack type is PDCallStackType_CustomUI 

    // Recive a callstack. This code will be called with the type supplied in callStackType and needs to
    // return in in the correct expected format (see the structs for each type)
    void* (*getCallStack(void* userData);

    ////////////////////////////
    // Local variables support 
    
    PDVariableTypeSupport localVariables;   // support for local variables 

    // Gets a list of local varibles given a current frame (usually selceted from a callstack)
    PDVariable* (*getLocals)(void* userData, int frameId, int* count);

    ///////////////////////////
    // Local variables support 
    
    PVariableTypeSupport watchVariables;    // support for watch variables 

    // Gets a list of local varibles given a current frame (usually selceted from a callstack)
    PDVariable* (*getWatchVariables)(void* userData, int* count);

    //////////////////
    // Thread support 
    
    int threadSupport;                 // Mask of PDThreadSupport
    PDThread* (*getCallStack(void* userData, size_t* threadCount);
    void (*setThread)(void* userData, int threadId);

    ////////////////////
    // Memory support

    // If target supports reading and writing memory (null supported)

    void (*readMemory)(void* dest, const void* address, size_t size);
    void (*writeMemory)(void* dest, const void* source, size_t size);

    // Register layout for the target (null supported)

    PDRegisterLayout* registerLayout;
    PDRegister* (*getRegisters)(void* userData, int* count);

} PDDebugPlugin2;


#ifdef _cplusplus
}
#endif

#endif

