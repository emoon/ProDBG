#pragma once

#include <QWidget>
#include <QPointer>

namespace prodbg {

class IBackendRequests;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class View : public QWidget
{
public:
    explicit View(QWidget* parent);
    virtual ~View();

    void setBackendInterface(IBackendRequests* interface);

    virtual void saveSettings() = 0;
    virtual void readSettings() = 0;

protected:
    QPointer<IBackendRequests> m_interface;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
