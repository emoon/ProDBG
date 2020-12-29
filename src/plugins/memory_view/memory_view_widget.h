#pragma once

#include <QtWidgets/QWidget>
#include "backend/backend_requests_interface.h"

class MemoryViewPrivate;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MemoryViewWidget : public QWidget {
    Q_OBJECT;
    using Base = QWidget;

public:
    enum DataType {
        X8,
        U8,
        S8,

        X16,
        U16,
        S16,

        X32,
        U32,
        S32,

        X64,
        U64,
        S64,

        F32,
        F64,
    };

    Q_ENUM(DataType);

    enum Endianess {
        Big,
        Little,
    };

    Q_ENUM(Endianess);

public:
    explicit MemoryViewWidget(QWidget* parent = nullptr);
    virtual ~MemoryViewWidget();

public:
    void set_backend_interface(prodbg::IBackendRequests* interface);
    void set_address(uint64_t address);
    void set_expression_status(bool status);

protected:
    void paintEvent(QPaintEvent* ev) override;
    void contextMenuEvent(QContextMenuEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;

public:
    DataType data_type() const;
    Q_SLOT void set_data_type(DataType t);
    Q_SLOT void end_read_memory(QVector<uint8_t>* target, uint64_t address, int address_width);
    Q_SLOT void program_counter_changed(const prodbg::IBackendRequests::ProgramCounterChange& pc);

    Endianess endianess() const;
    Q_SLOT void set_endianess(Endianess e);

    int elements_per_line() const;
    Q_SLOT void set_elements_per_line(int count);

public:
    Q_SLOT void display_next_page();
    Q_SLOT void display_prev_page();
    Q_SLOT void display_next_line();
    Q_SLOT void display_prev_line();

private:
    MemoryViewPrivate* m_private;
};

