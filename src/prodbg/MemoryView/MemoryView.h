#pragma once

#include "Backend/IBackendRequests.h"
#include "View.h"
#include <QPointer>
#include <QtWidgets/QWidget>

class Ui_MemoryView;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IBackendRequests;

class MemoryView : public View
{
    using Base = View;

    Q_OBJECT;

public:
    explicit MemoryView(QWidget* parent = nullptr);
    ~MemoryView();

    static View* createView(QWidget* parent);

    void interfaceSet();
    void readSettings();
    void writeSettings();

public:
    Q_SLOT void jumpToAddressExpression(const QString& expression);

private:
    Q_SLOT void endResolveAddress(uint64_t* out);
    Q_SLOT void jumpAddressChanged();
    Q_SLOT void endianChanged(int);
    Q_SLOT void dataTypeChanged(int);
    Q_SLOT void countChanged(const QString&);

private:
    Ui_MemoryView* m_Ui = nullptr;
    uint64_t m_evalAddress = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
