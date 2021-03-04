#pragma once

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "api/include/pd_ui_memory.h"
#include "backend/backend_requests_interface.h"

//class Ui_ImageView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ImageView : public prodbg::MemoryView
{
    Q_OBJECT
public:
    ~ImageView();
    int version();
    QString name();
    prodbg::MemoryView* create(QWidget* parent);
    void set_backend_interface(prodbg::IBackendRequests* interface);

private:
    //QPointer<prodbg::IBackendRequests> m_backend;
    //Ui_ImageView* m_ui = nullptr;
};

