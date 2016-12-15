#pragma once

#include <QtWidgets/QWidget>
#include "Backend/IBackendRequests.h"

namespace prodbg {

class MemoryViewPrivate;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MemoryViewWidget : public QWidget
{
    Q_OBJECT;
    using Base = QWidget;

public:
    enum DataType
    {
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

    enum Endianess
    {
        Big,
        Little,
    };

    Q_ENUM(Endianess);

public:
    explicit MemoryViewWidget(QWidget* parent = nullptr);
    virtual ~MemoryViewWidget();

public:
    void setBackendInterface(IBackendRequests* interface);
    void setAddress(uint64_t address);
    void setExpressionStatus(bool status);

protected:
    void paintEvent(QPaintEvent* ev) override;
    void contextMenuEvent(QContextMenuEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;

public:
    DataType dataType() const;
    Q_SLOT void setDataType(DataType t);
    Q_SLOT void endReadMemory(QVector<uint16_t>* target, uint64_t address, int addressWidth);
    Q_SLOT void programCounterChanged(const IBackendRequests::ProgramCounterChange& pc);

    Endianess endianess() const;
    Q_SLOT void setEndianess(Endianess e);

    int elementsPerLine() const;
    Q_SLOT void setElementsPerLine(int count);

public:
    Q_SLOT void displayNextPage();
    Q_SLOT void displayPrevPage();
    Q_SLOT void displayNextLine();
    Q_SLOT void displayPrevLine();

private:
    MemoryViewPrivate* m_Private;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
