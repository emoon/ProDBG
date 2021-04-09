#include "image.h"
#include "pd_ui_register_plugin.h"
#include <QtWidgets/QPushButton>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDMemoryView* ImageView::create(QWidget* parent) {
    ImageView* instance = new ImageView;
    auto button = new QPushButton(QStringLiteral("Test"), parent);

    connect(button, &QPushButton::clicked, this, []() { printf("pressing button\n"); });

    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImageView::~ImageView() {

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::set_backend_interface(PDIBackendRequests* interface) {
    //m_interface = interface;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void pd_register_view(PDRegisterViewPlugin* reg) {
    reg->register_memory_view(new ImageView);
}