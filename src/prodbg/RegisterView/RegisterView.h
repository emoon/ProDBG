#pragma once

#include <QWidget>
#include <QPointer>

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
    Q_SLOT void programCounterChanged(uint64_t pc);

private:
    Ui_RegisterView* m_ui = nullptr;
    QPointer<IBackendRequests> m_interface;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
