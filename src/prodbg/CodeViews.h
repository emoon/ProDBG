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
    CodeViews(QWidget* parent);
    virtual ~CodeViews();
    void setBreakpointModel(BreakpointModel* breakpoints);

    void toggleBreakpoint();
    void openFile(const QString& filename);
    void setBackendInterface(IBackendRequests* iface);

    Q_SLOT void programCounterChanged(const IBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void sourceFileLineChanged(const QString& filename, int line);
    Q_SLOT void toggleSourceAsm();

private:
    enum Mode
    {
        SourceView,
        Disassembly,
    };

    Mode m_mode = SourceView;
    int m_oldIndex = 0;

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
