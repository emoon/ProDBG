#pragma once

#include <QPlainTextEdit>
#include <QObject>
#include <QString>
#include <ProDBGAPI.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QThread;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDDebugPlugin;

namespace prodbg
{

class LineNumberArea;
class Qt5DebuggerThread;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FileLineBreakpoint
{
	QString filename;
	int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget* parent = 0);

	void beginDebug(const char* executable);
	void readSourceFile(const char* file);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent* event);
	void keyPressEvent(QKeyEvent* event);
	void step();

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void updateUIThread();
    void addBreakpoint(const char* filename, int line, int id);
    void setFileLine(const char* file, int line);

signals:
	void tryAddBreakpoint(const char*, int line);
	void tryStartDebugging(const char* filename, PDBreakpointFileLine* breakpoints, int bpCount);
	void tryStep();

private:

	PDDebugPlugin* m_debuggerPlugin;
	void* m_pluginData;

    QWidget* m_lineNumberArea;
	
	PDBreakpointFileLine* m_breakpoints;
	uint32_t m_breakpointCount;
	uint32_t m_breakpointCountMax;
	const char* m_sourceFile;
	PDDebugState m_debugState;
	Qt5DebuggerThread* m_debuggerThread;
	QThread* m_threadRunner;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor* editor) : QWidget(editor), m_codeEditor(editor) { }
    QSize sizeHint() const { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* event) { m_codeEditor->lineNumberAreaPaintEvent(event); }

private:
    CodeEditor* m_codeEditor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

