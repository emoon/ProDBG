#pragma once

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include "api/include/pd_memory_view.h"
#include "backend/backend_requests_interface.h"

class Ui_MemoryView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class HexView : public PDMemoryView
{
    Q_OBJECT

public:
    PDMemoryView* create(QWidget* parent);
    void set_backend_interface(PDIBackendRequests* interface);
    ~HexView();

private:
    PDMemoryView::Ver version() { return PDMemoryView::Ver::Version; }
    const char* name() { return "Hex"; }

    Q_SLOT void jump_to_address_expression(const QString& expression);
    Q_SLOT void end_resolve_address(uint64_t* out);
    Q_SLOT void jump_address_changed();
    Q_SLOT void endian_changed(int);
    Q_SLOT void data_type_changed(int);
    Q_SLOT void count_changed(int index);

    void init(QWidget* parent);
    QPointer<PDIBackendRequests> m_backend;

    Ui_MemoryView* m_ui = nullptr;
    uint64_t m_eval_address = 0;
};

