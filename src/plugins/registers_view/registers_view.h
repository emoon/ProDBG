#pragma once

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "pd_view.h"
#include "backend/backend_requests_interface.h"

class Ui_RegisterView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegistersPlugin : public PDView
{
    Q_OBJECT

public:

    PDView* create(QWidget* parent);
    void set_backend_interface(PDIBackendRequests* interface);
    ~RegistersPlugin();

    int version() { return PRODG_VIEW_VERSION; }
    const char* name() { return "Registers"; }

private:
    void init(QWidget* parent);
    QPointer<PDIBackendRequests> m_backend;
    Ui_RegisterView* m_ui = nullptr;
};
