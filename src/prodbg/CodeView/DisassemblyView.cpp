#include "BreakpointModel.h"
#include "DisassemblyView.h"
#include <QTextBlock>
#include <QPainter>
#include <QApplication>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: This is currently duplicated in CodeView/Disassembly view. Fix!

class AddressArea : public QWidget
{
public:
    AddressArea(DisassemblyView* view)
        : QWidget(view)
        , m_disassemblyView(view)
    {
    }

    QSize sizeHint() const { return QSize(m_disassemblyView->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* event) { m_disassemblyView->lineNumberAreaPaintEvent(event); }

private:
    DisassemblyView* m_disassemblyView;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DisassemblyView::DisassemblyView(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_addressArea(nullptr)
{
    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_addressArea = new AddressArea(this);

    updateAddressAreaWidth(0);
    highlightCurrentLine();

#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    connect(this, &QPlainTextEdit::blockCountChanged, this, &DisassemblyView::updateAddressAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &DisassemblyView::updateAddressArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &DisassemblyView::highlightCurrentLine);

    setFont(font);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DisassemblyView::~DisassemblyView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::updateAddressAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::updateAddressArea(const QRect& rect, int dy)
{
    if (dy) {
        m_addressArea->scroll(0, dy);
    } else {
        m_addressArea->update(0, rect.y(), m_addressArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateAddressAreaWidth(0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::highlightCurrentLine()
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
// TODO: Make this a bit more configurable and less hard-coded

int DisassemblyView::lineNumberAreaWidth()
{
    // 20 + to give rom for breakpoint marker

    int space = 20 + 3 + fontMetrics().width(QLatin1Char('9')) * m_addressWidth * 2;

    return space;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_addressArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPalette pal = QApplication::palette();

#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    QPainter painter(m_addressArea);
    painter.fillRect(event->rect(), pal.alternateBase());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();
    int width = m_addressArea->width();
    int height = fontMetrics().height();

    const int addressCount = m_disassemblyAdresses.count();

    int fontHeight = fontMetrics().height() - 2;

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(pal.text().color());

            if (blockNumber >= addressCount) {
                return;
            }

            uint64_t address = m_disassemblyAdresses[blockNumber].address;
            const QString& addressText = m_disassemblyAdresses[blockNumber].addressText;

            painter.drawText(0, top, width, height, Qt::AlignRight, addressText);

            if (m_breakpoints->hasBreakpointAddress(address)) {
                painter.setBrush(Qt::red);
                painter.drawEllipse(4, top, fontHeight, fontHeight);
            }

            // Draw ugly arrow!

            if (m_disassemblyAdresses[blockNumber].address == m_currentPc) {
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

void DisassemblyView::endDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, int addressWidth)
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
    updateDisassemblyCursor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::updatePc(uint64_t pc)
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

void DisassemblyView::updateDisassemblyCursor()
{
    for (int i = 0, count = m_disassemblyAdresses.count(); i < count; ++i) {
        if (m_disassemblyAdresses[i].address != m_currentPc) {
            continue;
        }

        setLine(i + 1);

        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::toggleBreakpoint()
{
    QTextCursor cursor = textCursor();
    int index = cursor.block().blockNumber();

    bool added = m_breakpoints->toggleAddressBreakpoint(m_disassemblyAdresses[index].address);

    if (added) {
        m_interface->beginAddAddressBreakpoint(m_disassemblyAdresses[index].address);
    } else {
        m_interface->beginRemoveAddressBreakpoint(m_disassemblyAdresses[index].address);
    }

    m_addressArea->repaint();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::setLine(int line)
{
    const QTextBlock& block = document()->findBlockByNumber(line - 1);
    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 0);
    setTextCursor(cursor);
    centerCursor();
    setFocus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisassemblyView::setBackendInterface(IBackendRequests* interface)
{
    m_interface = interface;
    connect(m_interface, &IBackendRequests::endDisassembly, this, &DisassemblyView::endDisassembly);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
