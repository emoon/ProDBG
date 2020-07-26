#pragma once

#include <QtCore/QPointer>
#include <QtWidgets/QTabWidget>
#include "Backend/IBackendRequests.h"

namespace prodbg {

class BreakpointModel;
class IBackendRequests;
class DisassemblyView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CodeViews : public QTabWidget {
public:
    CodeViews(BreakpointModel* breakpoints, QWidget* parent = 0);
    virtual ~CodeViews();

    void set_breakpoint_model(BreakpointModel* breakpoints);

    void reload_current_file();
    void toggle_breakpoint();
    void set_backend_interface(IBackendRequests* iface);

    Q_SLOT void open_file(const QString& filename, bool setActive);
    Q_SLOT void program_counter_changed(const IBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void session_ended();
    // Q_SLOT void toggleSourceAsm();

private:
    Q_SLOT void closeTab(int index);

    enum Mode {
        SourceView,
        Disassembly,
    };

    void read_settings();
    void write_settings();

    // void set_disassembly_mode();
    // void setSourceMode();

    Mode m_mode = SourceView;
    int m_oldIndex = 0;

    // If we get a PC with no source we will auto-switch to disassembly but if
    // we are in disassembly and get source/line again we switch back
    // bool m_wasInSourceView = false;

    // DisassemblyView* m_disassemblyView = nullptr;
    BreakpointModel* m_breakpoints = nullptr;
    QPointer<IBackendRequests> m_interface;
    QVector<QString> m_files;

    bool m_was_in_source_view = true;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void CodeViews::set_breakpoint_model(BreakpointModel* breakpoints) {
    m_breakpoints = breakpoints;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
