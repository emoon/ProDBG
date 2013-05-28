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

namespace prodbg
{

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
    Qt5CodeEditor(QWidget* parent = 0);

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
    void setFileLine(const char* file, int line);

signals:
	void tryAddBreakpoint(const char*, int line);
	void tryStartDebugging(const char* filename, PDBreakpointFileLine* breakpoints, int bpCount);
	void tryStep();

private:
    QWidget* m_lineNumberArea;
	const char* m_sourceFile;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(Qt5CodeEditor* editor) : QWidget(editor), m_codeEditor(editor) { }
    QSize sizeHint() const { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* event) { m_codeEditor->lineNumberAreaPaintEvent(event); }

private:
    Qt5CodeEditor* m_codeEditor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

