#pragma once

#include "backend/backend_requests.h"
#include "api/include/pd_ui.h"
#include <QtCore/QAbstractItemModel>
#include <QtCore/QString>
#include <QtCore/QVector>

class Ui_LocalsView;
class QFileSystemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalsModel;
class LocalNode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalsView : prodbg::View {
    Q_OBJECT

public:
    prodbg::View* create(QWidget* parent);
    void set_backend_interface(prodbg::IBackendRequests* iface);
    virtual ~LocalsView();

private:
    void init(QWidget* parent);

    Q_SLOT void reply_locals(const prodbg::IBackendRequests::Variables& variables);
    Q_SLOT void program_counter_changed(const prodbg::IBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void expand_variable(const QModelIndex& index);
    Q_SLOT void collpase_variable(const QModelIndex& index);

    prodbg::IBackendRequests* m_interface = nullptr;
    LocalNode* m_root = nullptr;
    LocalsModel* m_model = nullptr;
    Ui_LocalsView* m_ui = nullptr;

    uint64_t m_request_id;
    QString m_locals_name_request;
    LocalNode* m_request_node = nullptr;
    prodbg::IBackendRequests::ExpandVars m_expand_vars;
};


