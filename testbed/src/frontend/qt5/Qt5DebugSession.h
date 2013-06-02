#pragma once

#include <ProDBGAPI.h>
#include <QObject>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QWidget;
class QThread;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

class Qt5CodeEditor;
class Qt5Locals;
class Qt5CallStack;
class Qt5DebugOutput;
class Qt5DebuggerThread;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5DebugSession : public QObject
{
    Q_OBJECT;
public:

    static void createSession();

    Qt5DebugSession();

    // TODO: Do this more generic

    void addCodeEditor(Qt5CodeEditor* codeEditor);
    void addLocals(Qt5Locals* locals);
    void addCallStack(Qt5CallStack* callStack);
    void addDebugOutput(Qt5DebugOutput* debugOutput);

    void delCodeEditor(Qt5CodeEditor* codeEditor);
    void delLocals(Qt5Locals* locals);
    void delCallStack(Qt5CallStack* callStack);
    void delDebugOutput(Qt5DebugOutput* debugOutput);

    void begin(const char* executable);
    void step();

    bool hasLineBreakpoint(const char* filename, int line);
    bool getFilenameLine(const char** filename, int* line);

    bool addBreakpoint(const char* file, int line, int id);
    void delBreakpoint(int id);

    bool addBreakpointUI(const char* file, int line);
    void delBreakpointUI(int id);
    
private slots:
	// Called from the when a state change has happen that require UI update 
	void setDebugDataState(PDDebugDataState* state);

signals:
	void tryStartDebugging(const char* filename, PDBreakpointFileLine* breakpoints, int bpCount);
	void tryAddBreakpoint(const char* filename, int line);
	void tryStep();

private:

    QList<Qt5CodeEditor*> m_codeEditors;
    QList<Qt5Locals*> m_locals;
    QList<Qt5CallStack*> m_callStacks;
    QList<Qt5DebugOutput*> m_debugOutputs;

	PDDebugPlugin* m_debuggerPlugin;
	void* m_pluginData;
	QThread* m_threadRunner;

    Qt5DebuggerThread* m_debuggerThread;
    PDBreakpointFileLine* m_breakpoints;

    int m_breakpointCount;
    int m_breakpointMaxCount;
};

extern Qt5DebugSession* g_debugSession;

}

