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

class LineNumberArea;
class BreakpointModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DisassemblyView : public QPlainTextEdit
{
    Q_OBJECT

public:

    DisassemblyView (QWidget* parent = 0);
    virtual ~DisassemblyView();

    int lineNumberAreaWidth();
    void lineNumberAreaPaintEvent(QPaintEvent* event);

private:
    Q_SLOT void updateLineNumberAreaWidth(int newBlockCount);
    Q_SLOT void highlightCurrentLine();
    Q_SLOT void updateLineNumberArea(const QRect& rect, int dy);
    Q_SLOT void programCounterChanged(uint64_t pc);
    Q_SLOT void endDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, int addressWidth);

    QWidget* m_lineNumberArea;
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

}
