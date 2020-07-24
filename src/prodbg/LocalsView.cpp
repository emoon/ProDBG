#include "LocalsView.h"
//#include <QtCore/QAbstractItemModel>
//#include <QtCore/QAbstractListModel>
#include <QtCore/QDebug>
#include <QtWidgets/QFileSystemModel>
#include "ui_LocalsView.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Node {
    Node(const QString& name, const QString& data, const QString& type, bool may_have_children)
        : parent(nullptr), name(name), data(data), type(type), may_have_children(may_have_children) {
    }
    ~Node() {
        qDeleteAll(children);
    }

    Node* parent;
    QString name;
    QString data;
    QString type;
    bool may_have_children;

    QList<Node*> children;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalsModel : public QAbstractItemModel {
public:
    LocalsModel(QObject* parent = 0);
    ~LocalsModel();

    void set_root_node(Node* node);

    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& child) const;

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    Node* node_from_index(const QModelIndex& index) const;
    Node* m_root_node;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LocalsModel::LocalsModel(QObject* parent) : QAbstractItemModel(parent) {
    m_root_node = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LocalsModel::~LocalsModel() {
    delete m_root_node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LocalsModel::columnCount(const QModelIndex& parent) const {
    return 3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant LocalsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return QStringLiteral("Name");
            case 1:
                return QStringLiteral("Value");
            case 2:
                return QStringLiteral("Type");
            default:
                break;
        }
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Node* LocalsModel::node_from_index(const QModelIndex& index) const {
    if (index.isValid()) {
        return static_cast<Node*>(index.internalPointer());
    } else {
        return m_root_node;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LocalsModel::rowCount(const QModelIndex& parent) const {
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

QModelIndex LocalsModel::index(int row, int column, const QModelIndex& parent) const {
    if (!m_root_node || row < 0 || column < 0) {
        return QModelIndex();
    }

    Node* parent_node = node_from_index(parent);
    Node* childNode = parent_node->children.value(row);

    if (!childNode) {
        return QModelIndex();
    }

    return createIndex(row, column, childNode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QModelIndex LocalsModel::parent(const QModelIndex& child) const {
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

QVariant LocalsModel::data(const QModelIndex& index, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    Node* node = node_from_index(index);
    if (!node) {
        return QVariant();
    }

    switch (index.column()) {
        case 0:
            return node->name;
        case 1:
            return node->data;
        case 2:
            return node->type;
        default:
            break;
    }

    return QVariant();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LocalsView::LocalsView(QWidget* parent) : QWidget(parent), m_model(new LocalsModel), m_ui(new Ui_LocalsView) {
    m_ui->setupUi(this);

    m_ui->locals->setModel(m_model);

    // QObject::connect(m_ui->files, &QTreeView::doubleClicked, this, &LocalsView::item_double_clicked);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LocalsView::~LocalsView() {
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::set_backend_interface(IBackendRequests* iface) {
    m_interface = iface;

    /*
    if (iface) {
        connect(m_interface, &IBackendRequests::program_counter_changed, this,
                &LocalsView::program_counter_changed);
        connect(m_interface, &IBackendRequests::reply_source_files, this, &LocalsView::reply_source_files);
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void LocalsView::program_counter_changed(const IBackendRequests::ProgramCounterChange& pc) {
    // if we don't have any files we request them from the backend
    if (m_model->m_entries.size() != 0) {
        m_interface->reply_source_files();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::reply_source_files(const QVector<QString>& files) {
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
