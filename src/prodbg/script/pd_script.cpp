#include "pd_script.h"

#include <assert.h>
#include <string.h>

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

int PDScript_createState(PDScriptState** scriptState)
{
    assert(scriptState);
    *scriptState = reinterpret_cast<PDScriptState*>(luaL_newstate());
    luaL_openlibs(*scriptState);
    return 0;
}

void PDScript_destroyState(PDScriptState** scriptState)
{
    assert(scriptState);
    lua_close(reinterpret_cast<lua_State*>(*scriptState));
    *scriptState = nullptr;
}

int PDScript_loadFile(PDScriptState* scriptState, const char* scriptFile)
{
    int result = luaL_loadfile(scriptState, scriptFile);
    if (result != 0)
        return result;

    return lua_pcall(scriptState, 0, 0, 0);
}

int PDScript_loadString(PDScriptState* scriptState, const char* scriptString)
{
    int result = luaL_loadstring(scriptState, scriptString);
    if (result != 0)
        return result;

    return lua_pcall(scriptState, 0, 0, 0);
}

int PDScript_loadBuffer(PDScriptState* scriptState, const char* scriptBuffer, size_t size, const char* name, const char* mode)
{
    int result = luaL_loadbufferx(scriptState, scriptBuffer, size, name, mode);
    if (result != 0)
        return result;

    return lua_pcall(scriptState, 0, 0, 0);
}

int PDScript_primeCall(PDScriptState* scriptState, PDScriptCallState* callState, const char* funcName)
{
    assert(scriptState);
    assert(callState);

    callState->funcName = funcName ? strdup(funcName) : 0;

    // Push function name to call onto stack
    if (callState->funcName)
        return lua_getglobal(scriptState, callState->funcName); // Stack now at -1
    else
        return 0;
}

int PDScript_executeCall(PDScriptState* scriptState, PDScriptCallState* callState)
{
    assert(scriptState);
    assert(callState);

    int result = lua_pcall(scriptState, callState->inputCount, callState->outputCount, 0);
    if (result != 0)
    {
        const char* error = lua_tostring(scriptState, -1);
        (void)error;
        return result;
    }

    return 0;
}

#if 0

StkId top;  /* first free slot in the stack */
global_State *l_G;
CallInfo *ci;  /* call info for current function */
const Instruction *oldpc;  /* last pc traced */
StkId stack_last;  /* last free slot in the stack */
StkId stack;  /* stack base */

#endif