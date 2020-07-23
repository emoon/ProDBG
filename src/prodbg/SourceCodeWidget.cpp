#include "SourceCodeWidget.h"
#include <QDebug>
#include <QFile>
#include <QPainter>
#include "BreakpointModel.h"
#include "edbee/edbee.h"
#include "edbee/io/textdocumentserializer.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textgrammar.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textlinedata.h"
#include "edbee/views/components/textmargincomponent.h"
#include "edbee/views/texteditorscrollarea.h"
#include "edbee/views/textselection.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class BreakpointModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BreakpointDelegate : public edbee::TextMarginComponentDelegate {
public:
    int widthBeforeLineNumber() { return 30; }

    void renderAfter(QPainter* painter, int start_line, int end_line, int width, int line_height) {
        painter->setBrush(Qt::red);
        painter->setRenderHint(QPainter::Antialiasing);

        for (int line = start_line; line <= end_line; ++line) {
            int y = line * line_height;

            if (m_breakpoints->has_breakpoint_file_line(m_filename, line)) {
                painter->drawEllipse(4, y + 1, line_height - 2, line_height - 2);
            }
        }
    }

    QString m_filename;
    BreakpointModel* m_breakpoints;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SourceCodeWidget::SourceCodeWidget(BreakpointModel* breakpoints, QWidget* parent) : m_breakpoints(breakpoints) {
    m_margin_delegate = new BreakpointDelegate;
    m_editor = new edbee::TextEditorWidget(parent);

    m_margin_delegate->m_breakpoints = breakpoints;
    m_editor->textMarginComponent()->setDelegate(m_margin_delegate);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SourceCodeWidget::load_file(const QString& filename) {
    // TODO: Configure font
#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    edbee::TextDocumentSerializer serializer(m_editor->textDocument());
    QFile file(filename);
    if (!serializer.load(&file)) {
        qDebug() << "failed to load file";
    }

    auto grammar_manager = edbee::Edbee::instance()->grammarManager();
    auto grammar = grammar_manager->detectGrammarWithFilename(filename);
    m_editor->textDocument()->setLanguageGrammar(grammar);
    m_editor->textScrollArea()->enableShadowWidget(false);
    m_editor->textDocument()->config()->setFont(font);

    m_filename = filename;
    m_margin_delegate->m_filename = filename;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SourceCodeWidget::program_counter_changed(const IBackendRequests::ProgramCounterChange& pc) {
    if (pc.filename != QStringLiteral("") && m_filename != pc.filename) {
        m_editor->textDocument()->lineDataManager()->clear();
        load_file(pc.filename);
    }

    edbee::TextEditorController* controller = m_editor->controller();
    controller->moveCaretTo(pc.line - 1, 0, false);
    controller->scrollCaretVisible();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SourceCodeWidget::toggle_breakpoint_current_line() {
    auto doc = m_editor->textDocument();
    auto sel = m_editor->controller()->textSelection();

    for (int i = 0, cnt = sel->rangeCount(); i < cnt; ++i) {
        edbee::TextRange& range = sel->range(i);
        int line = doc->lineFromOffset(range.caret());

        m_breakpoints->toggle_file_line_breakpoint(m_filename, line);
        m_editor->update();
        // how to deal with more lines?
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SourceCodeWidget::~SourceCodeWidget() {
    printf("ending..\n");
    delete m_editor;
    delete m_margin_delegate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
