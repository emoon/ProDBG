#pragma once

#include <QWidget>
#include <QPointer>

class Ui_RegisterView;

namespace prodbg {

class BackendInterface;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegisterView : public QWidget
{
    Q_OBJECT;

public:
    RegisterView(QWidget* parent = nullptr);
    ~RegisterView();

    void setBackendInterface(BackendInterface* interface);

private:
    Q_SLOT void dataTypeChanged(int);

private:
    Ui_RegisterView* m_ui = nullptr;
    QPointer<BackendInterface> m_interface;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
