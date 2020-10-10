#pragma once

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "api/include/pd_ui.h"
#include "backend/backend_requests_interface.h"

class Ui_MemoryView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MemoryView : public QObject, prodbg::PDUIInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PDUIInterface_iid FILE "memory_view.json")
    Q_INTERFACES(prodbg::PDUIInterface)

public:
    prodbg::PDUIInterface* create(QWidget* parent);
    void set_backend_interface(prodbg::IBackendRequests* interface);
    ~MemoryView();

private:
    Q_SLOT void jump_to_address_expression(const QString& expression);
    Q_SLOT void end_resolve_address(uint64_t* out);
    Q_SLOT void jump_address_changed();
    Q_SLOT void endian_changed(int);
    Q_SLOT void data_type_changed(int);
    Q_SLOT void count_changed(const QString&);

    void init(QWidget* parent);
    QPointer<prodbg::IBackendRequests> m_backend;
    //static View* createView(QWidget* parent);

    Ui_MemoryView* m_ui = nullptr;
    uint64_t m_eval_address = 0;
};

