#pragma once

#include <QtCore/QPointer>
#include <QtWidgets/QTabWidget>
#include "backend/backend_requests_interface.h"

class BreakpointModel;
class PDIBackendRequests;
class DisassemblyView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CodeViews : public QTabWidget {
public:
    CodeViews(BreakpointModel* breakpoints, QWidget* parent = 0);
    virtual ~CodeViews();

    void set_breakpoint_model(BreakpointModel* breakpoints);

    void reload_current_file();
    void toggle_breakpoint();
    void set_backend_interface(PDIBackendRequests* iface);

    Q_SLOT void open_file(const QString& filename, bool setActive);
    Q_SLOT void program_counter_changed(const PDIBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void session_ended();

private:
    Q_SLOT void closeTab(int index);

    enum Mode {
        SourceView,
        Disassembly,
    };

    void read_settings();
    void write_settings();

    Mode m_mode = SourceView;
    int m_oldIndex = 0;

    // DisassemblyView* m_disassemblyView = nullptr;
    BreakpointModel* m_breakpoints = nullptr;
    QPointer<PDIBackendRequests> m_interface;
    QVector<QString> m_files;

    bool m_was_in_source_view = true;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void CodeViews::set_breakpoint_model(BreakpointModel* breakpoints) {
    m_breakpoints = breakpoints;
}
