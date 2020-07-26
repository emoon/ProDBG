#pragma once

#include <QtWidgets/QWidget>
#include "Backend/BackendRequests.h"
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QAbstractItemModel>

class Ui_CallstackView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class CallstackModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CallstackView : public QWidget {
    Q_OBJECT
public:
    void set_backend_interface(IBackendRequests* iface);

    explicit CallstackView(QWidget* parent);
    virtual ~CallstackView();

private:

    Q_SLOT void reply_callstack(const IBackendRequests::Callstack& variables);
    Q_SLOT void program_counter_changed(const IBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void item_double_clicked(const QModelIndex& item);

    CallstackModel* m_model = nullptr;
    IBackendRequests* m_interface = nullptr;
    Ui_CallstackView* m_ui = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}


