#include "locals_view.h"
#include <QtCore/QAbstractListModel>
#include <QtCore/QDebug>
#include <QtWidgets/QFileSystemModel>
#include "ui_locals_view.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct LocalNode {
    LocalNode(LocalNode* parent, const QString& name, const QString& data, const QString& type, int node_id,
         bool may_have_children)
        : parent(parent), name(name), data(data), type(type), node_id(node_id), may_have_children(may_have_children) {
        is_expanded = false;
    }
    ~LocalNode() {
        qDeleteAll(children);
    }

    LocalNode* parent;
    QString name;
    QString data;
    QString type;
    int node_id;
    bool is_expanded;
    bool may_have_children;

    QList<LocalNode*> children;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalsModel : public QAbstractItemModel {
public:
    LocalsModel(QObject* parent = 0);
    ~LocalsModel();

    void set_root_node(LocalNode* node) {
        m_root_node = node;
    }

    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& child) const;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    LocalNode* node_from_index(const QModelIndex& index) const;

private:
    LocalNode* m_root_node;
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

LocalNode* LocalsModel::node_from_index(const QModelIndex& index) const {
    if (index.isValid()) {
        return static_cast<LocalNode*>(index.internalPointer());
    } else {
        return m_root_node;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LocalsModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    LocalNode* parent_node = node_from_index(parent);
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

    LocalNode* parent_node = node_from_index(parent);
    LocalNode* child_node = parent_node->children.value(row);

    if (child_node) {
        return createIndex(row, column, child_node);
    }

    return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QModelIndex LocalsModel::parent(const QModelIndex& child) const {
    LocalNode* node = node_from_index(child);
    if (!node) {
        return QModelIndex();
    }

    LocalNode* parent_node = node->parent;
    if (!parent_node) {
        return QModelIndex();
    }

    LocalNode* prev_node = parent_node->parent;
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

    LocalNode* node = node_from_index(index);

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
    LocalNode* root = new LocalNode(nullptr, QStringLiteral(""), QStringLiteral(""), QStringLiteral(""), true);
    LocalNode* s = new LocalNode(root, QStringLiteral("Value 3"), QStringLiteral("{Foobar}"), QStringLiteral("FooBar"), true);

    root->children.push_back(
        new LocalNode(root, QStringLiteral("Value 0"), QStringLiteral("1.0"), QStringLiteral("float"), false));
    root->children.push_back(
        new LocalNode(root, QStringLiteral("Value 1"), QStringLiteral("2.0"), QStringLiteral("float"), false));
    root->children.push_back(
        new LocalNode(root, QStringLiteral("Value 2"), QStringLiteral("2.0"), QStringLiteral("float"), false));
    root->children.push_back(s);

    //s->children.push_back(new LocalNode(s, QStringLiteral("Value 4"), QStringLiteral("1.0"), QStringLiteral("float"),
   false));
    //s->children.push_back(new LocalNode(s, QStringLiteral("Value 5"), QStringLiteral("2.0"), QStringLiteral("float"),
   false));
    //s->children.push_back(new LocalNode(s, QStringLiteral("Value 6"), QStringLiteral("2.0"), QStringLiteral("float"),
   false));
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::init(QWidget* parent) {
    m_model = new LocalsModel;
    m_ui = new Ui_LocalsView;
    m_ui->setupUi(parent);
    m_ui->locals->setModel(m_model);
    m_ui->locals->setAlternatingRowColors(true);

    m_expand_vars.type = PDIBackendRequests::ExpandType::AllVariables;

    QObject::connect(m_ui->locals, &QTreeView::expanded, this, &LocalsView::expand_variable);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDView* LocalsView::create(QWidget* parent) {
    auto instance = new LocalsView;
    instance->init(parent);
    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LocalsView::~LocalsView() {
    delete m_ui;
    delete m_root;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::set_backend_interface(PDIBackendRequests* iface) {
    m_interface = iface;

    if (iface) {
        connect(m_interface, &PDIBackendRequests::program_counter_changed, this, &LocalsView::program_counter_changed);
        connect(m_interface, &PDIBackendRequests::reply_locals, this, &LocalsView::reply_locals);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::reply_locals(const PDIBackendRequests::Variables& vars) {
    if (vars.request_id != m_request_id) {
        return;
    }

    // handle the case if we have a sub-tree expanded

    if (m_request_node) {
        int node_id = 0;
        for (const auto& t : vars.variables) {
            m_request_node->children.push_back(
                new LocalNode(m_request_node, t.name, t.value, t.type, node_id, t.may_have_children));
            node_id++;
        }
        m_request_node->may_have_children = false;
    } else {
        // TODO: Fix memory leak
        /*
        if (m_root) {
            delete m_root;
        }
        */

        m_root = new LocalNode(nullptr, QStringLiteral(""), QStringLiteral(""), QStringLiteral(""), -1, true);
        int node_id = 0;

        for (const auto& t : vars.variables) {
            m_root->children.push_back(new LocalNode(m_root, t.name, t.value, t.type, node_id, t.may_have_children));
            node_id++;
        }
    }

    m_model->set_root_node(m_root);
    m_model->layoutChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::collpase_variable(const QModelIndex& index) {
    LocalNode* node = m_model->node_from_index(index);
    node->is_expanded = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::expand_variable(const QModelIndex& index) {
    PDIBackendRequests::ExpandVars expand_vars;

    LocalNode* node = m_model->node_from_index(index);
    node->is_expanded = true;

    int count = 0;

    LocalNode* current_node = node;

    // build expansion, first do a count, as we only traverse upwards this should be quite fast as this is the
    // number of levels some data has been expanded and it's likely not very super deep.

    while (m_root != current_node) {
        count++;
        current_node = current_node->parent;
    }

    expand_vars.tree.resize(count + 1);
    int write_index = count;
    current_node = node;

    // insert the tree indices

    while (m_root != current_node) {
        expand_vars.tree[write_index] = current_node->node_id;
        current_node = current_node->parent;
        write_index--;
    }
    expand_vars.tree[0] = count;
    expand_vars.type = PDIBackendRequests::ExpandType::Single;

    // TODO: Build fully from parent
    // m_locals_name_request = node->name;
    qDebug() << "expand node " << expand_vars.tree;

    m_request_id = m_interface->request_locals(expand_vars);
    m_request_node = node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalsView::program_counter_changed(const PDIBackendRequests::ProgramCounterChange& pc) {
    m_request_node = nullptr;
    printf("LocalsView::program_counter_changed request locals\n");
    m_request_id = m_interface->request_locals(m_expand_vars);
}

