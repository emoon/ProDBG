#include "FileBrowserView.h"
//#include <QtCore/QAbstractItemModel>
//#include <QtCore/QAbstractListModel>
#include <QtWidgets/QFileSystemModel>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QDebug>
#include "ui_FileBrowserView.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Node {
    Node(Node* parent, const QString& name, const QString& fullpath, bool directory)
        : parent(parent), name(name), fullpath(fullpath), directory(directory) {
    }
    ~Node() {
        qDeleteAll(children);
    }

    Node* parent;
    QString name;
    QString fullpath;
    bool directory;

    QList<Node*> children;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FileBrowserModel : public QAbstractItemModel {
public:
    FileBrowserModel(QObject* parent = 0);
    ~FileBrowserModel();

    void set_root_node(Node* node) {
        m_root_node = node;
    }

    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& child) const;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QString m_filter;

    Node* node_from_index(const QModelIndex& index) const;
private:
    Node* m_root_node;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBrowserModel::FileBrowserModel(QObject* parent) : QAbstractItemModel(parent) {
    m_root_node = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBrowserModel::~FileBrowserModel() {
    delete m_root_node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FileBrowserModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant FileBrowserModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return QStringLiteral("Filename");
            default:
                break;
        }
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Node* FileBrowserModel::node_from_index(const QModelIndex& index) const {
    if (index.isValid()) {
        return static_cast<Node*>(index.internalPointer());
    } else {
        return m_root_node;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FileBrowserModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    Node* parent_node = node_from_index(parent);
    if (!parent_node) {
        return 0;
    }

    return parent_node->children.count();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QModelIndex FileBrowserModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    Node* parent_node = node_from_index(parent);
    Node* child_node = parent_node->children.value(row);

    if (child_node) {
        return createIndex(row, column, child_node);
    }

    return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QModelIndex FileBrowserModel::parent(const QModelIndex& child) const {
    Node* node = node_from_index(child);
    if (!node) {
        return QModelIndex();
    }

    Node* parent_node = node->parent;
    if (!parent_node) {
        return QModelIndex();
    }

    Node* prev_node = parent_node->parent;
    if (!prev_node) {
        return QModelIndex();
    }

    int row = prev_node->children.indexOf(parent_node);

    return createIndex(row, 0, parent_node);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant FileBrowserModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    Node* node = node_from_index(index);

    switch (index.column()) {
        case 0: {
            return node->name;
            break;
        }
        default:
            break;
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt::ItemFlags FileBrowserModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBrowserView::FileBrowserView(QWidget* parent)
    : QWidget(parent), m_model(new FileBrowserModel), m_ui(new Ui_FileBrowserView) {
    m_ui->setupUi(this);

    m_filter_model = new QSortFilterProxyModel(this);
    m_filter_model->setSourceModel(m_model);

    m_ui->files->setModel(m_filter_model);
    //m_ui->files->setSortingEnabled(true);

    QObject::connect(m_ui->filter, &QLineEdit::textChanged, this, &FileBrowserView::filter_updated);

    QObject::connect(m_ui->files, &QTreeView::activated, this, &FileBrowserView::open_item);
    QObject::connect(m_ui->files, &QTreeView::doubleClicked, this, &FileBrowserView::open_item);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBrowserView::~FileBrowserView() {
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBrowserView::filter_updated(const QString& text) {
    m_filter_model->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
    m_filter_model->setFilterKeyColumn(0);
    m_model->layoutChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBrowserView::open_item(const QModelIndex& item) {
    Node* t = m_model->node_from_index(m_filter_model->mapToSource(item));
    open_file_signal(t->fullpath, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBrowserView::set_backend_interface(IBackendRequests* iface) {
    m_interface = iface;

    if (iface) {
        connect(m_interface, &IBackendRequests::program_counter_changed, this,
                &FileBrowserView::program_counter_changed);
        connect(m_interface, &IBackendRequests::reply_source_files, this, &FileBrowserView::reply_source_files);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBrowserView::program_counter_changed(const IBackendRequests::ProgramCounterChange& pc) {
    if (m_root_node) {
        return;
    }

    printf("requesting source files\n");

    m_interface->request_basic(IBackendRequests::BasicRequest::SourceFiles);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBrowserView::reply_source_files(const QVector<QString>& files) {
    printf("reply_source_files\n");

    // TODO: For now only have a basic list of all the files
    m_root_node = new Node(nullptr, QStringLiteral(""), QStringLiteral(""), false);

    for (const auto& file : files) {
        QFileInfo info(file);

        if (!info.isFile()) {
            continue;
        }

        // TODO: Move this to backend?

        Node* node = new Node(m_root_node, info.fileName(), file, false);
        m_root_node->children.push_back(node);
    }

    m_model->set_root_node(m_root_node);
    m_model->layoutChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
