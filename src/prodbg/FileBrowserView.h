#pragma once

#include <QtWidgets/QWidget>
#include "Backend/BackendRequests.h"
#include <QtCore/QVector>
#include <QtCore/QString>

class Ui_FileBrowserView;
class QFileSystemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class IBackendRequests;
//class FileBrowserModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FileBrowserView : public QWidget {
    Q_OBJECT
public:
    void set_backend_interface(IBackendRequests* iface);

    //Q_SLOT void program_counter_changed(const IBackendRequests::ProgramCounterChange& pc);
    //Q_SLOT void reply_source_files(const QVector<QString>& files);

    explicit FileBrowserView(QWidget* parent);
    virtual ~FileBrowserView();

    Q_SIGNAL void open_file_signal(const QString& filename);

private:

    Q_SLOT void item_double_clicked(const QModelIndex& item);

    QFileSystemModel* m_filesystem_model = nullptr;

    //FileBrowserModel* m_model = nullptr;
    IBackendRequests* m_interface = nullptr;
    Ui_FileBrowserView* m_ui = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
