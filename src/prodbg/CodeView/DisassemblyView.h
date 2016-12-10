#pragma once

#include "Backend/IBackendRequests.h"
#include <QPlainTextEdit>
#include <QPointer>
#include <QString>
#include <QPointer>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QThread;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class AddressArea;
class BreakpointModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DisassemblyView : public QPlainTextEdit
{
    Q_OBJECT

public:

    DisassemblyView(QWidget* parent = 0);
    virtual ~DisassemblyView();

    void updatePc(uint64_t pc);

    void toggleBreakpoint();
    int lineNumberAreaWidth();
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void setBackendInterface(IBackendRequests* interface);
    void setBreakpointModel(BreakpointModel* breakpoints);

    Q_SLOT void endDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, int addressWidth);

private:
    Q_SLOT void updateAddressAreaWidth(int newBlockCount);
    Q_SLOT void highlightCurrentLine();
    Q_SLOT void updateAddressArea(const QRect& rect, int dy);

    void resizeEvent(QResizeEvent* event);
    void updateDisassemblyCursor();
    void setLine(int line);

    QWidget* m_addressArea;
    QPointer<IBackendRequests> m_interface;
    BreakpointModel* m_breakpoints;

    struct AddressData
    {
        uint64_t address;
        QString addressText;
    };

    uint64_t m_disassemblyStart = 0;
    uint64_t m_disassemblyEnd = 0;
    uint64_t m_currentPc = 0;
    int m_addressWidth = 0;

    QString m_disassemblyText;

    // Currenty range of disassembly addresses
    QVector<AddressData> m_disassemblyAdresses;

    // Temp vector for recving data from backend
    QVector<IBackendRequests::AssemblyInstruction> m_recvInstructions;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void DisassemblyView::setBreakpointModel(BreakpointModel* breakpoints)
{
    m_breakpoints = breakpoints;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
