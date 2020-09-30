#pragma once

#include <QtCore/QObject>

namespace prodbg {

class IBackendRequests;

class PDUIInterface
{
public:
    virtual ~PDUIInterface() {}
    // TODO: Return state if needs animated update
    virtual PDUIInterface* create(QWidget* parent) = 0;

    // This gets called from the outside and sets an instance to the backend interface.
    // The backend interface is used by the plugin to request data from the backend (such as memory, registers, etc)
    // The plugin can also attach to certain events (such as the ProgramCounter change and request new data upon such events)
    virtual void set_backend_interface(prodbg::IBackendRequests* interface) = 0;
};

}

#define PDUIInterface_iid "org.prodbg.PDUIInterface"
Q_DECLARE_INTERFACE(prodbg::PDUIInterface, PDUIInterface_iid)
