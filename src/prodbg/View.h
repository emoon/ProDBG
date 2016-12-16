#pragma once

#include <QWidget>
#include <QPointer>

namespace prodbg {

class IBackendRequests;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class View : public QWidget
{
    Q_OBJECT

public:
    explicit View(QWidget* parent);
    virtual ~View();

    void setBackendInterface(IBackendRequests* interface);

    // not very nice. Just for testing now
    virtual void interfaceSet() = 0;

    virtual void readSettings() = 0;
    virtual void writeSettings() = 0;

protected:
    QPointer<IBackendRequests> m_interface;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
