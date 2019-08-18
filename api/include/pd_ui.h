#pragma once

#include <QtCore/QObject>

<<<<<<< HEAD
namespace prodbg {

class IBackendRequests;

=======
>>>>>>> 8b9929dcf62565f0e6b23d15f245e4c7afb83f65
class PDUIInterface
{
public:
    virtual ~PDUIInterface() {}
<<<<<<< HEAD
    // TODO: Return state if needs animated update
    virtual void create(QWidget* parent) = 0;

    // This gets called from the outside and sets an instance to the backend interface.
    // The backend interface is used by the plugin to request data from the backend (such as memory, registers, etc)
    // The plugin can also attach to certain events (such as the ProgramCounter change and request new data upon such events)
    virtual void set_backend_interface(prodbg::IBackendRequests* interface) = 0;
};

}

#define PDUIInterface_iid "org.prodbg.PDUIInterface"

Q_DECLARE_INTERFACE(prodbg::PDUIInterface, PDUIInterface_iid)
=======
    virtual void test(int t) = 0;
};

#define PDUIInterface_iid "org.prodbg.PDUIInterface"
>>>>>>> 8b9929dcf62565f0e6b23d15f245e4c7afb83f65

Q_DECLARE_INTERFACE(PDUIInterface, PDUIInterface_iid)
