#include "backend/backend_requests_interface.h"
#include "memory_view_widget.h"

#include <QtCore/QPointer>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtGui/QPalette>

#include <ctype.h>
#include <inttypes.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const QChar s_hex_table[16] = {
    QLatin1Char('0'), QLatin1Char('1'), QLatin1Char('2'), QLatin1Char('3'), QLatin1Char('4'), QLatin1Char('5'),
    QLatin1Char('6'), QLatin1Char('7'), QLatin1Char('8'), QLatin1Char('9'), QLatin1Char('a'), QLatin1Char('b'),
    QLatin1Char('c'), QLatin1Char('d'), QLatin1Char('e'), QLatin1Char('f'),
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct MemViewTypeMeta {
    int m_bytes_per_element;
    int m_display_width_chars;
    void (*m_formatter)(QString* target, int display_width, int byte_count, const uint16_t* values,
                        MemoryViewWidget::Endianess);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint64_t decode_value(const uint16_t* values, int count, MemoryViewWidget::Endianess endianess) {
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

static void format_hex(QString* target, int display_width, int byte_count, const uint16_t* values,
                       MemoryViewWidget::Endianess endianess) {
    const uint64_t value = decode_value(values, byte_count, endianess);

    int shift = (byte_count * 8) - 4;
    while (shift >= 0) {
        int val = (value >> shift) & 0xf;
        target->push_back(s_hex_table[val]);
        shift -= 4;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void assign_str(const char* str, int len, int display_width, QString* target) {
    for (int i = len; i < display_width; ++i) {
        target->push_back(QLatin1Char(' '));  // Right align
    }
    for (const char* p = str; *p; ++p) {
        target->push_back(QChar(int(*p)));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void format_unsigned(QString* target, int display_width, int byte_count, const uint16_t* values,
                            MemoryViewWidget::Endianess endianess) {
    const uint64_t value = decode_value(values, byte_count, endianess);
    char buffer[32];
    buffer[0] = 0;
    int len = 0;
    switch (byte_count) {
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
            len = snprintf(buffer, sizeof buffer, "%" PRIu64, (uint64_t)value);
            break;
    }

    assign_str(buffer, len, display_width, target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void format_signed(QString* target, int display_width, int byte_count, const uint16_t* values,
                          MemoryViewWidget::Endianess endianess) {
    const uint64_t value = decode_value(values, byte_count, endianess);
    char buffer[32];
    buffer[0] = 0;
    int len = 0;
    switch (byte_count) {
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
            len = snprintf(buffer, sizeof buffer, "%" PRId64, (int64_t)value);
            break;
    }

    assign_str(buffer, len, display_width, target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void formatFloat(QString* target, int display_width, int byte_count, const uint16_t* values,
                        MemoryViewWidget::Endianess endianess) {
    const uint64_t value = decode_value(values, byte_count, endianess);

    union {
        uint64_t i;
        double f;
    } itof64;

    union {
        uint32_t i;
        float f;
    } itof32;

    char buffer[32];
    buffer[0] = 0;
    int len = 0;
    switch (byte_count) {
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

    assign_str(buffer, len, display_width, target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const struct MemViewTypeMeta s_type_meta[] = {
    {1, 2, format_hex},        // kHexByte: FF
    {1, 3, format_unsigned},   // kUnsignedByte: 255
    {1, 4, format_signed},     // kSignedByte: -128
    {2, 4, format_hex},        // kHexWord: FFFF
    {2, 5, format_unsigned},   // kUnsignedWord: 65535
    {2, 6, format_signed},     // kSignedWord: -32768
    {4, 8, format_hex},        // kHexDword: FFFFFFFF
    {4, 10, format_unsigned},  // kUnsignedDword: 4294967295
    {4, 11, format_signed},    // kSignedDword: -2147483648
    {8, 16, format_hex},       // kHexQword: FFFFFFFFFFFFFFFF
    {8, 20, format_unsigned},  // kUnsignedQword: 18446744073709551615
    {8, 20, format_signed},    // kSignedQword: -9223372036854775808
    {4, 16, formatFloat},      // kFloat: arbitrary formatting width
    {8, 16, formatFloat},      // kDouble: arbitrary formatting width
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QChar s_ascii_tab[256];

class MemoryViewPrivate {
public:
    QPointer<prodbg::IBackendRequests> m_interface;

    int m_elements_per_row = 8;
    MemoryViewWidget::DataType m_data_type = MemoryViewWidget::X8;
    MemoryViewWidget::Endianess m_endianess = MemoryViewWidget::Little;

    int m_wheel_speed_rows = 4;
    int m_page_size_in_rows = 16;
    uint64_t m_top_row = 0;
    int m_address_width = 8;

    uint64_t m_cached_range_start = 0;
    uint64_t m_cached_range_end = 0;
    bool m_transfer_in_progress = false;
    bool m_expression_status = true;

    QVector<uint16_t> m_cache;
    QVector<uint16_t> m_transfer_cache;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void access(uint64_t address, uint64_t count, QVector<uint16_t>* values) {
        uint64_t start = address;
        uint64_t end = address + count;

        values->resize(0);

        // Very basic caching, we only support the exactly previous request as cache.
        if ((m_cached_range_start == start && end == m_cached_range_end) && !m_transfer_in_progress) {
            *values = m_transfer_cache;
            return;
        }

        // fill with dummy data for now until done
        for (uint64_t ai = address, ae = address + count; ai != ae; ++ai) {
            values->push_back(0 & 0xff);  // hack test
        }

        /*
        if (m_interface) {
            m_interface->beginReadMemory(start, end, &m_transfer_cache);
            m_transfer_in_progress = true;
        }
        */
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void jump(int row_count) {
        m_top_row += row_count * m_elements_per_row * bytes_per_element();
    }

    int bytes_per_element() const {
        return s_type_meta[m_data_type].m_bytes_per_element;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void paintEvent(QWidget* widget, QPaintEvent* ev) {
        QFont font = widget->font();
        QFontMetrics font_metrics(font);
        // QRect widgetRect = widget->rect();
        QRect dirty_rect = ev->rect();
        const int elements_per_row = m_elements_per_row;
        const int bytes_per_row = elements_per_row * bytes_per_element();

        //QColor base_color = QApplication::palette().brush(QPalette::Background).color();

        QPainter painter(widget);
        painter.setFont(font);

        //painter.fillRect(dirty_rect, base_color);

        if (!m_expression_status) {
            painter.drawText(dirty_rect, 0, QStringLiteral("Unable to evaluate expression."));
            return;
        }

        const int char_width = font_metrics.boundingRect(QLatin1Char('W')).width();
        const int row_height = font_metrics.height();
        const int rows = (widget->height() + row_height - 1) / row_height;

        uint64_t first_byte = m_top_row;
        uint64_t last_byte = m_top_row + bytes_per_row * rows;

        m_cache.clear();
        access(first_byte, last_byte - first_byte, &m_cache);

        int screen_y = 0;
        int data_offset = 0;

        const MemViewTypeMeta& type_meta = s_type_meta[m_data_type];

        QString row_text;
        row_text.reserve((1 + type_meta.m_display_width_chars));

        const int address_width = (m_address_width * 2) * char_width;
        const int data_width = char_width * (elements_per_row * type_meta.m_display_width_chars + elements_per_row - 1);
        const int ascii_width = bytes_per_row * char_width;
        const int gutter_width = char_width;

        for (int row = 0; row < rows; ++row) {
            QRect address_rect(0, screen_y, address_width, row_height);
            QRect data_rect(address_rect.x() + address_rect.width() + gutter_width, screen_y, data_width, row_height);
            QRect ascii_rect(data_rect.x() + data_rect.width() + gutter_width, screen_y, ascii_width, row_height);

            if (dirty_rect.intersects(address_rect)) {
                row_text.resize(0);
                uint64_t addr = (m_top_row + data_offset);

                uint32_t byte_shift = 56;

                if (m_address_width == 4) {
                    byte_shift = 24;
                } else {
                    byte_shift = 8;
                }

                for (int i = 0; i < m_address_width; ++i) {
                    uint8_t byte = addr >> byte_shift;
                    row_text.append(s_hex_table[byte >> 4]);
                    row_text.append(s_hex_table[byte & 0xf]);
                    addr <<= 8;
                }

                painter.drawText(address_rect, 0, row_text);
            }

            if (dirty_rect.intersects(data_rect)) {
                row_text.resize(0);

                for (int i = 0; i < elements_per_row; ++i) {
                    if (i > 0) {
                        row_text.push_back(QLatin1Char(' '));
                    }

                    const uint16_t* values = m_cache.constData() + data_offset + i * type_meta.m_bytes_per_element;
                    (*type_meta.m_formatter)(&row_text, type_meta.m_display_width_chars, type_meta.m_bytes_per_element,
                                             values, m_endianess);
                }

                painter.drawText(data_rect, 0, row_text);
            }

            if (dirty_rect.intersects(ascii_rect)) {
                row_text.resize(0);
                for (int i = 0; i < bytes_per_row; ++i) {
                    uint16_t value = m_cache.at(i + data_offset);
                    uint8_t byte = value & 0xff;
                    row_text.append(s_ascii_tab[byte]);
                }

                painter.drawText(ascii_rect, 0, row_text);
            }

            screen_y += row_height;
            data_offset += bytes_per_row;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewWidget::MemoryViewWidget(QWidget* parent) : Base(parent), m_private(new MemoryViewPrivate) {
    // Can be any fixed with font.
#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    font.setFixedPitch(true);
    setFont(font);

    setFocusPolicy(Qt::StrongFocus);

    if (!s_ascii_tab[int('a')].unicode()) {
        for (int i = 0; i < 256; ++i) {
            if (isprint(i)) {
                s_ascii_tab[i] = QLatin1Char(i);
            } else {
                s_ascii_tab[i] = QLatin1Char('.');
            }
        }
    }

    {
        QAction* next_page_action = new QAction(QStringLiteral("Next Page"), this);
        next_page_action->setShortcut(QKeySequence::MoveToNextPage);
        next_page_action->setShortcutContext(Qt::WidgetShortcut);
        addAction(next_page_action);
        connect(next_page_action, &QAction::triggered, this, &MemoryViewWidget::display_next_page);
    }

    {
        QAction* prev_page_action = new QAction(QStringLiteral("Previous Page"), this);
        prev_page_action->setShortcut(QKeySequence::MoveToPreviousPage);
        prev_page_action->setShortcutContext(Qt::WidgetShortcut);
        addAction(prev_page_action);
        connect(prev_page_action, &QAction::triggered, this, &MemoryViewWidget::display_prev_page);
    }

    {
        QAction* next_line_action = new QAction(QStringLiteral("Next Line"), this);
        next_line_action->setShortcut(QKeySequence::MoveToNextLine);
        next_line_action->setShortcutContext(Qt::WidgetShortcut);
        addAction(next_line_action);
        connect(next_line_action, &QAction::triggered, this, &MemoryViewWidget::display_next_line);
    }

    {
        QAction* prev_line_action = new QAction(QStringLiteral("Previous Line"), this);
        prev_line_action->setShortcut(QKeySequence::MoveToPreviousLine);
        prev_line_action->setShortcutContext(Qt::WidgetShortcut);
        addAction(prev_line_action);
        connect(prev_line_action, &QAction::triggered, this, &MemoryViewWidget::display_prev_line);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewWidget::~MemoryViewWidget() {
    delete m_private;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::set_backend_interface(prodbg::IBackendRequests* interface) {
    m_private->m_interface = interface;

    /*
    if (interface) {
        connect(interface, &IBackendRequests::endReadMemory, this, &MemoryViewWidget::endReadMemory);
        connect(interface, &IBackendRequests::program_counter_changed, this,
    &MemoryViewWidget::program_counter_changed);
    }
    */

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::program_counter_changed(const prodbg::IBackendRequests::ProgramCounterChange&) {
    // If pc has changed we re-request the current data again
    /*
    if (m_private->m_interface) {
        m_private->m_interface->beginReadMemory(m_private->m_cached_range_start, m_private->m_cached_range_end,
                                                &m_private->m_transfer_cache);
        m_private->m_transfer_in_progress = true;
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gets called when transfor from backend to frontend has finished

void MemoryViewWidget::end_read_memory(QVector<uint16_t>* target, uint64_t address, int address_width) {
    // so this is a hack. We need a better way to do this. This is because if
    // there are several memory requests in flight we must make sure that its
    // "ours" that gets called here.
    if (target != &m_private->m_transfer_cache) {
        return;
    }

    m_private->m_address_width = address_width;

    m_private->m_cached_range_start = address;
    m_private->m_cached_range_end = address + target->count();

    m_private->m_transfer_in_progress = false;

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::set_expression_status(bool status) {
    m_private->m_expression_status = status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::paintEvent(QPaintEvent* ev) {
    m_private->paintEvent(this, ev);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::display_next_page() {
    m_private->jump(m_private->m_page_size_in_rows);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::display_prev_page() {
    m_private->jump(-m_private->m_page_size_in_rows);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::display_next_line() {
    m_private->jump(1);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::display_prev_line() {
    m_private->jump(-1);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::contextMenuEvent(QContextMenuEvent* ev) {
    QMenu contextMenu;
    contextMenu.addActions(actions());
    contextMenu.exec(mapToGlobal(ev->pos()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::wheelEvent(QWheelEvent* ev) {
    if (ev->angleDelta().y() < 0) {
        m_private->jump(m_private->m_wheel_speed_rows);
    } else if (ev->angleDelta().y() > 0) {
        m_private->jump(-m_private->m_wheel_speed_rows);
    }

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewWidget::DataType MemoryViewWidget::data_type() const {
    return m_private->m_data_type;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::set_data_type(DataType type) {
    m_private->m_data_type = type;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewWidget::Endianess MemoryViewWidget::endianess() const {
    return m_private->m_endianess;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::set_endianess(Endianess e) {
    m_private->m_endianess = e;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MemoryViewWidget::elements_per_line() const {
    return m_private->m_elements_per_row;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::set_elements_per_line(int count) {
    m_private->m_elements_per_row = std::min(std::max(1, count), 64);
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewWidget::set_address(uint64_t address) {
    m_private->m_top_row = address;
    update();
}
