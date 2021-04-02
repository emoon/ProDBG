#pragma once

#include "backend/backend_requests_interface.h"
#include "pd_view.h"

#include <QtWidgets/QWidget>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QAbstractItemModel>

class Ui_CallstackView;

class CallstackModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CallstackView : public PDView {
    Q_OBJECT
public:
    PDView* create(QWidget* parent);
    void set_backend_interface(PDIBackendRequests* iface);

    virtual ~CallstackView();

private:
    void init(QWidget* parent);
    int version() { return PRODG_VIEW_VERSION; }
    QString name() { return QStringLiteral("Callstack"); }

    Q_SLOT void reply_callstack(const PDIBackendRequests::Callstack& variables);
    Q_SLOT void program_counter_changed(const PDIBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void item_double_clicked(const QModelIndex& item);

    CallstackModel* m_model = nullptr;
    PDIBackendRequests* m_interface = nullptr;
    Ui_CallstackView* m_ui = nullptr;
};

