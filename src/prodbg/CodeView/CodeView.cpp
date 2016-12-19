#include "CodeView.h"
#include "Backend/IBackendRequests.h"
#include "BreakpointModel.h"
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QTextBlock>
#include <QTextStream>
#include <QDebug>
#include <QApplication>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeView* editor)
        : QWidget(editor)
        , m_codeEditor(editor)
    {
    }

    QSize sizeHint() const { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* event) { m_codeEditor->lineNumberAreaPaintEvent(event); }

private:
    CodeView* m_codeEditor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeView::CodeView(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(nullptr)
    , m_fileWatcher(nullptr)
    , m_disassemblyStart(0)
    , m_disassemblyEnd(0)
{
    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_lineNumberArea = new LineNumberArea(this);
    m_fileWatcher = new QFileSystemWatcher(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect&, int)), this, SLOT(updateLineNumberArea(const QRect&, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(m_fileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChange(const QString)));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    setFont(font);

    readSettings();

    toggleSourceFile();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeView::~CodeView()
{
    writeSettings();
    delete m_fileWatcher;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::openFile()
{
    QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open Source File"));

    if (path.isEmpty()) {
        return;
    }

    readSourceFile(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::reload()
{
    int currentLine = getCurrentLine();
    readSourceFile(m_sourceFile);
    setLine(currentLine);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::fileChange(const QString filename)
{
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

    setLine(currentLine);

    // BUG: We need to readd the file here as it seems the watcher thinks it has been deleted (even if just changed)
    //      so we only get one notification of a change so when doing a re-add here we get correct notifications again

    m_fileWatcher->addPath(filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CodeView::lineNumberAreaWidth()
{
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

    int space = 20 + 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::highlightCurrentLine()
{
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

void CodeView::lineNumberAreaPaintEvent(QPaintEvent* event)
{
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

            if (m_breakpoints->hasBreakpointFileLine(m_sourceFile, blockNumber + 1)) {
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::step()
{
    // g_debugSession->callAction(PDAction_step);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setMode(Mode mode)
{
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

void CodeView::updateDisassemblyCursor()
{
    if (m_mode != Disassembly) {
        return;
    }

    for (int i = 0, count = m_disassemblyAdresses.count(); i < count; ++i) {
        if (m_disassemblyAdresses[i].address != m_currentPc) {
            continue;
        }

        setLine(i + 1);

        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::toggleDisassembly()
{
    m_mode = Disassembly;
    // programCounterChanged(m_currentPc);
    setPlainText(m_disassemblyText);
    updateDisassemblyCursor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::toggleSourceFile()
{
    m_mode = Sourcefile;
    setCenterOnScroll(false);
    setPlainText(m_sourceCodeData);

    if (m_currentSourceLine >= 0) {
        setLine(m_currentSourceLine);
    }
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

void CodeView::initDefaultSourceFile(const QString& filename)
{
    m_sourceFile = filename;
    readSourceFile(m_sourceFile);
    toggleSourceFile();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::readSourceFile(const QString& filename)
{
    QFile f(filename);

    if (!f.exists())
        return;

    if (!m_sourceFile.isEmpty()) {
        m_fileWatcher->removePath(m_sourceFile);
    }

    m_sourceFile = filename;

    m_fileWatcher->addPath(QString(filename));

    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);

    m_sourceCodeData = ts.readAll();

    if (m_mode == Sourcefile) {
        setPlainText(m_sourceCodeData);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CodeView::getCurrentLine()
{
    QTextCursor cursor = textCursor();
    int line = cursor.block().blockNumber();

    return line;

    /*

    // + 1 due to 1 indexed
    bool added = m_breakpoints->toggleFileLineBreakpoint(m_sourceFile, line + 1);

    if (added) {
        m_interface->beginAddFileLineBreakpoint(m_sourceFile, line);
    } else {
        m_interface->beginRemoveFileLineBreakpoint(m_sourceFile, line);
    }

    m_lineNumberArea->repaint();
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setLine(int line)
{
    const QTextBlock& block = document()->findBlockByNumber(line - 1);
    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 0);
    setTextCursor(cursor);
    centerCursor();
    setFocus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setBreakpointModel(BreakpointModel* breakpoints)
{
    m_breakpoints = breakpoints;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void CodeView::setBackendInterface(IBackendRequests* iface)
{
    m_interface = iface;

    if (!iface) {
        return;
    }

    connect(m_interface, &IBackendRequests::endDisassembly, this, &CodeView::endDisassembly);
    connect(m_interface, &IBackendRequests::programCounterChanged, this, &CodeView::programCounterChanged);
    connect(m_interface, &IBackendRequests::sourceFileLineChanged, this, &CodeView::sourceFileLineChanged);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::sessionEnded()
{
    m_currentSourceLine = -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setExceptionLine(int line)
{
    m_currentSourceLine = line;
    setLine(line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setFileLine(const QString& file, int line)
{
    if (file != m_sourceFile) {
        readSourceFile(file);
    }

    m_currentSourceLine = line;

    if (m_mode == Sourcefile) {
        setLine(line);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::readSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeView"));
    m_sourceFile = settings.value(QStringLiteral("Sourcefile")).toString();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::writeSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeView"));
    settings.setValue(QStringLiteral("Sourcefile"), m_sourceFile);
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
