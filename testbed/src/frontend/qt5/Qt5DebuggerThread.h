#pragma once

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <ProDBGAPI.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDDebugPlugin;

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5DebuggerThread : public QObject
{
	Q_OBJECT

public:
	Qt5DebuggerThread();
	PDDebugState getDebugState(void** data);

public slots:
    void start();
	void update();
	void tryAddBreakpoint(const char*, int line);
	void tryStartDebugging(const char* filename, PDBreakpointFileLine* breakpoints, int bpCount);
	void tryStep();
 
signals:
    void finished();
	void callUIthread();
	void addBreakpointUI(const char* filename, int line, int id);
    void setFileLine(const char* file, int line);
    void setCallStack(QStringList callstack);

private:

	PDBreakpointFileLine m_fileLine;

	PDDebugPlugin* m_debuggerPlugin;
	const char* m_executable;
	void* m_pluginData;
	QTimer m_timer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

