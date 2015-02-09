#ifndef _PRODBGSCRIPT_H_
#define _PRODBGSCRIPT_H_

#include "pd_common.h"

#ifdef _cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct lua_State;
typedef lua_State PDScriptState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDScriptCallState
{
    char* funcName;
    int inputCount;
    int outputCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDScriptVariantType
{
    
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDScriptCallVariant
{
    enum
    {
        PDScriptCallVariantType_Integer,
        PDScriptCallVariantType_Boolean,
        PDScriptCallVariantType_Float,
        PDScriptCallVariantType_StringPtr,
    } typeId;

    union
    {
        int asInteger;
        bool asBoolean;
        float asFloat;
        char* asStringPtr; // Null terminated
    };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDScriptCallSignature
{
    char function[64];

    int argumentCount;
    int returnCount;

    PDScriptCallVariant* argumentData;
    struct PDScriptCallVariant* returnData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int  PDScript_createState(PDScriptState** state);
void PDScript_destroyState(PDScriptState** state);

int PDScript_loadFile(PDScriptState* state, const char* scriptFile);
int PDScript_loadString(PDScriptState* state, const char* scriptString);
int PDScript_loadBuffer(PDScriptState* state, const char* scriptBuffer, int size, const char* name, const char* mode);

int PDScript_primeCall(PDScriptState* scriptState, struct PDScriptCallState* callState, const char* funcName);
int PDScript_executeCall(PDScriptState* scriptState, struct PDScriptCallState* callState);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _cplusplus
}
#endif

#endif

