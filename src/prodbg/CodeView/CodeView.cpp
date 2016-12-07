#include "CodeView.h"
#include "Backend/IBackendRequests.h"
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

    m_sourceFile = QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeView::~CodeView()
{
    delete m_fileWatcher;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update called from the debugging session

void CodeView::sessionUpdate()
{
    // const char* filename;
    // int line;

    // if (g_debugSession->getFilenameLine(&filename, &line))
    //   setFileLine(filename, line);

    update();
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
        printf("addressWidth %d\n", m_addressWidth); 
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

    m_lineStart = blockNumber;
    const bool isDisassembly = m_mode == Disassembly;
    const int addressCount = m_disassemblyAdresses.count();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);

            if (isDisassembly) {
                if (blockNumber >= addressCount) {
                    return;
                }
                painter.drawText(0, top, width, height, Qt::AlignRight, m_disassemblyAdresses[blockNumber].addressText);
            } else {
                painter.drawText(0, top, width, height, Qt::AlignRight, number);
            }

            // if (g_debugSession->hasLineBreakpoint(m_sourceFile, blockNumber))
            //   painter.drawArc(0, top + 1, 16, height - 2, 0, 360 * 16);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }

    m_lineEnd = blockNumber;
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

void CodeView::setAddress(uint64_t address)
{
    /*
    m_address = address;

    // Check if disassembly is with in the range of what we have

    if (address >= m_disassemblyStart && m_address < m_disassemblyEnd) {
        // We have the address within the range so now we need to find the current line
        // \todo: We should really just keep a faster way to look this up using a struct
        // with address + text line for each line so we don't need to performe this operation
        // which is just a waste of time

        QString text = toPlainText();
        QStringList textList = text.split(QLatin1Char('\n'));

        for (int i = 0, lineCount = textList.size(); i < lineCount; ++i) {
            QByteArray ba = textList[i].toLocal8Bit();
            char* temp = ba.data() + 4;
            uint64_t ta = strtoul(ba.data(), &temp, 16);

            // printf("address 0x%x 0x%x\n", (uint32_t)address, (uint32_t)ta);

            if (ta == address) {
                setLine(i + m_lineStart + 1);
                return;
            }
        }
    } else {
        // Right now we request from only from the PC because we know that that location
        // is valid (from the disassembly point of view) If we had some more contex it would
        // be nice to be able to request code around a PC.

        printf("request address 0x%x\n", (uint32_t)m_address);

        if (m_address < m_disassemblyStart)
            m_disassemblyStart = m_address;

        // g_debugSession->requestDisassembly(m_disassemblyStart, (m_lineEnd - m_lineStart));

        // m_disassemblyEnd = 0x40;
    }
    */
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

    setPlainText(m_disassemblyText);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::programCounterChanged(uint64_t pc) {
    m_interface->beginDisassembly(pc, 32, &m_recvInstructions);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::toggleDisassembly()
{
    printf("toggle disassembly\n");

    if (!m_interface) {
        return;
    }

    printf("requesting disassembly\n");

    m_interface->beginDisassembly(0, 32, &m_recvInstructions);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::toggleSourceFile()
{
    setPlainText(m_sourceCodeData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::keyPressEvent(QKeyEvent* event)
{
    // TODO: Use proper actions from main menu instead

    if (event->key() == Qt::Key_Space) {
        if (m_mode == Sourcefile) {
            m_mode = Disassembly;
            toggleDisassembly();
        } else {
            m_mode = Sourcefile;
            toggleSourceFile();
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::readSourceFile(const QString& filename)
{
    QFile f(filename);

    if (!f.exists())
        return;

    if (!m_sourceFile.isEmpty())
        m_fileWatcher->removePath(m_sourceFile);

    m_sourceFile = filename;

    m_fileWatcher->addPath(QString(filename));

    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);

    m_sourceCodeData = ts.readAll();

    setPlainText(m_sourceCodeData);
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

// void CodeView::setBackendInterface(IBackendRequests* interface)
//{
//   m_interface = interface;
// connect(m_interface, &IBackendRequests::endDisassembly, this, &CodeView::endDisassembly);
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setBackendInterface(IBackendRequests* iface)
{
    m_interface = iface;
    connect(m_interface, &IBackendRequests::endDisassembly, this, &CodeView::endDisassembly);
    connect(m_interface, &IBackendRequests::programCounterChanged, this, &CodeView::programCounterChanged);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeView::setFileLine(const QString& file, int line)
{
    if (file != m_sourceFile)
        readSourceFile(file);

    setLine(line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
