#include "amiga_framebuffer.h"
#include "../amiga_structs.h"

#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtCore/QDebug>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BufferRender : public QWidget {
    //Q_OBJECT
public:
    BufferRender(QWidget* parent) : QWidget(parent) {}
    ~BufferRender() { }

    void paintEvent(QPaintEvent* event) {
        QPainter painter(this);

        printf("paint!\n");

        if (!m_buffer) {
            QRect rect = geometry();
            qDebug() << rect;
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
    m_buffer_render->update();
    m_buffer_render->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_buffer_render->setMinimumSize(400, 400);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDView* AmigaFrameBuffer::create(QWidget* parent) {
    auto instance = new AmigaFrameBuffer;
    instance->init(parent);
    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

