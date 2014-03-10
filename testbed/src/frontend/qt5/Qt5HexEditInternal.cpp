#include <QtGui>
#include <QtWidgets>
#include "Qt5HexEditInternal.h"
#include "Qt5HexEditCommands.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int kHexCharsInLine = 47;
const int kGapHexAddress  = 10;
const int kGapHexAscii    = 16;
const int kBytesPerLine   = 16;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditInternal::Qt5HexEditInternal(QScrollArea* parent)
    : QWidget(parent)
    , m_scrollArea(parent)
    , m_undoStack(new QUndoStack(this))
    , m_size(0)
{
    setAddressWidth(4);
    setAddressOffset(0);
    setAddressArea(true);
    setAsciiArea(true);
    setHighlighting(true);
    setOverwriteMode(true);
    setReadOnly(false);
    setAddressAreaColor(QColor(0xd4, 0xd4, 0xd4, 0xff));
    setHighlightingColor(QColor(0xff, 0xff, 0x99, 0xff));
    setSelectionColor(QColor(0x6d, 0x9e, 0xff, 0xff));
    setFont(QFont("Courier", 10));
    resetSelection(0);
    setFocusPolicy(Qt::StrongFocus);

    connect(&m_cursorTimer, SIGNAL(timeout()), this, SLOT(updateCursor()));
    m_cursorTimer.setInterval(500);
    m_cursorTimer.start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditByteArray& Qt5HexEditInternal::getHexData()
{
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::insert(int index, const QByteArray& values)
{
    (void)index;

    if (values.length() > 0)
    {
        if (m_overwriteMode)
        {
            QUndoCommand* rangeUndoCommand = new Qt5HexEditRangeUndoCommand(&m_data, Qt5HexEditRangeUndoCommand::ReplaceOperation, index, values, values.length());
            m_undoStack->push(rangeUndoCommand);
            emit dataChanged();
        }
        else
        {
            QUndoCommand* rangeUndoCommand = new Qt5HexEditRangeUndoCommand(&m_data, Qt5HexEditRangeUndoCommand::InsertOperation, index, values, values.length());
            m_undoStack->push(rangeUndoCommand);
            emit dataChanged();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::insert(int index, char value)
{
    QUndoCommand* valueUndoCommand = new Qt5HexEditValueUndoCommand(&m_data, Qt5HexEditValueUndoCommand::InsertOperation, index, value);
    m_undoStack->push(valueUndoCommand);
    emit dataChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditInternal::indexOf(const QByteArray& values, int from)
{
    if (from > (m_data.getData().length() - 1))
    {
        from = m_data.getData().length() - 1;
    }

    int index = m_data.getData().indexOf(values, from);

    if (index > -1)
    {
        const int cursorPosition = index * 2;
        setCursorPosition(cursorPosition + values.length() * 2);
        resetSelection(cursorPosition);
        setSelection(cursorPosition + values.length() * 2);
        ensureVisible();
    }

    return index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditInternal::lastIndexOf(const QByteArray& values, int from)
{
    from -= values.length();
    if (from < 0)
    {
        from = 0;
    }

    int index = m_data.getData().lastIndexOf(values, from);

    if (index > -1)
    {
        int cursorPosition = index * 2;
        setCursorPosition(cursorPosition);
        resetSelection(cursorPosition);
        setSelection(cursorPosition + values.length() * 2);
        ensureVisible();
    }

    return index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::remove(int index, int length)
{
    (void)index;

    if (length > 0)
    {
        if (length == 1)
        {
            if (m_overwriteMode)
            {
                QUndoCommand* valueUndoCommand = new Qt5HexEditValueUndoCommand(&m_data, Qt5HexEditValueUndoCommand::ReplaceOperation, index, char(0));
                m_undoStack->push(valueUndoCommand);
                emit dataChanged();
            }
            else
            {
                QUndoCommand* valueUndoCommand = new Qt5HexEditValueUndoCommand(&m_data, Qt5HexEditValueUndoCommand::RemoveOperation, index, char(0));
                m_undoStack->push(valueUndoCommand);
                emit dataChanged();
            }
        }
        else
        {
            QByteArray values = QByteArray(length, char(0));
            if (m_overwriteMode)
            {
                QUndoCommand* rangeUndoCommand = new Qt5HexEditRangeUndoCommand(&m_data, Qt5HexEditRangeUndoCommand::ReplaceOperation, index, values, values.length());
                m_undoStack->push(rangeUndoCommand);
                emit dataChanged();
            }
            else
            {
                QUndoCommand* rangeUndoCommand = new Qt5HexEditRangeUndoCommand(&m_data, Qt5HexEditRangeUndoCommand::RemoveOperation, index, values, length);
                m_undoStack->push(rangeUndoCommand);
                emit dataChanged();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::replace(int index, char value)
{
    QUndoCommand* valueUndoCommand = new Qt5HexEditValueUndoCommand(&m_data, Qt5HexEditValueUndoCommand::ReplaceOperation, index, value);
    m_undoStack->push(valueUndoCommand);
    resetSelection();
    emit dataChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::replace(int index, const QByteArray& values)
{
    QUndoCommand* rangeUndoCommand = new Qt5HexEditRangeUndoCommand(&m_data, Qt5HexEditRangeUndoCommand::ReplaceOperation, index, values, values.length());
    m_undoStack->push(rangeUndoCommand);
    resetSelection();
    emit dataChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::replace(int position, int length, const QByteArray& values)
{
    QUndoCommand* rangeUndoCommand = new Qt5HexEditRangeUndoCommand(&m_data, Qt5HexEditRangeUndoCommand::ReplaceOperation, position, values, length);
    m_undoStack->push(rangeUndoCommand);
    resetSelection();
    emit dataChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::undo()
{
    m_undoStack->undo();
    emit dataChanged();
    setCursorPosition(m_cursorPosition);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::redo()
{
    m_undoStack->redo();
    emit dataChanged();
    setCursorPosition(m_cursorPosition);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QString Qt5HexEditInternal::getReadableString()
{
    return m_data.getReadableString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QString Qt5HexEditInternal::getReadableStringFromSelection()
{
    return m_data.getReadableString(getSelectionBegin(), getSelectionEnd());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setAddressArea(bool addressArea)
{
    m_addressArea = addressArea;
    adjust();
    setCursorPosition(m_cursorPosition);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setAddressWidth(int addressWidth)
{
    m_data.setAddressWidth(addressWidth);
    setCursorPosition(m_cursorPosition);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setAsciiArea(bool asciiArea)
{
    m_asciiArea = asciiArea;
    adjust();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setHighlighting(bool mode)
{
    m_highlighting = mode;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setAddressOffset(int offset)
{
    m_data.setAddressOffset(offset);
    adjust();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditInternal::getAddressOffset() const
{
    return m_data.getAddressOffset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setCursorPosition(int position)
{
    // Hide cursor
    m_blinking = false;
    update();

    // Test for cursor in range
    if (m_overwriteMode)
    {
        if (position > (m_data.getSize() * 2 - 1))
            position = (m_data.getSize() * 2 - 1);
    }
    else
    {
        if (position > (m_data.getSize() * 2))
            position = (m_data.getSize() * 2);
    }

    if (position < 0)
    {
        position = 0;
    }

    // Calculate cursor position
    m_cursorPosition = position;
    m_cursorY = (position / (2 * kBytesPerLine)) * m_characterHeight + 4;
    int positionX = (position % (2 * kBytesPerLine));
    m_cursorX = (((positionX >> 1) * 3) + (positionX % 2)) * m_characterWidth + m_hexPosition;

    // Show cursor
    m_blinking = true;
    update();
    emit currentAddressChanged(m_cursorPosition >> 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditInternal::getCursorPosition() const
{
    return m_cursorPosition;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setData(const QByteArray& data)
{
    m_data.setData(data);
    m_undoStack->clear();
    adjust();
    setCursorPosition(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray Qt5HexEditInternal::getData()
{
    return m_data.getData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setOverwriteMode(bool overwriteMode)
{
    m_overwriteMode = overwriteMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5HexEditInternal::getOverwriteMode() const
{
    return m_overwriteMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5HexEditInternal::getReadOnly() const
{
    return m_readOnly;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setAddressAreaColor(const QColor& color)
{
    m_addressAreaColor = color;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QColor Qt5HexEditInternal::getAddressAreaColor() const
{
    return m_addressAreaColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setHighlightingColor(const QColor& color)
{
    m_highlightingColor = color;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QColor Qt5HexEditInternal::getHighlightingColor() const
{
    return m_highlightingColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setSelectionColor(const QColor& color)
{
    m_selectionColor = color;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QColor Qt5HexEditInternal::getSelectionColor() const
{
    return m_selectionColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setFont(const QFont& font)
{
    QWidget::setFont(font);
    adjust();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditInternal::calculateCursorPos(QPoint position) const
{
    int result = -1;

    // Find character under cursor
    if ((position.x() >= m_hexPosition) && (position.x() < (m_hexPosition + kHexCharsInLine * m_characterWidth)))
    {
        int positionX = (position.x() - m_hexPosition) / m_characterWidth;

        if ((positionX % 3) == 0)
        {
            positionX = (positionX / 3) * 2;
        }
        else
        {
            positionX = ((positionX / 3) * 2) + 1;
        }

        int positionY = ((position.y() - 3) / m_characterHeight) * 2 * kBytesPerLine;

        result = positionX + positionY;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::resetSelection(int position)
{
    if (position < 0)
    {
        position = 0;
    }

    position /= 2;

    m_selectionAnchor = position;
    m_selectionBegin  = position;
    m_selectionEnd    = position;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::resetSelection()
{
    m_selectionBegin = m_selectionAnchor;
    m_selectionEnd   = m_selectionAnchor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::setSelection(int position)
{
    if (position < 0)
    {
        position = 0;
    }

    position /= 2;

    if (position >= m_selectionAnchor)
    {
        m_selectionEnd = position;
        m_selectionBegin = m_selectionAnchor;
    }
    else
    {
        m_selectionBegin = position;
        m_selectionEnd = m_selectionAnchor;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditInternal::getSelectionBegin() const
{
    return m_selectionBegin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditInternal::getSelectionEnd() const
{
    return m_selectionEnd;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::keyPressEvent(QKeyEvent* event)
{
    handleCursorMovements(event);
    handleSelectCommands(event);
    handleEditCommands(event);

    if (event->matches(QKeySequence::Copy))
    {
        QString result = QString();
        for (int index = getSelectionBegin(); index < getSelectionEnd(); ++index)
        {
            result += m_data.getData().mid(index, 1).toHex() + " ";
            if ((index % 16) == 15)
            {
                result.append('\n');
            }
        }

        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(result);
    }

    // Toggle between insert and overwrite mode
    if ((event->key() == Qt::Key_Insert) && (event->modifiers() == Qt::NoModifier))
    {
        m_overwriteMode = !m_overwriteMode;
        setCursorPosition(m_cursorPosition);
        overwriteModeChanged(m_overwriteMode);
    }

    ensureVisible();
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::mouseMoveEvent(QMouseEvent* event)
{
    m_blinking = false;
    update();
    int actualPosition = calculateCursorPos(event->pos());
    setCursorPosition(actualPosition);
    setSelection(actualPosition);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::mousePressEvent(QMouseEvent* event)
{
    m_blinking = false;
    update();
    int actualPosition = calculateCursorPos(event->pos());
    resetSelection(actualPosition);
    setCursorPosition(actualPosition);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    // Paint background patterns
    painter.fillRect(event->rect(), this->palette().color(QPalette::Base));

    if (m_addressArea)
    {
        painter.fillRect(QRect(m_addressPosition, event->rect().top(), m_hexPosition - kGapHexAddress + 2, height()), m_addressAreaColor);
    }

    if (m_asciiArea)
    {
        const int linePosition = m_asciiPosition - (kGapHexAscii >> 1);
        painter.setPen(Qt::gray);
        painter.drawLine(linePosition, event->rect().top(), linePosition, height());
    }

    painter.setPen(this->palette().color(QPalette::WindowText));

    // Calculate position
    int firstLineIndex = ((event->rect().top() / m_characterHeight) - m_characterHeight) * kBytesPerLine;
    if (firstLineIndex < 0)
    {
        firstLineIndex = 0;
    }

    int lastLineIndex = ((event->rect().bottom() / m_characterHeight) + m_characterHeight) * kBytesPerLine;
    if (lastLineIndex > m_data.getSize())
    {
        lastLineIndex = m_data.getSize();
    }

    int startPositionY = ((firstLineIndex) / kBytesPerLine) * m_characterHeight + m_characterHeight;

    // Paint address area
    if (m_addressArea)
    {
        for (int lineIndex = firstLineIndex, positionY = startPositionY; lineIndex < lastLineIndex; lineIndex += kBytesPerLine, positionY += m_characterHeight)
        {
            QString address = QString("%1").arg(lineIndex + m_data.getAddressOffset(), m_data.getRealAddressNumbers(), 16, QChar('0'));
            painter.drawText(m_addressPosition, positionY, address);
        }
    }

    // Paint hex area
    QByteArray hexValues(m_data.getData().mid(firstLineIndex, lastLineIndex - firstLineIndex + 1).toHex());
    QBrush highlightedBrush = QBrush(m_highlightingColor);
    QPen highlightedPen = QPen(this->palette().color(QPalette::WindowText));
    QBrush selectedBrush = QBrush(m_selectionColor);
    QPen selectedPen = QPen(Qt::white);
    QPen standardPen = QPen(this->palette().color(QPalette::WindowText));

    painter.setBackgroundMode(Qt::TransparentMode);

    for (int lineIndex = firstLineIndex, positionY = startPositionY; lineIndex < lastLineIndex; lineIndex += kBytesPerLine, positionY += m_characterHeight)
    {
        QByteArray hex;

        int positionX = m_hexPosition;
        for (int columnIndex = 0; ((lineIndex + columnIndex) < m_data.getSize() && (columnIndex < kBytesPerLine)); ++columnIndex)
        {
            int positionValues = lineIndex + columnIndex;
            if ((getSelectionBegin() <= positionValues) && (getSelectionEnd() > positionValues))
            {
                painter.setBackground(selectedBrush);
                painter.setBackgroundMode(Qt::OpaqueMode);
                painter.setPen(selectedPen);
            }
            else
            {
                if (m_highlighting)
                {
                    // hilight diff bytes
                    painter.setBackground(highlightedBrush);
                    if (m_data.getDataChanged(positionValues))
                    {
                        painter.setPen(highlightedPen);
                        painter.setBackgroundMode(Qt::OpaqueMode);
                    }
                    else
                    {
                        painter.setPen(standardPen);
                        painter.setBackgroundMode(Qt::TransparentMode);
                    }
                }
            }

            // Paint hex value
            if (columnIndex == 0)
            {
                hex = hexValues.mid((lineIndex - firstLineIndex) * 2, 2);
                painter.drawText(positionX, positionY, hex);
                positionX += 2 * m_characterWidth;
            }
            else
            {
                hex = hexValues.mid((lineIndex + columnIndex - firstLineIndex) * 2, 2).prepend(" ");
                painter.drawText(positionX, positionY, hex);
                positionX += 3 * m_characterWidth;
            }
        }
    }

    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setPen(this->palette().color(QPalette::WindowText));

    // Paint ascii area
    if (m_asciiArea)
    {
        for (int lineIndex = firstLineIndex, positionY = startPositionY; lineIndex < lastLineIndex; lineIndex += kBytesPerLine, positionY += m_characterHeight)
        {
            int asciiPositionX = m_asciiPosition;
            for (int columnIndex = 0; ((lineIndex + columnIndex) < m_data.getSize() && (columnIndex < kBytesPerLine)); ++columnIndex)
            {
                painter.drawText(asciiPositionX, positionY, m_data.getAsciiChar(lineIndex + columnIndex));
                asciiPositionX += m_characterWidth;
            }
        }
    }

    // Paint cursor
    if (m_blinking && !m_readOnly && hasFocus())
    {
        if (m_overwriteMode)
        {
            painter.fillRect(m_cursorX, m_cursorY + m_characterHeight - 2, m_characterWidth, 2, this->palette().color(QPalette::WindowText));
        }
        else
        {
            painter.fillRect(m_cursorX, m_cursorY, 2, m_characterHeight, this->palette().color(QPalette::WindowText));
        }
    }

    if (m_size != m_data.getSize())
    {
        m_size = m_data.getSize();
        emit currentSizeChanged(m_size);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::updateCursor()
{
    m_blinking = !m_blinking;
    update(m_cursorX, m_cursorY, m_characterWidth, m_characterHeight);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::adjust()
{
    m_characterWidth  = fontMetrics().width(QLatin1Char('9'));
    m_characterHeight = fontMetrics().height();
    m_addressPosition = 0;

    if (m_addressArea)
    {
        m_hexPosition = m_data.getRealAddressNumbers() * m_characterWidth + kGapHexAddress;
    }
    else
    {
        m_hexPosition = 0;
    }

    m_asciiPosition = m_hexPosition + kHexCharsInLine * m_characterWidth + kGapHexAscii;

    // Notify QAbstractScollbar of our size
    setMinimumHeight(((m_data.getSize() / 16 + 1) * m_characterHeight) + 5);
    if (m_asciiArea)
    {
        setMinimumWidth(m_asciiPosition + (kBytesPerLine * m_characterWidth));
    }
    else
    {
        setMinimumWidth(m_hexPosition + kHexCharsInLine * m_characterWidth);
    }

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::ensureVisible()
{
    // Scrolls to m_cursorX\m_cursorY
    // X-margin is 3 pixels, Y-margin is half of m_characterHeight
    m_scrollArea->ensureVisible(m_cursorX, m_cursorY + m_characterHeight / 2, 3, m_characterHeight / 2 + 2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::handleCursorMovements(QKeyEvent* event)
{
    if (event->matches(QKeySequence::MoveToNextChar))
    {
        setCursorPosition(m_cursorPosition + 1);
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToPreviousChar))
    {
        setCursorPosition(m_cursorPosition - 1);
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToEndOfLine))
    {
        setCursorPosition(m_cursorPosition | (2 * kBytesPerLine - 1));
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToStartOfLine))
    {
        setCursorPosition(m_cursorPosition - (m_cursorPosition % (2 * kBytesPerLine)));
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToPreviousLine))
    {
        setCursorPosition(m_cursorPosition - (2 * kBytesPerLine));
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToNextLine))
    {
        setCursorPosition(m_cursorPosition + (2 * kBytesPerLine));
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToNextPage))
    {
        setCursorPosition(m_cursorPosition + (((m_scrollArea->viewport()->height() / m_characterHeight) - 1) * 2 * kBytesPerLine));
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToPreviousPage))
    {
        setCursorPosition(m_cursorPosition - (((m_scrollArea->viewport()->height() / m_characterHeight) - 1) * 2 * kBytesPerLine));
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToEndOfDocument))
    {
        setCursorPosition(m_data.getSize() * 2);
        resetSelection(m_cursorPosition);
    }

    if (event->matches(QKeySequence::MoveToStartOfDocument))
    {
        setCursorPosition(0);
        resetSelection(m_cursorPosition);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::handleSelectCommands(QKeyEvent* event)
{
    if (event->matches(QKeySequence::SelectAll))
    {
        resetSelection(0);
        setSelection(2 * m_data.getSize() + 1);
    }

    if (event->matches(QKeySequence::SelectNextChar))
    {
        const int position = m_cursorPosition + 1;
        setCursorPosition(position);
        setSelection(position);
    }

    if (event->matches(QKeySequence::SelectPreviousChar))
    {
        const int position = m_cursorPosition - 1;
        setSelection(position);
        setCursorPosition(position);
    }

    if (event->matches(QKeySequence::SelectEndOfLine))
    {
        const int position = m_cursorPosition - (m_cursorPosition % (2 * kBytesPerLine)) + (2 * kBytesPerLine);
        setCursorPosition(position);
        setSelection(position);
    }

    if (event->matches(QKeySequence::SelectStartOfLine))
    {
        const int position = m_cursorPosition - (m_cursorPosition % (2 * kBytesPerLine));
        setCursorPosition(position);
        setSelection(position);
    }

    if (event->matches(QKeySequence::SelectPreviousLine))
    {
        const int position = m_cursorPosition - (2 * kBytesPerLine);
        setCursorPosition(position);
        setSelection(position);
    }

    if (event->matches(QKeySequence::SelectNextLine))
    {
        const int position = m_cursorPosition + (2 * kBytesPerLine);
        setCursorPosition(position);
        setSelection(position);
    }

    if (event->matches(QKeySequence::SelectNextPage))
    {
        const int position = m_cursorPosition + (((m_scrollArea->viewport()->height() / m_characterHeight) - 1) * 2 * kBytesPerLine);
        setCursorPosition(position);
        setSelection(position);
    }

    if (event->matches(QKeySequence::SelectPreviousPage))
    {
        const int position = m_cursorPosition - (((m_scrollArea->viewport()->height() / m_characterHeight) - 1) * 2 * kBytesPerLine);
        setCursorPosition(position);
        setSelection(position);
    }

    if (event->matches(QKeySequence::SelectEndOfDocument))
    {
        const int position = m_data.getSize() * 2;
        setCursorPosition(position);
        setSelection(position);
    }

    if (event->matches(QKeySequence::SelectStartOfDocument))
    {
        const int position = 0;
        setCursorPosition(position);
        setSelection(position);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditInternal::handleEditCommands(QKeyEvent* event)
{
    if (m_readOnly)
        return;

    const int characterX = (m_cursorX - m_hexPosition) / m_characterWidth;
    const int positionX  = (characterX / 3) * 2 + (characterX % 3);

    int positionValues = (m_cursorY / m_characterHeight) * kBytesPerLine + positionX / 2;

    // Hex input
    const int key = int(event->text()[0].toLower().toLatin1());

    printf("Got edit event: %d %c\n", key, key);

    //if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f'))
    if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f'))
    {

        if (getSelectionBegin() != getSelectionEnd())
        {
            positionValues = getSelectionBegin();
            remove(positionValues, getSelectionEnd() - positionValues);
            setCursorPosition(2 * positionValues);
            resetSelection(2 * positionValues);
        }

        if (!m_overwriteMode && (characterX % 3) == 0)
        {
            // Insert a byte
            insert(positionValues, char(0));
        }

        // Change content
        if (m_data.getSize() > 0)
        {
            QByteArray hexValue = m_data.getData().mid(positionValues, 1).toHex();

            if ((characterX % 3) == 0)
            {
                hexValue[0] = key;
            }
            else
            {
                hexValue[1] = key;
            }

            replace(positionValues, QByteArray().fromHex(hexValue)[0]);

            setCursorPosition(m_cursorPosition + 1);
            resetSelection(m_cursorPosition);
        }
    }

    // Cut and paste
    if (event->matches(QKeySequence::Cut))
    {
        QString result = QString();
        for (int index = getSelectionBegin(); index < getSelectionEnd(); ++index)
        {
            result += m_data.getData().mid(index, 1).toHex() + " ";
            if ((index % 16) == 15)
            {
                result.append("\n");
            }
        }

        remove(getSelectionBegin(), getSelectionEnd() - getSelectionBegin());
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(result);
        setCursorPosition(getSelectionBegin());
        resetSelection(getSelectionBegin());
    }

    if (event->matches(QKeySequence::Paste))
    {
        QClipboard* clipboard = QApplication::clipboard();
        QByteArray values = QByteArray().fromHex(clipboard->text().toLatin1());
        insert(m_cursorPosition / 2, values);
        setCursorPosition(m_cursorPosition + 2 * values.length());
        resetSelection(getSelectionBegin());
    }

    // Delete
    if (event->matches(QKeySequence::Delete))
    {
        if (getSelectionBegin() != getSelectionEnd())
        {
            positionValues = getSelectionBegin();
            remove(positionValues, getSelectionEnd() - positionValues);
            setCursorPosition(2 * positionValues);
            resetSelection(2 * positionValues);
        }
        else
        {
            if (m_overwriteMode)
            {
                replace(positionValues, char(0));
            }
            else
            {
                remove(positionValues, 1);
            }
        }
    }

    // Backspace
    if ((event->key() == Qt::Key_Backspace) && (event->modifiers() == Qt::NoModifier))
    {
        if (getSelectionBegin() != getSelectionEnd())
        {
            positionValues = getSelectionBegin();
            remove(positionValues, getSelectionEnd() - positionValues);
            setCursorPosition(2 * positionValues);
            resetSelection(2 * positionValues);
        }
        else
        {
            if (positionValues > 0)
            {
                if (m_overwriteMode)
                {
                    replace(positionValues - 1, char(0));
                }
                else
                {
                    remove(positionValues - 1, 1);
                }

                setCursorPosition(m_cursorPosition - 2);
            }
        }
    }

    // Undo
    if (event->matches(QKeySequence::Undo))
    {
        undo();
    }

    // Redo
    if (event->matches(QKeySequence::Redo))
    {
        redo();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}
