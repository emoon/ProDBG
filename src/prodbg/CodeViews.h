#pragma once

#include "Backend/IBackendRequests.h"
#include <QPointer>
#include <QTabWidget>

namespace prodbg {

class BreakpointModel;
class IBackendRequests;
class DisassemblyView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CodeViews : public QTabWidget
{
public:
    CodeViews(BreakpointModel* breakpoints, QWidget* parent = 0);
    virtual ~CodeViews();
    void setBreakpointModel(BreakpointModel* breakpoints);

    void reloadCurrentFile();
    void toggleBreakpoint();
    void openFile(const QString& filename, bool setActive);
    void setBackendInterface(IBackendRequests* iface);

    Q_SLOT void programCounterChanged(const IBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void sessionEnded();
    Q_SLOT void toggleSourceAsm();

private:
    Q_SLOT void closeTab(int index);

    enum Mode
    {
        SourceView,
        Disassembly,
    };

    void readSettings();
    void writeSettings();

    void setDisassemblyMode();
    void setSourceMode();

    Mode m_mode = SourceView;
    int m_oldIndex = 0;

    // If we get a PC with no source we will auto-switch to disassembly but if we are in disassembly
    // and get source/line again we switch back
    bool m_wasInSourceView = false;

    DisassemblyView* m_disassemblyView = nullptr;
    BreakpointModel* m_breakpoints = nullptr;
    QPointer<IBackendRequests> m_interface;
    QVector<QString> m_files;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void CodeViews::setBreakpointModel(BreakpointModel* breakpoints)
{
    m_breakpoints = breakpoints;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
