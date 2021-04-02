#pragma once

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

class PDIBackendRequests;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class View : public QWidget {
    Q_OBJECT

   public:
    explicit View(QWidget* parent);
    virtual ~View();

    void set_backend_interface(IBackendRequests* interface);

    // not very nice. Just for testing now
    virtual void interfaceSet() = 0;

    virtual void readSettings() = 0;
    virtual void writeSettings() = 0;

   protected:
    QPointer<IBackendRequests> m_interface;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
