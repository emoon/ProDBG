#pragma once

#include <stdint.h>
#include <QtCore/QObject>
#include <QtCore/QString>

class PDIBackendRequests;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PDMemoryView : public QObject
{
public:
    virtual ~PDMemoryView() {}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class Ver {
        Version = 1,
    };

    virtual Ver version() = 0;

    // Name of the plugin
    virtual const char* name() = 0;

    // TODO: Return state if needs animated update
    virtual PDMemoryView* create(QWidget* parent) = 0;

    // This gets called from the outside and sets an instance to the backend interface.
    // The backend interface is used by the plugin to request data from the backend (such as memory, registers, etc)
    // The plugin can also attach to certain events (such as the ProgramCounter change and request new data upon such events)
    virtual void set_backend_interface(PDIBackendRequests* interface) = 0;
};

