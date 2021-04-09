#include "amiga_framebuffer.h"
#include "../amiga_structs.h"
#include "backend/backend_requests_interface.h"

#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtCore/QDebug>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BufferRender::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    printf("paint!\n");

    if (m_image) {
        painter.drawImage(rect(), *m_image);
    }

    /*
    if (!m_buffer) {
        QRect rect = geometry();
        qDebug() << rect;
        painter.fillRect(rect, QColor(Qt::black));
    }
    */

    printf("%d %p\n", m_request_in_progress, m_backend_interface);

    if (!m_request_in_progress && m_backend_interface) {
        QVector<uint8_t> dummy;
        printf("request framebuffer\n");
        m_backend_interface->request_custom(AmigaMessages_RequestFramebuffer, &m_frame_buffer_data, nullptr);
        m_request_in_progress = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BufferRender::frame_buffer_data(int message_id, QVector<uint8_t>* data) {
    if (message_id != AmigaMessages_ReplyFrameBuffer) {
        return;
    }

    AmigaFrameBufferData* d = (AmigaFrameBufferData*)data->data();

    delete m_image;
    m_image = new QImage((uint8_t*)&d->data[0], d->width, d->height, QImage::Format_RGBX8888);

    printf("got data from backend\n");
    m_request_in_progress = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmigaFrameBuffer::~AmigaFrameBuffer() {
    delete m_buffer_render;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaFrameBuffer::set_backend_interface(PDIBackendRequests* iface) {
    m_interface = iface;
    m_buffer_render->m_backend_interface = iface;
    QObject::connect(iface, &PDIBackendRequests::reply_custom, m_buffer_render, &BufferRender::frame_buffer_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaFrameBuffer::init(QWidget* parent) {
    m_buffer_render = new BufferRender(parent);
    m_buffer_render->update();
    m_buffer_render->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_buffer_render->setMinimumSize(755, 286);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDView* AmigaFrameBuffer::create(QWidget* parent) {
    auto instance = new AmigaFrameBuffer;
    instance->init(parent);
    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

