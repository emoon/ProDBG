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
    Node(Node* parent, const QString& name, const QString& data, const QString& type, bool may_have_children)
        : parent(parent), name(name), data(data), type(type), may_have_children(may_have_children) {
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

    Node* node_from_index(const QModelIndex& index) const;
private:
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

    int count = parent_node->children.count();

    if (parent_node->may_have_children && count == 0) {
        return 1;
    } else {
        return count;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QModelIndex LocalsModel::index(int row, int column, const QModelIndex& parent) const {
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
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    Node* node = node_from_index(index);

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

Qt::ItemFlags LocalsModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

/*
    Node* root = new Node(nullptr, QStringLiteral(""), QStringLiteral(""), QStringLiteral(""), true);
    Node* s = new Node(root, QStringLiteral("Value 3"), QStringLiteral("{Foobar}"), QStringLiteral("FooBar"), true);

    root->children.push_back(
        new Node(root, QStringLiteral("Value 0"), QStringLiteral("1.0"), QStringLiteral("float"), false));
    root->children.push_back(
        new Node(root, QStringLiteral("Value 1"), QStringLiteral("2.0"), QStringLiteral("float"), false));
    root->children.push_back(
        new Node(root, QStringLiteral("Value 2"), QStringLiteral("2.0"), QStringLiteral("float"), false));
    root->children.push_back(s);

    //s->children.push_back(new Node(s, QStringLiteral("Value 4"), QStringLiteral("1.0"), QStringLiteral("float"), false));
    //s->children.push_back(new Node(s, QStringLiteral("Value 5"), QStringLiteral("2.0"), QStringLiteral("float"), false));
    //s->children.push_back(new Node(s, QStringLiteral("Value 6"), QStringLiteral("2.0"), QStringLiteral("float"), false));
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LocalsView::LocalsView(QWidget* parent) : QWidget(parent), m_model(new LocalsModel), m_ui(new Ui_LocalsView) {
    m_ui->setupUi(this);
    m_ui->locals->setModel(m_model);
    m_ui->locals->setAlternatingRowColors(true);

    QObject::connect(m_ui->locals, &QTreeView::expanded, this, &LocalsView::expand_variable);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LocalsView::~LocalsView() {
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::set_backend_interface(IBackendRequests* iface) {
    m_interface = iface;

    if (iface) {
        connect(m_interface, &IBackendRequests::program_counter_changed, this,
                &LocalsView::program_counter_changed);
        connect(m_interface, &IBackendRequests::reply_locals, this, &LocalsView::reply_locals);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::reply_locals(const IBackendRequests::Variables& vars) {
    if (vars.request_id != m_request_id) {
        return;
    }

    // handle the case if we have a sub-tree expanded

    if (m_request_node) {
        for (const auto& t : vars.variables) {
            m_request_node->children.push_back(new Node(m_request_node, t.name, t.value, t.type, t.may_have_children));
        }
        m_request_node->may_have_children = false;
    } else {
        // TODO: Fix memory leak
        /*
        if (m_root) {
            delete m_root;
        }
        */

        m_root = new Node(nullptr, QStringLiteral(""), QStringLiteral(""), QStringLiteral(""), true);

        for (const auto& t : vars.variables) {
            m_root->children.push_back(new Node(m_root, t.name, t.value, t.type, t.may_have_children));
        }
    }

    m_model->set_root_node(m_root);
    m_model->layoutChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::expand_variable(const QModelIndex& index) {
    // TODO: Build fully from parent
    Node* node = m_model->node_from_index(index);
    m_locals_name_request = node->name;
    qDebug() << "expand node " << m_locals_name_request;
    m_request_id = m_interface->request_locals(m_locals_name_request);
    m_request_node = node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::program_counter_changed(const IBackendRequests::ProgramCounterChange& pc) {
    m_request_node = nullptr;
    m_request_id = m_interface->request_locals(QStringLiteral(""));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
