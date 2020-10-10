#include "code_views.h"
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include "breakpoint_model.h"
#include "code_view.h"
#include "disassembly_view.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeViews::CodeViews(BreakpointModel* breakpoints, QWidget* parent) : QTabWidget(parent), m_breakpoints(breakpoints) {
    setTabsClosable(true);
    //m_disassemblyView = new DisassemblyView(nullptr);
    //m_disassemblyView->setBreakpointModel(breakpoints);
    //read_settings();

    connect(this, &QTabWidget::tabCloseRequested, this, &CodeViews::closeTab);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeViews::~CodeViews() {
    write_settings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::toggle_breakpoint() {
    const int index = currentIndex();

    printf("toggle breakpoint %d\n", index);

    if (index == 0 && tabText(0).indexOf(QStringLiteral("Disassembly")) == 0) {
        //m_disassemblyView->toggleBreakpoint();
    } else {
        CodeView* view = dynamic_cast<CodeView*>(widget(index));
        Q_ASSERT(view);

        const QString& filename = tabToolTip(index);

        int line = view->get_current_line();
        bool added = m_breakpoints->toggle_file_line_breakpoint(filename, line);

        if (m_interface) {
            if (added) {
                m_interface->request_add_file_line_breakpoint(filename, line);
            } else {
                m_interface->request_remove_file_line_breakpoint(filename, line);
            }
        }

        view->repaint();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::session_ended() {
    for (int i = 0, c = count(); i < c; ++i) {
        if (tabText(i).indexOf(QStringLiteral("Disassembly")) == 0) {
            continue;
        }

        CodeView* view = dynamic_cast<CodeView*>(widget(i));
        Q_ASSERT(view);

        view->session_ended();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::open_file(const QString& filename, bool setActive) {
    QFileInfo info(filename);

    CodeView* code_view = new CodeView(m_breakpoints);
    code_view->load_file(info.absoluteFilePath());
    //codeView->setBreakpointModel(m_breakpoints);

    int index = addTab(code_view, info.fileName());

    setTabToolTip(index, info.absoluteFilePath());

    if (setActive) {
        setCurrentIndex(index);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::reload_current_file() {
    const int index = currentIndex();

    if (tabText(index).indexOf(QStringLiteral("Disassembly")) == 0) {
        return;
    }

    CodeView* view = dynamic_cast<CodeView*>(widget(index));
    Q_ASSERT(view);

    view->reload();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::program_counter_changed(const IBackendRequests::ProgramCounterChange& pc) {
    qDebug() << "CodeViews::program_counter_changed";
    qDebug() << pc.line;
    qDebug() << pc.filename;

    //m_disassemblyView->updatePc(pc.pc);

    // No source/line for the PC so toggle to disassmbly mode if we already
    // aren't there

    if (pc.line == -1) {
        if (m_mode == SourceView) {
            m_was_in_source_view = true;
        }

        //setDisassemblyMode();
    } else {
        /*
        if (m_mode == Disassembly && m_was_in_source_view) {
            toggleSourceAsm();
            m_was_in_source_view = false;
        }
        */

        int tabs_count = count();

        // Search for the source file if we already have it open activate it
        for (int i = 0; i < tabs_count; ++i) {
            const QString& filename = tabToolTip(i);

            if (filename != pc.filename) {
                continue;
            }

            CodeView* view = dynamic_cast<CodeView*>(widget(i));
            Q_ASSERT(view);
            view->set_exception_line(pc.line);

            if (m_mode == SourceView) {
                setCurrentIndex(i);
            }

            return;
        }

        open_file(pc.filename, m_mode == SourceView);

        CodeView* view = dynamic_cast<CodeView*>(widget(tabs_count));
        view->set_exception_line(pc.line);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::set_backend_interface(IBackendRequests* iface) {
    m_interface = iface;

    //m_disassemblyView->set_backend_interface(iface);

    if (iface) {
        connect(m_interface, &IBackendRequests::program_counter_changed, this, &CodeViews::program_counter_changed);
        connect(m_interface, &IBackendRequests::session_ended, this, &CodeViews::session_ended);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toggles to the disassembly view. This view is (right now) always placed to
// the left most of the tabs

/*
void CodeViews::toggleSourceAsm() {
    if (m_mode == SourceView) {
        setDisassemblyMode();
    } else {
        setSourceMode();
    }
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void CodeViews::setDisassemblyMode() {
    const int tabs_count = count();

    if (tabs_count == 0) {
        addTab(m_disassemblyView, QStringLiteral("Disassembly"));
    } else {
        // Check if the first tab is disassembly, if not insert it at that
        // position
        if (tabText(0).indexOf(QStringLiteral("Disassembly")) != 0) {
            insertTab(0, m_disassemblyView, QStringLiteral("Disassembly"));
        }
    }

    m_old_index = currentIndex();

    setCurrentIndex(0);

    m_mode = Disassembly;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void CodeViews::set_source_mode() {
    const int tabs_count = count();

    if (m_old_index <= tabs_count) {
        setCurrentIndex(m_old_index);
    }

    m_mode = SourceView;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::closeTab(int index) {
    removeTab(index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::read_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeViews"));

    int size = settings.beginReadArray(QStringLiteral("sourceFiles"));

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString filename = settings.value(QStringLiteral("sourceFile")).toString();
        open_file(filename, false);
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::write_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeViews"));
    settings.beginWriteArray(QStringLiteral("sourceFiles"));

    int index = 0;

    for (int i = 0, c = count(); i < c; ++i) {
        if (tabText(i).indexOf(QStringLiteral("Disassembly")) == 0) {
            continue;
        }

        QString filename = tabToolTip(i);

        settings.setArrayIndex(index);
        settings.setValue(QStringLiteral("sourceFile"), filename);

        index += 1;
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg

