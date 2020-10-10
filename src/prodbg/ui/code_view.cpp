#include "code_view.h"
#include <QtCore/QDebug>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtGui/QPainter>
#include <QtGui/QTextBlock>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include "backend/backend_requests_interface.h"
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

CodeView::CodeView(BreakpointModel* breakpoints, QWidget* parent)
    : edbee::TextEditorWidget(parent) {
    m_margin_delegate = new BreakpointDelegate;
    m_margin_delegate->m_breakpoints = breakpoints;
    textMarginComponent()->setDelegate(m_margin_delegate);
    controller()->setReadonly(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CodeView::load_file(const QString& filename) {
    // TODO: Configure font
#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif
    edbee::TextDocumentSerializer serializer(textDocument());
    QFile file(filename);
    if (!serializer.load(&file)) {
        qDebug() << "failed to load file";
    }

    auto grammar_manager = edbee::Edbee::instance()->grammarManager();
    auto grammar = grammar_manager->detectGrammarWithFilename(filename);
    textDocument()->setLanguageGrammar(grammar);
    textScrollArea()->enableShadowWidget(false);
    textDocument()->config()->setFont(font);

    m_source_file = filename;
    m_margin_delegate->m_filename = filename;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeView::~CodeView() {
    write_settings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::reload() {
    int currentLine = get_current_line();
    load_file(m_source_file);
    set_line(currentLine);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::file_change(const QString filename) {
    /*
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, QStringLiteral("File has been changed"),
                                  QStringLiteral("File %1 was changed, Reload?").arg(filename),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (reply != QMessageBox::Yes)
        return;

    QFile f(filename);

    if (!f.exists())
        return;

    int currentLine = get_current_line();

    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    setPlainText(ts.readAll());

    set_line(currentLine);

    // BUG: We need to readd the file here as it seems the watcher thinks it has
    // been deleted (even if just changed)
    //      so we only get one notification of a change so when doing a re-add
    //      here we get correct notifications again

    m_fileWatcher->addPath(filename);
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CodeView::get_current_line() {
    auto doc = textDocument();
    auto sel = controller()->textSelection();

    for (int i = 0, cnt = sel->rangeCount(); i < cnt; ++i) {
        edbee::TextRange& range = sel->range(i);
        return doc->lineFromOffset(range.caret());
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::set_line(int line) {
    edbee::TextEditorController* c = controller();
    c->moveCaretTo(line - 1, 0, false);
    c->scrollCaretVisible();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::session_ended() {
    m_current_source_line = -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::set_exception_line(int line) {
    m_current_source_line = line;
    set_line(line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::read_settings() {
    /*
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeView"));
    m_source_file = settings.value(QStringLiteral("Sourcefile")).toString();
    settings.endGroup();
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::write_settings() {
    /*
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeView"));
    settings.setValue(QStringLiteral("Sourcefile"), m_source_file);
    settings.endGroup();
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
