#pragma once

#include "Backend/IBackendRequests.h"
#include <QPointer>
#include <QVector>
#include <QWidget>

class Ui_RegisterView;

namespace prodbg {

class IBackendRequests;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegisterView : public QWidget
{
    Q_OBJECT;

public:
    RegisterView(QWidget* parent = nullptr);
    ~RegisterView();

    void setBackendInterface(IBackendRequests* interface);

private:
    Q_SLOT void endReadRegisters(QVector<IBackendRequests::Register>* target);
    Q_SLOT void programCounterChanged(const IBackendRequests::ProgramCounterChange& pc);

private:
    Ui_RegisterView* m_ui = nullptr;
    QPointer<IBackendRequests> m_interface;
    QVector<uint16_t> m_memRes;
    QVector<IBackendRequests::Register> m_targetRegisters;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
