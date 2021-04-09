#pragma once

#include "pd_view.h"
#include <QtWidgets/QWidget>
#include <QtGui/QImage>

class PDIBackendRequests;
class BufferRender;

class BufferRender : public QWidget {
    Q_OBJECT

public:
    BufferRender(QWidget* parent) : QWidget(parent) {}
    ~BufferRender() { }

    void paintEvent(QPaintEvent* event);
    Q_SLOT void frame_buffer_data(int message_id, QVector<uint8_t>* data);

    QVector<uint8_t> m_frame_buffer_data;
    bool m_request_in_progress = false;
    PDIBackendRequests* m_backend_interface = nullptr;
    QImage* m_image = nullptr;
    int m_old_width = 0;
    int m_old_height = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmigaFrameBuffer : public PDView {
    Q_OBJECT

public:
    PDView* create(QWidget* parent);
    virtual ~AmigaFrameBuffer();

private:
    void init(QWidget* parent);
    void set_backend_interface(PDIBackendRequests* iface);
    int version() { return PRODG_VIEW_VERSION; }
    const char* name() { return "Amiga:FrameBuffer"; }

    BufferRender* m_buffer_render;
    PDIBackendRequests* m_interface = nullptr;
};

