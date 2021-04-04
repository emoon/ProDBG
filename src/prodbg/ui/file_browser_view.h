#pragma once

#include <QtWidgets/QWidget>
#include "backend/backend_requests_interface.h"
#include "pd_view.h"
#include <QtCore/QVector>
#include <QtCore/QString>

class Ui_FileBrowserView;
class QFileSystemModel;
class QSortFilterProxyModel;

class PDIBackendRequests;
class FileBrowserModel;
class Node;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FileBrowserView : public PDView {
    Q_OBJECT
public:
    PDView* create(QWidget* parent);
    void set_backend_interface(PDIBackendRequests* iface);

    Q_SLOT void program_counter_changed(const PDIBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void reply_source_files(const QVector<QString>& files);

    virtual ~FileBrowserView();

    Q_SIGNAL void open_file_signal(const QString& filename, bool set_active);

private:

    void init(QWidget* parent);
    int version() { return PRODG_VIEW_VERSION; }
    const char* name() { return "File Browser"; }

    Q_SLOT void open_item(const QModelIndex& item);
    Q_SLOT void filter_updated(const QString& text);

    QSortFilterProxyModel* m_filter_model;
    bool m_has_requested_files = false;
    Node* m_root_node = nullptr;
    FileBrowserModel* m_model = nullptr;
    PDIBackendRequests* m_interface = nullptr;
    Ui_FileBrowserView* m_ui = nullptr;
};

