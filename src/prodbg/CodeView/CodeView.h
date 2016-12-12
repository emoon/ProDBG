#pragma once

#include "Backend/IBackendRequests.h"
#include <QObject>
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
class QFileSystemWatcher;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class LineNumberArea;
class BreakpointModel;

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

    void toggleBreakpoint();
    void setBreakpointModel(BreakpointModel* breakpoints);
    void openFile();
    void reload();

    void setMode(Mode mode);
    void setExceptionAddress(uint64_t address);

    void initDefaultSourceFile(const QString& file);
    void readSourceFile(const QString& file);

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void setFileLine(const QString& file, int line);
    void setAddress(uint64_t address);
    void setLine(int line);
    void setExceptionLine(int line);
    void sessionEnded();

    int lineNumberAreaWidth();

    int getCurrentLine();

protected:
    void resizeEvent(QResizeEvent* event);
    //void keyPressEvent(QKeyEvent* event);
    void step();

private:
    Q_SLOT void updateLineNumberAreaWidth(int newBlockCount);
    Q_SLOT void highlightCurrentLine();
    Q_SLOT void updateLineNumberArea(const QRect&, int);
    Q_SLOT void fileChange(const QString filename);

private:
    void toggleDisassembly();
    void toggleSourceFile();
    void updateDisassemblyCursor();

    void readSettings();
    void writeSettings();

    QPointer<IBackendRequests> m_interface;
    BreakpointModel* m_breakpoints = 0;

    QWidget* m_lineNumberArea;
    QFileSystemWatcher* m_fileWatcher;

    QString m_sourceFile;
    QString m_sourceCodeData;
    Mode m_mode = Mode::Sourcefile;

    int m_currentSourceLine = 0;

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
