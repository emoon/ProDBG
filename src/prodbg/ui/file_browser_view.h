#pragma once

#include <QtWidgets/QWidget>
#include "backend/backend_requests_interface.h"
#include <QtCore/QVector>
#include <QtCore/QString>

class Ui_FileBrowserView;
class QFileSystemModel;
class QSortFilterProxyModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class IBackendRequests;
class FileBrowserModel;
class Node;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FileBrowserView : public QWidget {
    Q_OBJECT
public:
    void set_backend_interface(IBackendRequests* iface);

    Q_SLOT void program_counter_changed(const IBackendRequests::ProgramCounterChange& pc);
    Q_SLOT void reply_source_files(const QVector<QString>& files);

    explicit FileBrowserView(QWidget* parent);
    virtual ~FileBrowserView();

    Q_SIGNAL void open_file_signal(const QString& filename, bool set_active);

private:

    Q_SLOT void open_item(const QModelIndex& item);
    Q_SLOT void filter_updated(const QString& text);

    QSortFilterProxyModel* m_filter_model;
    bool m_has_requested_files = false;
    Node* m_root_node = nullptr;
    FileBrowserModel* m_model = nullptr;
    IBackendRequests* m_interface = nullptr;
    Ui_FileBrowserView* m_ui = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
