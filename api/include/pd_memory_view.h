#pragma once

#include <stdint.h>
#include <QtCore/QObject>
#include <QtCore/QString>

namespace prodbg {

class IBackendRequests;

#define PRODG_MEMORY_VIEW_VERSION 1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MemoryView : public QObject
{
public:
    virtual ~MemoryView() {}

    // Has to return PRODG_MEMORY_VIEW_VERSION. This is to allow supporting different versions of the interface
    virtual int version() = 0;

    // Name of the plugin
    virtual QString name() = 0;

    // TODO: Return state if needs animated update
    virtual MemoryView* create(QWidget* parent) = 0;

    // This gets called from the outside and sets an instance to the backend interface.
    // The backend interface is used by the plugin to request data from the backend (such as memory, registers, etc)
    // The plugin can also attach to certain events (such as the ProgramCounter change and request new data upon such events)
    virtual void set_backend_interface(prodbg::IBackendRequests* interface) = 0;
};

}

