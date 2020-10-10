#pragma once

#include <QObject>
#include <QString>
#include "backend/backend_requests_interface.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace edbee {
    class TextEditorWidget;
    class TextMarginComponentDelegate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This Widget is used for showing source files, set breakpoints and for stepping of code
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class BreakpointModel;
class BreakpointDelegate;

class SourceCodeWidget : public QObject {
    Q_OBJECT
public:
    explicit SourceCodeWidget(BreakpointModel* breakpoints, QWidget* parent);
    void load_file(const QString& filename);
    ~SourceCodeWidget();

    Q_SLOT void program_counter_changed(const IBackendRequests::ProgramCounterChange& pc);

    void toggle_breakpoint_current_line();

    edbee::TextEditorWidget* m_editor = nullptr;
    BreakpointModel* m_breakpoints = nullptr;
    BreakpointDelegate* m_margin_delegate = nullptr;

    QString m_filename;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
