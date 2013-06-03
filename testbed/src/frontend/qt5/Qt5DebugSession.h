#pragma once

#include <ProDBGAPI.h>
#include <QObject>
#include <QVector>

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

    void delCodeEditor(Qt5CodeEditor* codeEditor);
    void delLocals(Qt5Locals* locals);
    void delCallStack(Qt5CallStack* callStack);

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
	//void setDebugDataState(PDDebugDataState* state);
	void setDebugState(PDSerializeRead* reader);

signals:
	void tryStartDebugging();
	void tryStep();

private:

    QList<Qt5CodeEditor*> m_codeEditors;
    QList<Qt5Locals*> m_locals;
    QList<Qt5CallStack*> m_callStacks;

	PDBackendPlugin* m_debuggerPlugin;
	void* m_pluginData;
	QThread* m_threadRunner;

	// Not sure how we should deal with this but lets keep it
	// here like a cache.
	// Maybe we should have this inside a more general cache

	struct BreakpointFileLine
	{
		const char* filename;
		int line;
		int id;
	};
		
	//QVector<BreakpointFileLine> m_breakpoints;
	BreakpointFileLine* m_breakpoints;
 
    int m_breakpointCount;
    int m_breakpointMaxCount;

    Qt5DebuggerThread* m_debuggerThread;
};

extern Qt5DebugSession* g_debugSession;

}

