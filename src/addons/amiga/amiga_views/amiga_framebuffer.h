#pragma once

#include "pd_view.h"

class PDIBackendRequests;
class BufferRender;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmigaFrameBuffer : public PDView {
    Q_OBJECT

public:
    PDView* create(QWidget* parent);
    void set_backend_interface(PDIBackendRequests* iface) { m_interface = iface; }
    virtual ~AmigaFrameBuffer();

private:
    void init(QWidget* parent);
    int version() { return PRODG_VIEW_VERSION; }
    const char* name() { return "Amiga:FrameBuffer"; }

    BufferRender* m_buffer_render;
    PDIBackendRequests* m_interface = nullptr;
};

