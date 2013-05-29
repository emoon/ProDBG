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

class Qt5DebuggerThread;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5DebugSession : public QObject
{
    Q_OBJECT;
public:

    static void createSession();

    Qt5DebugSession();

    void connectWidget(QObject* widget);
    void disconnectWidget(QObject* widget);
    void begin(const char* executable);
    void step();

    bool hasLineBreakpoint(const char* filename, int line);
    bool getFilenameLine(const char** filename, int* line);

    bool addBreakpoint(const char* file, int line, int id);
    void delBreakpoint(int id);

    bool addBreakpointUI(const char* file, int line);
    void delBreakpointUI(int id);
    
private slots:
	// Called from the debugging thread when a step finished
	void stepFinished();

signals:
	void tryStartDebugging(const char* filename, PDBreakpointFileLine* breakpoints, int bpCount);
	void tryAddBreakpoint(const char* filename, int line);

private:

	PDDebugPlugin* m_debuggerPlugin;
	void* m_pluginData;
	PDDebugState m_debugState;
	QThread* m_threadRunner;

    Qt5DebuggerThread* m_debuggerThread;
    PDBreakpointFileLine* m_breakpoints;

    int m_breakpointCount;
    int m_breakpointMaxCount;
};

extern Qt5DebugSession* g_debugSession;

}

