#pragma once

#include <angelscript.h>

#include <string>
#include <vector>
#include <map>

namespace asdbg {

class Engine {
public:
    // Register callbacks to handle to-string conversions of application types
    typedef std::string (*ToStringFunc)(void* object, bool expandMembers, Engine* engine);
    void registerToStringFunc(const asIObjectType* type, ToStringFunc callback);

public:
    Engine();
    ~Engine();

    // Processing
    void tick();

    // User interaction
    void takeCommands(asIScriptContext* context);
    void output(const std::string& text);

    // Line callback invoked by context
    void lineCallback(asIScriptContext* context);

    // Commands
    void addFileBreakpoint(const std::string& file, int line);
    void addFuncBreakpoint(const std::string& func);

    void listBreakpoints();
    void listLocalVariables(asIScriptContext* context);
    void listGlobalVariables(asIScriptContext* context);
    void listMemberProperties(asIScriptContext* context);
    void listStatistics(asIScriptContext* context);

    void printCallstack(asIScriptContext* context);
    void printValue(const std::string& expression, asIScriptContext* context);

    // Helpers
    bool interpretCommand(const std::string& command, asIScriptContext* context);
    bool checkBreakPoint(asIScriptContext* context);

    std::string toString(void* value, asUINT typeId, bool expandMembers, asIScriptEngine* engine);

protected:
    enum DebugAction {
        DebugAction_Continue, /// Continue until next break point
        DebugAction_StepInto, /// Stop at next instruction
        DebugAction_StepOver, /// Stop at next instruction, skipping called functions
        DebugAction_StepOut,  /// Run until returning from current function
    };

    struct Breakpoint {
        Breakpoint(const std::string& file, int line, bool func)
            : name(file)
            , lineNumber(line)
            , function(func)
            , needsAdjusting(true) {
        }

        std::string name;
        int lineNumber;
        bool function;
        bool needsAdjusting;
    };

protected:
    DebugAction m_debugAction;
    asUINT m_lastCommandAtStackLevel;
    asIScriptFunction* m_lastFunction;
    std::vector<Breakpoint> m_breakpoints;

    // Registered callbacks for converting objects to strings
    std::map<const asIObjectType*, ToStringFunc> m_toStringCallbacks;

    bool m_connected;
};

}