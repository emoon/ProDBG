#include "asdbg.h"
#include "asdbg_engine.h"

#include <pd_backend.h>
#include <pd_remote.h>

#include <iostream>  // cout
#include <sstream>   // stringstream
#include <stdlib.h>  // atoi
#include <assert.h>  // assert
#include <stdarg.h>  // va_arg
#include <string.h>  // memset

#ifdef _WIN32
#include <windows.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_out(const char* format, ...) {
    va_list ap;

    va_start(ap, format);
#if defined(_WIN32)
    {
        char buffer[2048];
        vsprintf(buffer, format, ap);
        OutputDebugStringA(buffer);
    }
#else
    vprintf(format, ap);
#endif
    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct AngelScriptDebugger {
    asdbg::Engine* engine;
    asIScriptContext* context;
    int runState;

} AngelScriptDebugger;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AngelScriptDebugger* g_debugger;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc) {
    (void)serviceFunc;

    g_debugger = (AngelScriptDebugger*)malloc(sizeof(AngelScriptDebugger));    // this is a bit ugly but for this plugin we only have one instance
    memset(g_debugger, 0, sizeof(AngelScriptDebugger));

    g_debugger->runState = PDDebugState_Running;

    return g_debugger;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData) {
    free(userData);
    g_debugger = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer) {
    //int event = 0;

    (void)reader;

    AngelScriptDebugger* debugger = (AngelScriptDebugger*)userData;

    /*processEvents(plugin, reader, writer);

       doAction(plugin, action);

       if (plugin->state == PDDebugState_running)
        updateLLDBEvent(plugin, writer);*/

    //doAction(debugger, action, writer);

    uint32_t event;
    while ((event = PDRead_get_event(reader)) != 0) {
        switch (event) {
            case PDEventType_GetCallstack:
                log_out("GetCallstack!\n");
                //debugger->engine
                //getCallstack(reader, writer);
                break;

            case PDEventType_GetLocals:
                log_out("GetLocals!\n");
                //getLocals(reader, writer);
                break;
        }
    }

    return PDDebugState(debugger->runState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin s_asdebuggerPlugin =
{
    "AngelScript",
    createInstance,
    destroyInstance,
    0,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace asdbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::registerToStringFunc(const asIObjectType* type, ToStringFunc callback) {
    if (m_toStringCallbacks.find(type) == m_toStringCallbacks.end())
        m_toStringCallbacks.insert(std::map<const asIObjectType*, ToStringFunc>::value_type(type, callback));
}

Engine::Engine()
    : m_debugAction(DebugAction_Continue)
    , m_lastCommandAtStackLevel(0)
    , m_lastFunction(nullptr)
    , m_connected(false) {
    if (!PDRemote_create(&s_asdebuggerPlugin, 0)) {
        output("Unable to setup debugger connection\n");
        return;
    }

    //s_asdebuggerPlugin->

    m_connected = true;
}

Engine::~Engine() {

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::tick() {
    if (!PDRemote_isConnected()) {
        //if ((instructions & 127) == 0)
        PDRemote_update(0);

        return;
    }

    PDRemote_update(1);

#if 0
    if (g_debugger->runState == PDDebugState_stopException) {
        for (;;) {
            switch (g_debugger->runState) {
                case PDDebugState_running:
                {
                    printf("AS: start running\n");
                    goto go_on; // start running as usually
                }
                case PDDebugState_trace:
                {
                    printf("trace\n");
                    step6502(1);
                    g_debugger->runState = PDDebugState_stopException;
                    break;
                }

                default:
                    break;
            }

            PDRemote_update(1);
        }
    }else {
        updateDebugger();
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::takeCommands(asIScriptContext* context) {

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::output(const std::string& text) {
    // By default we just output to stdout
    std::cout << text;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::lineCallback(asIScriptContext* context) {

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::addFileBreakpoint(const std::string& file, int line) {
    // Store just file name, not entire path
    size_t r = file.find_last_of("\\/");
    std::string actual;
    if (r != std::string::npos)
        actual = file.substr(r + 1);
    else
        actual = file;

    // Trim the file name
    size_t b = actual.find_first_not_of(" \t");
    size_t e = actual.find_last_not_of(" \t");
    actual = actual.substr(b, e != std::string::npos ? e - b + 1 : std::string::npos);

    std::stringstream s;
    s << "Setting break point in file '" << actual << "' at line " << line << std::endl;
    output(s.str());

    Breakpoint bp(actual, line, false);
    m_breakpoints.push_back(bp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::addFuncBreakpoint(const std::string& func) {
    // Trim the function name
    size_t b = func.find_first_not_of(" \t");
    size_t e = func.find_last_not_of(" \t");
    std::string actual = func.substr(b, e != std::string::npos ? e - b + 1 : std::string::npos);

    std::stringstream s;
    s << "Adding deferred break point for function '" << actual << "'" << std::endl;
    output(s.str());

    Breakpoint bp(actual, 0, true);
    m_breakpoints.push_back(bp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::listBreakpoints() {
    // List all break points
    std::stringstream s;
    for (size_t breakpoint = 0; breakpoint < m_breakpoints.size(); ++breakpoint) {
        if (m_breakpoints[breakpoint].function)
            s << breakpoint << " - " << m_breakpoints[breakpoint].name << std::endl;
        else
            s << breakpoint << " - " << m_breakpoints[breakpoint].name << ":" << m_breakpoints[breakpoint].lineNumber << std::endl;
    }

    output(s.str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::listLocalVariables(asIScriptContext* context) {
    if (context == nullptr) {
        output("No script is running\n");
        return;
    }

    asIScriptFunction* func = context->GetFunction();
    if (!func)
        return;

    std::stringstream s;
    for (asUINT variable = 0; variable < func->GetVarCount(); ++variable) {
        if (context->IsVarInScope(variable))
            s << func->GetVarDecl(variable) << " = " << toString(context->GetAddressOfVar(variable), context->GetVarTypeId(variable), false, context->GetEngine()) << std::endl;
    }

    output(s.str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::listGlobalVariables(asIScriptContext* context) {
    if (context == nullptr) {
        output("No script is running\n");
        return;
    }

    // Determine the current module from the function
    asIScriptFunction* func = context->GetFunction();
    if (!func)
        return;

    asIScriptModule* mod = func->GetModule();
    if (!mod)
        return;

    std::stringstream s;
    for (asUINT n = 0; n < mod->GetGlobalVarCount(); n++) {
        int typeId = 0;
        mod->GetGlobalVar(n, 0, 0, &typeId);
        s << mod->GetGlobalVarDeclaration(n) << " = " << toString(mod->GetAddressOfGlobalVar(n), typeId, false, context->GetEngine()) << std::endl;
    }

    output(s.str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::listMemberProperties(asIScriptContext* context) {
    if (context == nullptr) {
        output("No script is running\n");
        return;
    }

    if (void* ptr = context->GetThisPointer()) {
        std::stringstream s;
        s << "this = " << toString(ptr, context->GetThisTypeId(), true, context->GetEngine()) << std::endl;
        output(s.str());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::listStatistics(asIScriptContext* context) {
    if (context == nullptr) {
        output("No script is running\n");
        return;
    }

    asIScriptEngine* engine = context->GetEngine();

    asUINT gcCurrSize, gcTotalDestr, gcTotalDet, gcNewObjects, gcTotalNewDestr;
    engine->GetGCStatistics(&gcCurrSize, &gcTotalDestr, &gcTotalDet, &gcNewObjects, &gcTotalNewDestr);

    std::stringstream s;
    s << "Garbage collector:" << std::endl;
    s << " current size:          " << gcCurrSize << std::endl;
    s << " total destroyed:       " << gcTotalDestr << std::endl;
    s << " total detected:        " << gcTotalDet << std::endl;
    s << " new objects:           " << gcNewObjects << std::endl;
    s << " new objects destroyed: " << gcTotalNewDestr << std::endl;

    output(s.str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::printCallstack(asIScriptContext* context) {
    if (context == nullptr) {
        output("No script is running\n");
        return;
    }

    std::stringstream s;
    const char* file = nullptr;
    int line = 0;
    for (asUINT entry = 0; entry < context->GetCallstackSize(); ++entry) {
        line = context->GetLineNumber(entry, 0, &file);
        s << file << ":" << line << "; " << context->GetFunction(entry)->GetDeclaration() << std::endl;
    }

    output(s.str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::printValue(const std::string& expression, asIScriptContext* context) {
    if (context == nullptr) {
        output("No script is running\n");
        return;
    }

    asIScriptEngine* engine = context->GetEngine();

    int len = 0;
    asETokenClass token = engine->ParseToken(expression.c_str(), 0, &len);

    // GW-TODO: If the expression starts with :: we should only look for global variables
    // GW-TODO: If the expression starts with identifier followed by ::, then use that as namespace
    if (token == asTC_IDENTIFIER) {
        std::string name(expression.c_str(), len);

        // Find the variable
        void* ptr = nullptr;
        int typeId = 0;

        asIScriptFunction* func = context->GetFunction();
        if (!func)
            return;

        // We start from the end, in case the same name is reused in different scopes
        for (asUINT variable = func->GetVarCount(); variable-- > 0;) {
            if (context->IsVarInScope(variable) && name == context->GetVarName(variable)) {
                ptr = context->GetAddressOfVar(variable);
                typeId = context->GetVarTypeId(variable);
                break;
            }
        }

        // Look for class members, if we're in a class method
        if (!ptr && func->GetObjectType()) {
            if (name == "this") {
                ptr = context->GetThisPointer();
                typeId = context->GetThisTypeId();
            }else {
                asIObjectType* type = engine->GetObjectTypeById(context->GetThisTypeId());
                for (asUINT prop = 0; prop < type->GetPropertyCount(); ++prop) {
                    const char* propName = nullptr;
                    int offset = 0;
                    bool isReference = false;

                    type->GetProperty(prop, &propName, &typeId, 0, &offset, &isReference);
                    if (name == propName) {
                        ptr = (void*)(((asBYTE*)context->GetThisPointer()) + offset);
                        if (isReference)
                            ptr = *(void**)ptr;
                        break;
                    }
                }
            }
        }

        // Look for global variables
        if (!ptr) {
            if (asIScriptModule* mod = func->GetModule()) {
                for (asUINT variable = 0; variable < mod->GetGlobalVarCount(); ++variable) {
                    // GW-TODO: Handle namespace too
                    const char* varName = nullptr, *nameSpace = nullptr;
                    mod->GetGlobalVar(variable, &varName, &nameSpace, &typeId);
                    if (name == varName) {
                        ptr = mod->GetAddressOfGlobalVar(variable);
                        break;
                    }
                }
            }
        }

        if (ptr) {
            // GW-TODO: If there is a . after the identifier, check for members

            std::stringstream s;
            s << toString(ptr, typeId, true, engine) << std::endl;
            output(s.str());
        }
    }else {
        output("Invalid expression. Expected identifier\n");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Engine::interpretCommand(const std::string& command, asIScriptContext* context) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Engine::checkBreakPoint(asIScriptContext* context) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Engine::toString(void* value, asUINT typeId, bool expandMembers, asIScriptEngine* engine) {
    if (value == 0)
        return "<null>";

    std::stringstream s;

    if (typeId == asTYPEID_VOID)
        return "<void>";
    else if (typeId == asTYPEID_BOOL)
        return *(bool*)value ? "true" : "false";
    else if (typeId == asTYPEID_INT8)
        s << (int)*(signed char*)value;
    else if (typeId == asTYPEID_INT16)
        s << (int)*(signed short*)value;
    else if (typeId == asTYPEID_INT32)
        s << *(signed int*)value;
    else if (typeId == asTYPEID_INT64)
        s << *(asINT64*)value;
    else if (typeId == asTYPEID_UINT8)
        s << (unsigned int)*(unsigned char*)value;
    else if (typeId == asTYPEID_UINT16)
        s << (unsigned int)*(unsigned short*)value;
    else if (typeId == asTYPEID_UINT32)
        s << *(unsigned int*)value;
    else if (typeId == asTYPEID_UINT64)
        s << *(asQWORD*)value;
    else if (typeId == asTYPEID_FLOAT)
        s << *(float*)value;
    else if (typeId == asTYPEID_DOUBLE)
        s << *(double*)value;
    else if ((typeId & asTYPEID_MASK_OBJECT) == 0) {
        // The type is an enum
        s << *(asUINT*)value;

        // Check if the value matches one of the defined enums
        for (int enumIndex = engine->GetEnumValueCount(typeId); enumIndex-- > 0;) {
            int enumVal;
            const char* enumName = engine->GetEnumValueByIndex(typeId, enumIndex, &enumVal);
            if (enumVal == *(int*)value) {
                s << ", " << enumName;
                break;
            }
        }
    }else if (typeId & asTYPEID_SCRIPTOBJECT) {
        // Dereference handles, so we can see what it points to
        if (typeId & asTYPEID_OBJHANDLE)
            value = *(void**)value;

        asIScriptObject* obj = (asIScriptObject*)value;

        // Print the address of the object
        s << "{" << obj << "}";

        // Print the members
        if (obj && expandMembers) {
            asIObjectType* type = obj->GetObjectType();
            for (asUINT prop = 0; prop < obj->GetPropertyCount(); ++prop)
                s << std::endl << "  " << type->GetPropertyDeclaration(prop) << " = " << toString(obj->GetAddressOfProperty(prop), obj->GetPropertyTypeId(prop), false, engine);
        }
    }else {
        // Dereference handles, so we can see what it points to
        if (typeId & asTYPEID_OBJHANDLE)
            value = *(void**)value;

        // Print the address for reference types so it will be
        // possible to see when handles point to the same object
        asIObjectType* type = engine->GetObjectTypeById(typeId);
        if (type->GetFlags() & asOBJ_REF)
            s << "{" << value << "}";

        if (value) {
            // Check if there is a registered to-string callback
            std::map<const asIObjectType*, ToStringFunc>::iterator it = m_toStringCallbacks.find(type);
            if (it == m_toStringCallbacks.end()) {
                // If the type is a template instance, there might be a
                // to-string callback for the generic template type
                if (type->GetFlags() & asOBJ_TEMPLATE) {
                    asIObjectType *tmplType = engine->GetObjectTypeByName(type->GetName());
                    it = m_toStringCallbacks.find(tmplType);
                }
            }

            if (it != m_toStringCallbacks.end()) {
                if (type->GetFlags() & asOBJ_REF)
                    s << std::endl;

                // Invoke the callback to get the string representation of this type
                std::string str = it->second(value, expandMembers, this);
                s << str;
            }else {
                // GW-TODO: Value types can have their properties expanded by default
            }
        }
    }

    return s.str();
}

}
