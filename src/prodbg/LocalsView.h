#pragma once

#include <QtWidgets/QWidget>
#include "Backend/BackendRequests.h"
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QAbstractItemModel>

class Ui_LocalsView;
class QFileSystemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class IBackendRequests;
class LocalsModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalsView : public QWidget {
    Q_OBJECT
public:
    void set_backend_interface(IBackendRequests* iface);

    explicit LocalsView(QWidget* parent);
    virtual ~LocalsView();

private:

    LocalsModel* m_model = nullptr;
    IBackendRequests* m_interface = nullptr;
    Ui_LocalsView* m_ui = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

