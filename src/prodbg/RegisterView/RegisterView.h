#pragma once

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
    Q_SLOT void dataTypeChanged(int);
    Q_SLOT void getSomeData();
    Q_SLOT void endReadMemory(QVector<uint16_t>* res, uint64_t address, int addressSize);

private:
    Ui_RegisterView* m_ui = nullptr;
    QPointer<IBackendRequests> m_interface;
    QVector<uint16_t> m_memRes;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
