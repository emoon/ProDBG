#pragma once

#include <QObject>
#include <QPlainTextEdit>
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

struct AssemblyRegister;
class AssemblyHighlighter;
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

public:
    Q_SIGNAL void tryAddBreakpoint(const char*, int line);
    Q_SIGNAL void tryStartDebugging(const char* filename);
    Q_SIGNAL void tryStep();

private:
    AssemblyHighlighter* m_assemblyHighlighter;
    QWidget* m_lineNumberArea;
    QFileSystemWatcher* m_fileWatcher;

    QString m_sourceFile;
    AssemblyRegister* m_assemblyRegisters;
    Mode m_mode;

    uint64_t m_address;
    uint64_t m_disassemblyStart;
    uint64_t m_disassemblyEnd;

    int m_lineStart;
    int m_lineEnd;
    int m_assemblyRegistersCount;

    // Range of dissassembly (this currently assumes that the disassembly is non-SMC)
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
