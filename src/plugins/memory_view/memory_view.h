#pragma once

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "api/include/pd_memory_view.h"
#include "backend/backend_requests_interface.h"

class Ui_HexView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class HexView : public prodbg::MemoryView
{
    Q_OBJECT

public:
    prodbg::MemoryView* create(QWidget* parent);
    void set_backend_interface(prodbg::IBackendRequests* interface);
    ~HexView();

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

    Ui_HexView* m_ui = nullptr;
    uint64_t m_eval_address = 0;
};

