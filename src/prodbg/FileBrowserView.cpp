#include "FileBrowserView.h"
//#include <QtCore/QAbstractItemModel>
//#include <QtCore/QAbstractListModel>
#include <QtWidgets/QFileSystemModel>
#include <QtCore/QDebug>
#include "ui_FileBrowserView.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
class FileBrowserModel : public QAbstractTableModel {
    Q_OBJECT
public:
    PlaylistModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    struct Entry {
        QString path;
        QString name;
    };

    QString m_filter;

    std::vector<Entry> m_filter_entries;
    std::vector<Entry> m_entries;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FileBrowserModel::rowCount(const QModelIndex& parent) {
    return m_entries.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FileBrowserModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant FileBrowserModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return m_entries[index.row()].name;
        }
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant FileBrowserModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return QStringLiteral("File");
        }
    }

    return QVariant();
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBrowserView::FileBrowserView(QWidget* parent)
    : QWidget(parent), m_filesystem_model(new QFileSystemModel), m_ui(new Ui_FileBrowserView) {
    m_ui->setupUi(this);
    QString path = QStringLiteral("/home/emoon/code/projects/prodbg/");
    m_filesystem_model->setRootPath(path);
    m_ui->files->setModel(m_filesystem_model);
    m_ui->files->setRootIndex(m_filesystem_model->index(path));
    m_ui->files->setItemsExpandable(true);

    QObject::connect(m_ui->files, &QTreeView::doubleClicked, this, &FileBrowserView::item_double_clicked);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBrowserView::~FileBrowserView() {
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBrowserView::item_double_clicked(const QModelIndex& item) {
    open_file_signal(m_filesystem_model->filePath(item));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBrowserView::set_backend_interface(IBackendRequests* iface) {
    m_interface = iface;

    /*
    if (iface) {
        connect(m_interface, &IBackendRequests::program_counter_changed, this,
                &FileBrowserView::program_counter_changed);
        connect(m_interface, &IBackendRequests::reply_source_files, this, &FileBrowserView::reply_source_files);
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void FileBrowserView::program_counter_changed(const IBackendRequests::ProgramCounterChange& pc) {
    // if we don't have any files we request them from the backend
    if (m_model->m_entries.size() != 0) {
        m_interface->reply_source_files();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBrowserView::reply_source_files(const QVector<QString>& files) {
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
