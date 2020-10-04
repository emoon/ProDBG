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
#include "Backend/IBackendRequests.h"
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
    : edbee::TextEditorWidget(parent),
      m_lineNumberArea(nullptr),
      m_fileWatcher(nullptr),
      m_disassemblyStart(0),
      m_disassemblyEnd(0) {

    m_margin_delegate = new BreakpointDelegate;
    m_margin_delegate->m_breakpoints = breakpoints;
    textMarginComponent()->setDelegate(m_margin_delegate);
    controller()->setReadonly(true);

    //m_fileWatcher = new QFileSystemWatcher(this);
    //connect(m_fileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChange(const QString)));

    //connect(this, SIGNAL(updateRequest(const QRect&, int)), this, SLOT(updateLineNumberArea(const QRect&, int)));
    //connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    /*
#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    setFont(font);

    readSettings();
    */

    toggleSourceFile();
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
    writeSettings();
    delete m_fileWatcher;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::openFile() {
    QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open Source File"));

    if (path.isEmpty()) {
        return;
    }

    load_file(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::reload() {
    int currentLine = getCurrentLine();
    load_file(m_source_file);
    set_line(currentLine);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::fileChange(const QString filename) {
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

    int currentLine = getCurrentLine();

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

/*
int CodeView::lineNumberAreaWidth() {
    int digits = 1;
    if (m_mode == Disassembly) {
        digits = m_addressWidth * 2;
    } else {
        int max = qMax(1, blockCount());
        while (max >= 10) {
            max /= 10;
            ++digits;
        }
    }

    // 20 + to give rom for breakpoint marker

    int space = 20 + 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void CodeView::updateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;
    QTextCursor cursor = textCursor();

    QString text = cursor.selectedText();

    QString string = cursor.block().text();

    QColor lineColor = QApplication::palette().highlight().color();

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = cursor;
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPalette pal = QApplication::palette();

#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), pal.alternateBase());

    painter.setFont(font);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();
    int width = m_lineNumberArea->width() - 4;
    int height = fontMetrics().height();

    int fontHeight = fontMetrics().height() - 2;

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(pal.text().color());

            painter.drawText(0, top, width, height, Qt::AlignRight, number);

            if (m_breakpoints->has_breakpoint_file_line(m_source_file, blockNumber + 1)) {
                painter.setBrush(Qt::red);
                painter.drawEllipse(4, top, fontHeight, fontHeight);
            }

            // Draw ugly arrow!

            if ((blockNumber + 1) == m_currentSourceLine) {
                float scale = fontHeight / 2.0f;
                float pos_x = 10.0f;
                float pos_y = top + scale;

                const QPointF points[7] = {
                    QPointF((0.0f * scale) + pos_x, (-0.5f * scale) + pos_y),
                    QPointF((0.5f * scale) + pos_x, (-0.5f * scale) + pos_y),
                    QPointF((0.5f * scale) + pos_x, (-1.0f * scale) + pos_y),

                    QPointF((1.0f * scale) + pos_x, (0.0f * scale) + pos_y),

                    QPointF((0.5f * scale) + pos_x, (1.0f * scale) + pos_y),
                    QPointF((0.5f * scale) + pos_x, (0.5f * scale) + pos_y),
                    QPointF((0.0f * scale) + pos_x, (0.5f * scale) + pos_y),
                };

                painter.setPen(Qt::yellow);
                painter.setBrush(Qt::yellow);
                painter.drawConvexPolygon(points, 7);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::step() {
    // g_debugSession->callAction(PDAction_step);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setMode(Mode mode) {
    m_mode = mode;

    switch (mode) {
        case Sourcefile:
        case Mixed:
            break;

        case Disassembly: {
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::updateDisassemblyCursor() {
    if (m_mode != Disassembly) {
        return;
    }

    for (int i = 0, count = m_disassemblyAdresses.count(); i < count; ++i) {
        if (m_disassemblyAdresses[i].address != m_currentPc) {
            continue;
        }

        set_line(i + 1);

        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::toggleDisassembly() {
    /*
    m_mode = Disassembly;
    // program_counter_changed(m_currentPc);
    setPlainText(m_disassemblyText);
    updateDisassemblyCursor();
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::toggleSourceFile() {
    /*
    m_mode = Sourcefile;
    setCenterOnScroll(false);
    setPlainText(m_sourceCodeData);

    if (m_currentSourceLine >= 0) {
        set_line(m_currentSourceLine);
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void CodeView::keyPressEvent(QKeyEvent* event)
{
    // TODO: Use proper actions from main menu instead

    if (event->key() == Qt::Key_Space) {
        if (m_mode == Sourcefile) {
            toggleDisassembly();
        } else {
            toggleSourceFile();
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::initDefaultSourceFile(const QString& filename) {
    m_source_file = filename;
    load_file(m_source_file);
    toggleSourceFile();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::readSourceFile(const QString& filename) {
    QFile f(filename);

    if (!f.exists())
        return;

    if (!m_source_file.isEmpty()) {
        m_fileWatcher->removePath(m_source_file);
    }

    m_source_file = filename;

    m_fileWatcher->addPath(QString(filename));

    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);

    m_sourceCodeData = ts.readAll();

    if (m_mode == Sourcefile) {
        textDocument()->setText(m_sourceCodeData);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CodeView::getCurrentLine() {
    auto doc = textDocument();
    auto sel = controller()->textSelection();

    for (int i = 0, cnt = sel->rangeCount(); i < cnt; ++i) {
        edbee::TextRange& range = sel->range(i);
        return doc->lineFromOffset(range.caret());
    }

    return -1;

    /*

    // + 1 due to 1 indexed
    bool added = m_breakpoints->toggleFileLineBreakpoint(m_source_file, line +
    1);

    if (added) {
        m_interface->beginAddFileLineBreakpoint(m_source_file, line);
    } else {
        m_interface->beginRemoveFileLineBreakpoint(m_source_file, line);
    }

    m_lineNumberArea->repaint();
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::set_line(int line) {
    edbee::TextEditorController* c = controller();
    c->moveCaretTo(line - 1, 0, false);
    c->scrollCaretVisible();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void CodeView::setBreakpointModel(BreakpointModel* breakpoints) {
    m_breakpoints = breakpoints;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void CodeView::set_backend_interface(IBackendRequests* iface)
{
    m_interface = iface;

    if (!iface) {
        return;
    }

    connect(m_interface, &IBackendRequests::endDisassembly, this,
&CodeView::endDisassembly); connect(m_interface,
&IBackendRequests::program_counter_changed, this,
&CodeView::program_counter_changed); connect(m_interface,
&IBackendRequests::sourceFileLineChanged, this,
&CodeView::sourceFileLineChanged);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::session_ended() {
    m_currentSourceLine = -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setExceptionLine(int line) {
    m_currentSourceLine = line;
    set_line(line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setFileLine(const QString& file, int line) {
    if (file != m_source_file) {
        load_file(file);
    }

    m_currentSourceLine = line;

    if (m_mode == Sourcefile) {
        set_line(line);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::readSettings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeView"));
    m_source_file = settings.value(QStringLiteral("Sourcefile")).toString();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::writeSettings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeView"));
    settings.setValue(QStringLiteral("Sourcefile"), m_source_file);
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
