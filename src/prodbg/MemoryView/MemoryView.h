#pragma once

#include <QtWidgets/QWidget>

class Ui_MemoryView;

namespace prodbg {

class IBackendRequests;

class MemoryView : public QWidget
{
    using Base = QWidget;

    Q_OBJECT;

public:
    explicit MemoryView(QWidget* parent = nullptr);
    ~MemoryView();

    void setBackendInterface(IBackendRequests* interface);

public:
    Q_SLOT void jumpToAddressExpression(const QString& expression);

private:
    Q_SLOT void jumpAddressChanged();
    Q_SLOT void endianChanged(int);
    Q_SLOT void dataTypeChanged(int);
    Q_SLOT void countChanged(const QString&);

private:
    Ui_MemoryView* m_Ui = nullptr;
};
}
