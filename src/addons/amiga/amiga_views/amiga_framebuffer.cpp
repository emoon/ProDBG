#include "amiga_framebuffer.h"
#include "../amiga_structs.h"

#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BufferRender : public QWidget {
    //Q_OBJECT
public:
    BufferRender(QWidget* parent) : QWidget(parent) {}
    ~BufferRender() { }

    void paintEvent(QWidget* widget, QPaintEvent* event) {
        QPainter painter(widget);

        if (!m_buffer) {
            QRect rect = event->rect();
            painter.fillRect(rect, QColor(Qt::black));
        }
    }

    AmigaFrameBuffer* m_buffer = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmigaFrameBuffer::~AmigaFrameBuffer() {
    delete m_buffer_render;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaFrameBuffer::init(QWidget* parent) {
    m_buffer_render = new BufferRender(parent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDView* AmigaFrameBuffer::create(QWidget* parent) {
    auto instance = new AmigaFrameBuffer;
    instance->init(parent);
    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

