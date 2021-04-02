#include "callstack_view.h"
#include "ui_callstack_view.h"
#include <QtCore/QDebug>
#include <QtWidgets/QFileSystemModel>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CallstackModel : public QAbstractTableModel {
public:
    CallstackModel(QObject* parent);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

    PDIBackendRequests::Callstack m_callstack;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CallstackModel::CallstackModel(QObject* parent) : QAbstractTableModel(parent) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant CallstackModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return QStringLiteral("Frame");
            case 1:
                return QStringLiteral("Language");
            default:
                break;
        }
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CallstackModel::removeRows(int row, int count, const QModelIndex&) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CallstackModel::rowCount(const QModelIndex&) const {
    return m_callstack.entries.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CallstackModel::columnCount(const QModelIndex&) const {
    return 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant CallstackModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return m_callstack.entries[index.row()].desc;
            case 1:
                return m_callstack.entries[index.row()].lang;
        }
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CallstackView::init(QWidget* parent) {
    m_model = new CallstackModel(nullptr);
    m_ui = new Ui_CallstackView;
    m_ui->setupUi(parent);
    m_ui->callstack->setModel(m_model);
    QObject::connect(m_ui->callstack, &QTreeView::doubleClicked, this, &CallstackView::item_double_clicked);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDView* CallstackView::create(QWidget* parent) {
    auto instance = new CallstackView;
    instance->init(parent);
    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CallstackView::~CallstackView() {
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CallstackView::item_double_clicked(const QModelIndex& item) {
    if (m_interface) {
        m_interface->request_frame_index(item.row());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CallstackView::set_backend_interface(PDIBackendRequests* iface) {
    m_interface = iface;

    if (iface) {
        connect(m_interface, &PDIBackendRequests::program_counter_changed, this, &CallstackView::program_counter_changed);
        connect(m_interface, &PDIBackendRequests::reply_callstack, this, &CallstackView::reply_callstack);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CallstackView::reply_callstack(const PDIBackendRequests::Callstack& callstack) {
    m_model->m_callstack = callstack;
    m_model->layoutChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CallstackView::program_counter_changed(const PDIBackendRequests::ProgramCounterChange& pc) {
    m_interface->request_basic(PDIBackendRequests::BasicRequest::Callstack);
}

