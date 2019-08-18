#pragma once

<<<<<<< HEAD
#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "api/include/pd_ui.h"
#include "Backend/IBackendRequests.h"

class Ui_RegisterView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegistersPlugin : public QObject, prodbg::PDUIInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.prodbg.PDUIInterface" FILE "registers_view.json")
    Q_INTERFACES(prodbg::PDUIInterface)

public:
    void create(QWidget* parent);
    void set_backend_interface(prodbg::IBackendRequests* interface);

    Q_SLOT void endReadRegisters(QVector<prodbg::IBackendRequests::Register>* target);
    Q_SLOT void programCounterChanged(const prodbg::IBackendRequests::ProgramCounterChange& pc);
private:

    // Interface for the backend
    QPointer<prodbg::IBackendRequests> m_backend;

    Ui_RegisterView* m_ui = nullptr;
=======
//#include "../../api/include/pd_ui.h"
#include "api/include/pd_ui.h"

class RegistersPlugin : public QObject, PDUIInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.prodbg.PDUIInterface" FILE "registers_view.json")
    Q_INTERFACES(PDUIInterface)

public:
    virtual void test(int t);
>>>>>>> 8b9929dcf62565f0e6b23d15f245e4c7afb83f65
};

