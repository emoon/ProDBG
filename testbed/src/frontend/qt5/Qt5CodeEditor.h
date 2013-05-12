#pragma once

#include <QPlainTextEdit>
#include <QObject>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDDebugPlugin;

namespace prodbg
{

class LineNumberArea;

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

private:

	PDDebugPlugin* m_debuggerPlugin;
	void* m_pluginData;

    QWidget* m_lineNumberArea;
	uint32_t* m_breakpoints;
	uint32_t m_breakpointCount;
	uint32_t m_breakpointCountMax;
	const char* m_sourceFile;
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

