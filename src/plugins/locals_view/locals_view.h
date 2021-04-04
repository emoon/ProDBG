#pragma once

#include "backend/backend_requests.h"
#include "pd_view.h"
#include <QtCore/QAbstractItemModel>
#include <QtCore/QString>
#include <QtCore/QVector>

class Ui_LocalsView;
class QFileSystemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalsModel;
class LocalNode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalsView : public PDView {
    Q_OBJECT

public:
    PDView* create(QWidget* parent);
    void set_backend_interface(PDIBackendRequests* iface);
    virtual ~LocalsView();

private:
    void init(QWidget* parent);
    int version() { return PRODG_VIEW_VERSION; }
    const char* name() { return "Locals"; }

    Q_SLOT void reply_locals(const PDIBackendRequests::Variables& variables);
    Q_SLOT void program_counter_changed(const PDIBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void expand_variable(const QModelIndex& index);
    Q_SLOT void collpase_variable(const QModelIndex& index);

    PDIBackendRequests* m_interface = nullptr;
    LocalNode* m_root = nullptr;
    LocalsModel* m_model = nullptr;
    Ui_LocalsView* m_ui = nullptr;

    uint64_t m_request_id;
    QString m_locals_name_request;
    LocalNode* m_request_node = nullptr;
    PDIBackendRequests::ExpandVars m_expand_vars;
};


