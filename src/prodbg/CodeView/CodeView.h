#pragma once

#include "Backend/IBackendRequests.h"
#include <QObject>
#include <QPlainTextEdit>
#include <QPointer>
#include <QString>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QThread;
class QFileSystemWatcher;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class LineNumberArea;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FileLineBreakpoint
{
    QString filename;
    int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CodeView : public QPlainTextEdit
{
    Q_OBJECT

public:
    enum Mode
    {
        Sourcefile,  // Sourcefile (.c .s) etc
        Disassembly, // Disassembly
        Mixed,       // Mixed Source + Disassembly mode
    };

    CodeView(QWidget* parent = 0);
    virtual ~CodeView();

    void setBackendInterface(IBackendRequests* iface);
    void setMode(Mode mode);
    void setExceptionAddress(uint64_t address);
    void readSourceFile(const QString& file);
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void setFileLine(const QString& file, int line);
    void setAddress(uint64_t address);
    void setLine(int line);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void step();

private:
    Q_SLOT void updateLineNumberAreaWidth(int newBlockCount);
    Q_SLOT void highlightCurrentLine();
    Q_SLOT void updateLineNumberArea(const QRect&, int);
    Q_SLOT void sessionUpdate();
    Q_SLOT void fileChange(const QString filename);
    Q_SLOT void endDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, int addressWidth);
    Q_SLOT void programCounterChanged(uint64_t pc);

private:
    void toggleDisassembly();
    void toggleSourceFile();
    void updateDisassemblyCursor();

    QPointer<IBackendRequests> m_interface;

    QWidget* m_lineNumberArea;
    QFileSystemWatcher* m_fileWatcher;

    uint32_t m_lineStart = 0;
    uint32_t m_lineEnd = 0;

    QString m_sourceFile;
    QString m_sourceCodeData;
    Mode m_mode = Mode::Sourcefile;

    // Data for disassembly

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
