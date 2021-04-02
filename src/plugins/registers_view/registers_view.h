#pragma once

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "api/include/pd_ui.h"
#include "backend/backend_requests_interface.h"

class Ui_RegisterView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegistersPlugin : public prodbg::View
{
    Q_OBJECT

public:
    prodbg::View* create(QWidget* parent);
    void set_backend_interface(prodbg::IBackendRequests* interface);
    ~RegistersPlugin();

private:
    void init(QWidget* parent);
    QPointer<prodbg::IBackendRequests> m_backend;
    Ui_RegisterView* m_ui = nullptr;
};
