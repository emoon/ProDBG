#pragma once

#include "Backend/IBackendRequests.h"
#include <QPointer>
#include <QTabWidget>

namespace prodbg {

class BreakpointModel;
class IBackendRequests;

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

    Q_SLOT void toggleSourceAsm();

private:
    BreakpointModel* m_breakpoints;
    QPointer<IBackendRequests> m_interface;
    QVector<QString> m_files;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void CodeViews::setBreakpointModel(BreakpointModel* breakpoints)
{
    m_breakpoints = breakpoints;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void CodeViews::setBackendInterface(IBackendRequests* iface)
{
    m_interface = iface;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
