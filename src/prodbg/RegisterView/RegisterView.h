#pragma once

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "Backend/IBackendRequests.h"
#include "View.h"

class Ui_RegisterView;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegisterView : public View {
    Q_OBJECT;

   public:
    RegisterView(QWidget* parent = nullptr);
    ~RegisterView();

    static View* createView(QWidget* parent);
    const QString& name();

    void interfaceSet();
    void readSettings();
    void writeSettings();

   private:
    Q_SLOT void endReadRegisters(QVector<IBackendRequests::Register>* target);
    Q_SLOT void programCounterChanged(
        const IBackendRequests::ProgramCounterChange& pc);

   private:
    Ui_RegisterView* m_ui = nullptr;
    QVector<IBackendRequests::Register> m_targetRegisters;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
