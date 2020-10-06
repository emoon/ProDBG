#pragma once

#include <stdint.h>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include "Backend/IBackendRequests.h"
#include "edbee/texteditorwidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QThread;
class QFileSystemWatcher;
class QWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class BreakpointModel;
class BreakpointDelegate;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CodeView : public edbee::TextEditorWidget {
    Q_OBJECT

public:
    CodeView(BreakpointModel* breakpoints, QWidget* parent = 0);
    ~CodeView();

    void toggle_breakpoint_current_line();

    void reload();
    void load_file(const QString& file);

    void set_exception_line(int line);
    void session_ended();

    int get_current_line();

protected:
    void step();

    Q_SLOT void file_change(const QString filename);

    void set_line(int line);

    void read_settings();
    void write_settings();

    QPointer<IBackendRequests> m_interface;
    BreakpointModel* m_breakpoints = 0;

    QString m_source_file;
    int m_current_source_line = 0;

    BreakpointDelegate* m_margin_delegate = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
