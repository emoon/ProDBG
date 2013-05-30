#pragma once

#include <QObject>
#include <QTimer>
#include <QStringList>
#include "ProDBGAPI.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5DebuggerThread : public QObject
{
	Q_OBJECT

public:
	Qt5DebuggerThread();

public slots:
    void start();
	void update();
	void tryAddBreakpoint(const char*, int line);
	void tryStartDebugging(const char* filename, PDBreakpointFileLine* breakpoints, int bpCount);
	void tryStep();
 
signals:
    void finished();
	void addBreakpointUI(const char* filename, int line, int id);
	void sendDebugDataState(PDDebugDataState* state);

private:

	PDDebugDataState m_debugDataState;

	PDDebugPlugin* m_debuggerPlugin;
	const char* m_executable;
	void* m_pluginData;
	int m_oldLine; // temp hack
	QTimer m_timer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

