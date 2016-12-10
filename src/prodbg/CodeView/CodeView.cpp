#include "CodeView.h"
#include "Backend/IBackendRequests.h"
#include "BreakpointModel.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QtGui>

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

void CodeView::fileChange(const QString filename)
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, QStringLiteral("File has been changed"),
                                  QStringLiteral("File %1 was changed, Reload?").arg(filename),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    QFile f(filename);

    if (!f.exists())
        return;

    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    setPlainText(ts.readAll());

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

    QColor lineColor = QColor(Qt::lightGray).lighter(100);

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
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();
    int width = m_lineNumberArea->width();
    int height = fontMetrics().height();

    const bool isDisassembly = m_mode == Disassembly;
    const int addressCount = m_disassemblyAdresses.count();

    int fontHeight = fontMetrics().height() - 2;

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);

            if (isDisassembly) {
                if (blockNumber >= addressCount) {
                    return;
                }

                uint64_t address = m_disassemblyAdresses[blockNumber].address;
                const QString& addressText = m_disassemblyAdresses[blockNumber].addressText;

                painter.drawText(0, top, width, height, Qt::AlignRight, addressText);

                if (m_breakpoints->hasBreakpointAddress(address)) {
                    // TODO: Make sure to take font size into account
                    painter.setBrush(Qt::red);
                    painter.drawEllipse(4, top, fontHeight, fontHeight);
                }
            } else {
                painter.drawText(0, top, width, height, Qt::AlignRight, number);

                if (m_breakpoints->hasBreakpointFileLine(m_sourceFile, blockNumber + 1)) {
                    // TODO: Make sure to take font size into account
                    painter.setBrush(Qt::red);
                    painter.drawEllipse(4, top, fontHeight, fontHeight);
                }
            }

            // if (g_debugSession->hasLineBreakpoint(m_sourceFile, blockNumber))
            //   painter.drawArc(0, top + 1, 16, height - 2, 0, 360 * 16);
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

void CodeView::endDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, int addressWidth)
{
    if (instructions->count() == 0) {
        return;
    }

    QString addressText;

    m_disassemblyText.resize(0);
    m_disassemblyAdresses.resize(0);

    m_disassemblyStart = instructions->at(0).address;
    m_addressWidth = addressWidth;

    for (auto& inst : *instructions) {
        switch (addressWidth) {
            case 2:
                addressText.sprintf("%04X", (uint16_t)inst.address);
                break;
            case 4:
                addressText.sprintf("%08X", (uint32_t)inst.address);
                break;
            default:
                addressText.sprintf("%16llX", inst.address);
                break;
        }

        m_disassemblyText.append(inst.text);
        m_disassemblyText.append(QLatin1Char('\n'));
        m_disassemblyAdresses.append({ inst.address, addressText });
    }

    m_disassemblyEnd = instructions->at(instructions->count() - 1).address;
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

void CodeView::sourceFileLineChanged(const QString& filename, int line)
{
    setFileLine(filename, line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::programCounterChanged(uint64_t pc)
{
    m_currentPc = pc;

    // check if pc is with the disassembly range and search for the current line to set

    if ((pc >= m_disassemblyStart && pc <= m_disassemblyEnd) && m_disassemblyEnd != 0) {
        updateDisassemblyCursor();
    } else {
        // QSize size = frameSize();
        int fontHeight = fontMetrics().height();
        int linesInView = (height() / fontHeight) - 1;
        if (linesInView <= 0) {
            linesInView = 1;
        }

        if (pc >= linesInView) {
            pc -= linesInView;
        }

        // mask out the lower bits of start offset so we have a 4 byte even address to disassemble from
        pc &= (uint64_t)(~3);

        if (m_interface) {
            m_interface->beginDisassembly(pc, linesInView * 2, &m_recvInstructions);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::toggleDisassembly()
{
    m_mode = Disassembly;
    programCounterChanged(m_currentPc);
    setPlainText(m_disassemblyText);
    updateDisassemblyCursor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::toggleSourceFile()
{
    m_mode = Sourcefile;
    setCenterOnScroll(false);
    setPlainText(m_sourceCodeData);
    setLine(m_currentSourceLine);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void CodeView::toggleBreakpoint()
{
    /*
    if (m_mode == Disassembly) {
        QTextCursor cursor = textCursor();
        int index = cursor.block().blockNumber();
        bool added = m_breakpoints->toggleAddressBreakpoint(m_disassemblyAdresses[index].address);

        if (added) {
            interface->beginAddAddressBreakpoint(m_disassemblyAdresses[index].address);
        } else {
            interface->beginRemoveAddressBreakpoint(m_disassemblyAdresses[index].address);
        }

    } else {
        QTextCursor cursor = textCursor();
        int line = cursor.block().blockNumber();

        // + 1 due to 1 indexed
        bool added = m_breakpoints->toggleFileLineBreakpoint(m_sourceFile, line + 1);

        if (added) {
            interface->beginAddFileLineBreakpoint(m_sourceFile, line);
        } else {
            interface->beginRemoveFileLineBreakpoint(m_sourceFile, line);
        }
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
