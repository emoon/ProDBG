#pragma once

#include <ProDBGAPI.h>
#include <QObject>
#include <QPlainTextEdit>
#include <QString>

#include <core/Core.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QThread;
class QFileSystemWatcher;
QT_END_NAMESPACE

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

class Qt5CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    enum Mode
    {
        Sourcefile,  // Sourcefile (.c .s) etc
        Disassembly, // Disassembly
        Mixed,       // Mixed Source + Disassembly mode
    };

    Qt5CodeEditor(QWidget* parent = 0);
    virtual ~Qt5CodeEditor();

    void setMode(Mode mode);
    void setExceptionAddress(uint64_t address);
    void readSourceFile(const char* file);
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void setFileLine(const char* file, int line);
    void setAddress(uint64_t address);
    void setLine(int line);
    int lineNumberAreaWidth();
    void setDisassembly(const char* text, int64_t start, int32_t instructionCount);
    void setAssemblyRegisters(AssemblyRegister* registers, int count);

protected:
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void step();

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect&, int);
    void sessionUpdate();
    void fileChange(const QString filename);

signals:
    void tryAddBreakpoint(const char*, int line);
    void tryStartDebugging(const char* filename);
    void tryStep();

private:
    AssemblyHighlighter* m_assemblyHighlighter;
    QWidget* m_lineNumberArea;
    QFileSystemWatcher* m_fileWatcher;

    const char* m_sourceFile;
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

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(Qt5CodeEditor* editor)
        : QWidget(editor)
        , m_codeEditor(editor)
    {
    }
    QSize sizeHint() const { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* event) { m_codeEditor->lineNumberAreaPaintEvent(event); }

private:
    Qt5CodeEditor* m_codeEditor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
