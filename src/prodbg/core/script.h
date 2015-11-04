#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct lua_State;
typedef lua_State ScriptState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ScriptCallState {
    char* funcName;
    int inputCount;
    int outputCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum ScriptVariantType {

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ScriptCallVariant {
    enum {
        ScriptCallVariantType_Integer,
        ScriptCallVariantType_Boolean,
        ScriptCallVariantType_Float,
        ScriptCallVariantType_StringPtr,
    } typeId;

    union {
        int asInteger;
        bool asBoolean;
        float asFloat;
        char* asStringPtr; // Null terminated
    };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ScriptCallSignature {
    char function[64];

    int argumentCount;
    int returnCount;

    ScriptCallVariant* argumentData;
    struct ScriptCallVariant* returnData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int  Script_createState(ScriptState** state);
void Script_destroyState(ScriptState** state);

int Script_loadFile(ScriptState* state, const char* scriptFile);
int Script_loadString(ScriptState* state, const char* scriptString);
int Script_loadBuffer(ScriptState* state, const char* scriptBuffer, int size, const char* name, const char* mode);

int Script_primeCall(ScriptState* scriptState, struct ScriptCallState* callState, const char* funcName);
int Script_executeCall(ScriptState* scriptState, struct ScriptCallState* callState);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

