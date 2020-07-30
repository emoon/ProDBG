#pragma once

#include <QtWidgets/QWidget>
#include "Backend/BackendRequests.h"
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QAbstractItemModel>

class Ui_LocalsView;
class QFileSystemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class IBackendRequests;
class LocalsModel;
class Node;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalsView : public QWidget {
    Q_OBJECT
public:
    void set_backend_interface(IBackendRequests* iface);

    explicit LocalsView(QWidget* parent);
    virtual ~LocalsView();

private:

    Q_SLOT void reply_locals(const IBackendRequests::Variables& variables);
    Q_SLOT void program_counter_changed(const IBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void expand_variable(const QModelIndex& index);
    Q_SLOT void collpase_variable(const QModelIndex& index);

    Node* m_root = nullptr;
    LocalsModel* m_model = nullptr;
    IBackendRequests* m_interface = nullptr;
    Ui_LocalsView* m_ui = nullptr;

    uint64_t m_request_id;
    QString m_locals_name_request;
    Node* m_request_node = nullptr;
    IBackendRequests::ExpandVars m_expand_vars;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

