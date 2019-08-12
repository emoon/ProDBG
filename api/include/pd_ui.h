#pragma once

#include <QtCore/QObject>

class PDUIInterface
{
public:
    virtual ~PDUIInterface() {}
    virtual void test(int t) = 0;
};

#define PDUIInterface_iid "org.prodbg.PDUIInterface"

Q_DECLARE_INTERFACE(PDUIInterface, PDUIInterface_iid)
