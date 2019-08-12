#pragma once

//#include "../../api/include/pd_ui.h"
#include "api/include/pd_ui.h"

class RegistersPlugin : public QObject, PDUIInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.prodbg.PDUIInterface" FILE "registers_view.json")
    Q_INTERFACES(PDUIInterface)

public:
    virtual void test(int t);
};

