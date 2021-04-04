#pragma once

#include <QtCore/QObject>

class PDIBackendRequests;
class QWidget;

#define PRODG_VIEW_VERSION 1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PDView : public QObject
{
public:
    virtual ~PDView() {}

    // Has to return PRODG_VIEW_VERSION. This is to allow supporting different versions of the interface
    virtual int version() = 0;

    // Name of the plugin
    virtual const char* name() = 0;

    // TODO: Return state if needs animated update
    virtual PDView* create(QWidget* parent) = 0;

    // This gets called from the outside and sets an instance to the backend interface.
    // The backend interface is used by the plugin to request data from the backend (such as memory, registers, etc)
    // The plugin can also attach to certain events (such as the ProgramCounter change and request new data upon such events)
    virtual void set_backend_interface(PDIBackendRequests* interface) = 0;
};

