#include "MemoryViewWidget.h"
#include "Backend/IBackendRequests.h"

#include <QtCore/QPointer>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QApplication>

#include <ctype.h>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const QChar s_HexTable[16] = {
    QLatin1Char('0'), QLatin1Char('1'), QLatin1Char('2'), QLatin1Char('3'), QLatin1Char('4'), QLatin1Char('5'),
    QLatin1Char('6'), QLatin1Char('7'), QLatin1Char('8'), QLatin1Char('9'), QLatin1Char('a'), QLatin1Char('b'),
    QLatin1Char('c'), QLatin1Char('d'), QLatin1Char('e'), QLatin1Char('f'),
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct MemViewTypeMeta
{
    int m_BytesPerElement;
    int m_DisplayWidthChars;
    void (*m_Formatter)(QString* target, int displayWidth, int byteCount, const uint16_t* values,
                        MemoryViewWidget::Endianess);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint64_t decodeValue(const uint16_t* values, int count, MemoryViewWidget::Endianess endianess)
{
    uint64_t value = 0;
    switch (endianess) {
        case MemoryViewWidget::Big:
            for (int i = 0; i < count; ++i) {
                value <<= 8;
                value |= (values[i] & 0xff);
            }
            break;
        case MemoryViewWidget::Little:
            for (int i = count - 1; i >= 0; --i) {
                value <<= 8;
                value |= (values[i] & 0xff);
            }
            break;
    }

    return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void formatHex(QString* target, int displayWidth, int byteCount, const uint16_t* values,
                      MemoryViewWidget::Endianess endianess)
{
    const uint64_t value = decodeValue(values, byteCount, endianess);

    int shift = (byteCount * 8) - 4;
    while (shift >= 0) {
        int val = (value >> shift) & 0xf;
        target->push_back(s_HexTable[val]);
        shift -= 4;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void assignStr(const char* str, int len, int displayWidth, QString* target)
{
    for (int i = len; i < displayWidth; ++i) {
        target->push_back(QLatin1Char(' ')); // Right align
    }
    for (const char* p = str; *p; ++p) {
        target->push_back(QChar(int(*p)));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void formatUnsigned(QString* target, int displayWidth, int byteCount, const uint16_t* values,
                           MemoryViewWidget::Endianess endianess)
{
    const uint64_t value = decodeValue(values, byteCount, endianess);
    char buffer[32];
    buffer[0] = 0;
    int len = 0;
    switch (byteCount) {
        case 1:
            len = snprintf(buffer, sizeof buffer, "%u", (uint8_t)value);
            break;
        case 2:
            len = snprintf(buffer, sizeof buffer, "%u", (uint16_t)value);
            break;
        case 4:
            len = snprintf(buffer, sizeof buffer, "%u", (uint32_t)value);
            break;
        case 8:
            len = snprintf(buffer, sizeof buffer, "%llu", (uint64_t)value);
            break;
    }

    assignStr(buffer, len, displayWidth, target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void formatSigned(QString* target, int displayWidth, int byteCount, const uint16_t* values,
                         MemoryViewWidget::Endianess endianess)
{
    const uint64_t value = decodeValue(values, byteCount, endianess);
    char buffer[32];
    buffer[0] = 0;
    int len = 0;
    switch (byteCount) {
        case 1:
            len = snprintf(buffer, sizeof buffer, "%d", (int8_t)value);
            break;
        case 2:
            len = snprintf(buffer, sizeof buffer, "%d", (int16_t)value);
            break;
        case 4:
            len = snprintf(buffer, sizeof buffer, "%d", (int32_t)value);
            break;
        case 8:
            len = snprintf(buffer, sizeof buffer, "%lld", (int64_t)value);
            break;
    }

    assignStr(buffer, len, displayWidth, target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void formatFloat(QString* target, int displayWidth, int byteCount, const uint16_t* values,
                        MemoryViewWidget::Endianess endianess)
{
    const uint64_t value = decodeValue(values, byteCount, endianess);

    union
    {
        uint64_t i;
        double f;
    } itof64;

    union
    {
        uint32_t i;
        float f;
    } itof32;

    char buffer[32];
    buffer[0] = 0;
    int len = 0;
    switch (byteCount) {
        case 4:
            itof32.i = (uint32_t)value;
            len = snprintf(buffer, sizeof buffer, "%15.5g", itof32.f);
            break;
        case 8:
            itof64.i = value;
            // XXX: MSVC
            len = snprintf(buffer, sizeof buffer, "%15.5g", itof64.f);
            break;

        default:
            return;
    }

    assignStr(buffer, len, displayWidth, target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const struct MemViewTypeMeta s_TypeMeta[] = {
    { 1, 2, formatHex },       // kHexByte: FF
    { 1, 3, formatUnsigned },  // kUnsignedByte: 255
    { 1, 4, formatSigned },    // kSignedByte: -128
    { 2, 4, formatHex },       // kHexWord: FFFF
    { 2, 5, formatUnsigned },  // kUnsignedWord: 65535
    { 2, 6, formatSigned },    // kSignedWord: -32768
    { 4, 8, formatHex },       // kHexDword: FFFFFFFF
    { 4, 10, formatUnsigned }, // kUnsignedDword: 4294967295
    { 4, 11, formatSigned },   // kSignedDword: -2147483648
    { 8, 16, formatHex },      // kHexQword: FFFFFFFFFFFFFFFF
    { 8, 20, formatUnsigned }, // kUnsignedQword: 18446744073709551615
    { 8, 20, formatSigned },   // kSignedQword: -9223372036854775808
    { 4, 16, formatFloat },    // kFloat: arbitrary formatting width
    { 8, 16, formatFloat },    // kDouble: arbitrary formatting width
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QChar s_AsciiTab[256];

class MemoryViewPrivate
{
public:
    QPointer<IBackendRequests> m_Interface;

    int m_ElementsPerRow = 8;
    MemoryViewWidget::DataType m_DataType = MemoryViewWidget::X8;
    MemoryViewWidget::Endianess m_Endianess = MemoryViewWidget::Little;

    int m_WheelSpeedRows = 4;
    int m_PageSizeInRows = 16;
    uint64_t m_TopRow = 0;
    int m_adddressWidth = 8;

    uint64_t m_cachedRangeStart = 0;
    uint64_t m_cachedRangeEnd = 0;
    bool m_transferInProgress = false;
    bool m_expressionStatus = true;

    QVector<uint16_t> m_Cache;
    QVector<uint16_t> m_transferCache;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void access(uint64_t address, uint64_t count, QVector<uint16_t>* values)
    {
        uint64_t start = address;
        uint64_t end = address + count;

        values->resize(0);

        // Very basic caching, we only support the exactly previous request as cache.
        if ((m_cachedRangeStart == start && end == m_cachedRangeEnd) && !m_transferInProgress) {
            *values = m_transferCache;
            return;
        }

        // fill with dummy data for now until done
        for (uint64_t ai = address, ae = address + count; ai != ae; ++ai) {
            values->push_back(0 & 0xff); // hack test
        }

        if (m_Interface) {
            m_Interface->beginReadMemory(start, end, &m_transferCache);
            m_transferInProgress = true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void jump(int rowCount) { m_TopRow += rowCount * m_ElementsPerRow * bytesPerElement(); }

    int bytesPerElement() const { return s_TypeMeta[m_DataType].m_BytesPerElement; }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void paintEvent(QWidget* widget, QPaintEvent* ev)
    {
        QFont font = widget->font();
        QFontMetrics fontMetrics(font);
        // QRect widgetRect = widget->rect();
        QRect dirtyRect = ev->rect();
        const int elementsPerRow = m_ElementsPerRow;
        const int bytesPerRow = elementsPerRow * bytesPerElement();

        QColor baseColor = QApplication::palette().base().color();

        QPainter painter(widget);
        painter.setFont(font);

        painter.fillRect(dirtyRect, baseColor);

        if (!m_expressionStatus) {
            painter.drawText(dirtyRect, 0, QStringLiteral("Unable to evaluate expression."));
            return;
        }

        const int charWidth = fontMetrics.boundingRect(QLatin1Char('W')).width();
        const int rowHeight = fontMetrics.height();
        const int rows = (widget->height() + rowHeight - 1) / rowHeight;

        uint64_t firstByte = m_TopRow;
        uint64_t lastByte = m_TopRow + bytesPerRow * rows;

        m_Cache.clear();
        access(firstByte, lastByte - firstByte, &m_Cache);

        int screenY = 0;
        int dataOffset = 0;

        const MemViewTypeMeta& typeMeta = s_TypeMeta[m_DataType];

        QString rowText;
        rowText.reserve((1 + typeMeta.m_DisplayWidthChars));

        const int addressWidth = (m_adddressWidth * 2) * charWidth;
        const int dataWidth = charWidth * (elementsPerRow * typeMeta.m_DisplayWidthChars + elementsPerRow - 1);
        const int asciiWidth = bytesPerRow * charWidth;
        const int gutterWidth = charWidth;

        for (int row = 0; row < rows; ++row) {
            QRect addressRect(0, screenY, addressWidth, rowHeight);
            QRect dataRect(addressRect.x() + addressRect.width() + gutterWidth, screenY, dataWidth, rowHeight);
            QRect asciiRect(dataRect.x() + dataRect.width() + gutterWidth, screenY, asciiWidth, rowHeight);

            if (dirtyRect.intersects(addressRect)) {
                rowText.resize(0);
                uint64_t addr = (m_TopRow + dataOffset);

                uint32_t byteShift = 56;

                if (m_adddressWidth == 4) {
                    byteShift = 24;
                } else {
                    byteShift = 8;
                }

                for (int i = 0; i < m_adddressWidth; ++i) {
                    uint8_t byte = addr >> byteShift;
                    rowText.append(s_HexTable[byte >> 4]);
                    rowText.append(s_HexTable[byte & 0xf]);
                    addr <<= 8;
                }

                painter.drawText(addressRect, 0, rowText);
            }

            if (dirtyRect.intersects(dataRect)) {
                rowText.resize(0);

                for (int i = 0; i < elementsPerRow; ++i) {
                    if (i > 0) {
                        rowText.push_back(QLatin1Char(' '));
                    }

                    const uint16_t* values = m_Cache.constData() + dataOffset + i * typeMeta.m_BytesPerElement;
                    (*typeMeta.m_Formatter)(&rowText, typeMeta.m_DisplayWidthChars, typeMeta.m_BytesPerElement, values,
                                            m_Endianess);
                }

                painter.drawText(dataRect, 0, rowText);
            }

            if (dirtyRect.intersects(asciiRect)) {
                rowText.resize(0);
                for (int i = 0; i < bytesPerRow; ++i) {
                    uint16_t value = m_Cache.at(i + dataOffset);
                    uint8_t byte = value & 0xff;
                    rowText.append(s_AsciiTab[byte]);
                }

                painter.drawText(asciiRect, 0, rowText);
            }

            screenY += rowHeight;
            dataOffset += bytesPerRow;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewWidget::MemoryViewWidget(QWidget* parent)
    : Base(parent)
    , m_Private(new MemoryViewPrivate)
{
    // Can be any fixed with font.
#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    font.setFixedPitch(true);
    setFont(font);

    setFocusPolicy(Qt::StrongFocus);

    if (!s_AsciiTab[int('a')].unicode()) {
        for (int i = 0; i < 256; ++i) {
            if (isprint(i)) {
                s_AsciiTab[i] = QLatin1Char(i);
            } else {
                s_AsciiTab[i] = QLatin1Char('.');
            }
        }
    }

    {
        QAction* nextPageAction = new QAction(QStringLiteral("Next Page"), this);
        nextPageAction->setShortcut(QKeySequence::MoveToNextPage);
        nextPageAction->setShortcutContext(Qt::WidgetShortcut);
        this->addAction(nextPageAction);
        connect(nextPageAction, &QAction::triggered, this, &MemoryViewWidget::displayNextPage);
    }

    {
        QAction* prevPageAction = new QAction(QStringLiteral("Previous Page"), this);
        prevPageAction->setShortcut(QKeySequence::MoveToPreviousPage);
        prevPageAction->setShortcutContext(Qt::WidgetShortcut);
        this->addAction(prevPageAction);
        connect(prevPageAction, &QAction::triggered, this, &MemoryViewWidget::displayPrevPage);
    }

    {
        QAction* nextLineAction = new QAction(QStringLiteral("Next Line"), this);
        nextLineAction->setShortcut(QKeySequence::MoveToNextLine);
        nextLineAction->setShortcutContext(Qt::WidgetShortcut);
        this->addAction(nextLineAction);
        connect(nextLineAction, &QAction::triggered, this, &MemoryViewWidget::displayNextLine);
    }

    {
        QAction* prevLineAction = new QAction(QStringLiteral("Previous Line"), this);
        prevLineAction->setShortcut(QKeySequence::MoveToPreviousLine);
        prevLineAction->setShortcutContext(Qt::WidgetShortcut);
        this->addAction(prevLineAction);
        connect(prevLineAction, &QAction::triggered, this, &MemoryViewWidget::displayPrevLine);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewWidget::~MemoryViewWidget()
{
    delete m_Private;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::setBackendInterface(IBackendRequests* interface)
{
    m_Private->m_Interface = interface;

    if (interface) {
        connect(interface, &IBackendRequests::endReadMemory, this, &MemoryViewWidget::endReadMemory);
        connect(interface, &IBackendRequests::programCounterChanged, this, &MemoryViewWidget::programCounterChanged);
    }

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::programCounterChanged(const IBackendRequests::ProgramCounterChange&)
{
    // If pc has changed we re-request the current data again
    if (m_Private->m_Interface) {
        m_Private->m_Interface->beginReadMemory(m_Private->m_cachedRangeStart, m_Private->m_cachedRangeEnd,
                                                &m_Private->m_transferCache);
        m_Private->m_transferInProgress = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gets called when transfor from backend to frontend has finished

void MemoryViewWidget::endReadMemory(QVector<uint16_t>* target, uint64_t address, int addressWidth)
{
    // so this is a hack. We need a better way to do this. This is because if there are several memory requests
    // in flight we must make sure that its "ours" that gets called here.
    if (target != &m_Private->m_transferCache) {
        return;
    }

    m_Private->m_adddressWidth = addressWidth;

    m_Private->m_cachedRangeStart = address;
    m_Private->m_cachedRangeEnd = address + target->count();

    m_Private->m_transferInProgress = false;

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::setExpressionStatus(bool status)
{
    m_Private->m_expressionStatus = status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::paintEvent(QPaintEvent* ev)
{
    m_Private->paintEvent(this, ev);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::displayNextPage()
{
    m_Private->jump(m_Private->m_PageSizeInRows);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::displayPrevPage()
{
    m_Private->jump(-m_Private->m_PageSizeInRows);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::displayNextLine()
{
    m_Private->jump(1);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::displayPrevLine()
{
    m_Private->jump(-1);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::contextMenuEvent(QContextMenuEvent* ev)
{
    QMenu contextMenu;
    contextMenu.addActions(actions());
    contextMenu.exec(mapToGlobal(ev->pos()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::wheelEvent(QWheelEvent* ev)
{
    if (ev->angleDelta().y() < 0) {
        m_Private->jump(m_Private->m_WheelSpeedRows);
    } else if (ev->angleDelta().y() > 0) {
        m_Private->jump(-m_Private->m_WheelSpeedRows);
    }

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewWidget::DataType MemoryViewWidget::dataType() const
{
    return m_Private->m_DataType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::setDataType(DataType type)
{
    m_Private->m_DataType = type;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewWidget::Endianess MemoryViewWidget::endianess() const
{
    return m_Private->m_Endianess;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::setEndianess(Endianess e)
{
    m_Private->m_Endianess = e;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MemoryViewWidget::elementsPerLine() const
{
    return m_Private->m_ElementsPerRow;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::setElementsPerLine(int count)
{
    m_Private->m_ElementsPerRow = std::min(std::max(1, count), 64);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::setAddress(uint64_t address)
{
    m_Private->m_TopRow = address;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
