#pragma once

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "pd_memory_view.h"
#include "backend/backend_requests_interface.h"

//class Ui_ImageView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ImageView : public PDMemoryView
{
    Q_OBJECT
public:
    ~ImageView();
    PDMemoryView::Ver version() { return PDMemoryView::Ver::Version; }
    QString name() { return QStringLiteral("Image"); }
    PDMemoryView* create(QWidget* parent);
    void set_backend_interface(PDIBackendRequests* interface);

private:
    //QPointer<prodbg::IBackendRequests> m_backend;
    //Ui_ImageView* m_ui = nullptr;
};

