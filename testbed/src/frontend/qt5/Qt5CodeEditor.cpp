#include "Qt5CodeEditor.h"
#include "Qt5DebugSession.h"
#include <QtGui>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CodeEditor::Qt5CodeEditor(QWidget* parent) : QPlainTextEdit(parent)
{
	// http://www.qtcentre.org/threads/39941-readonly-QTextEdit-with-visible-Cursor
	//setReadOnly(true);
	//setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect&, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	// TODO: connect(m_debuggerThread, &Qt5DebuggerThread::addBreakpointUI, this, &Qt5CodeEditor::addBreakpoint); 
	// TODO: connect(m_debuggerThread, &Qt5DebuggerThread::setFileLine, this, &Qt5CodeEditor::setFileLine); 

	// TODO: connect(this, &Qt5CodeEditor::tryAddBreakpoint, m_debuggerThread, &Qt5DebuggerThread::tryAddBreakpoint); 
	// TODO: connect(this, &Qt5CodeEditor::tryStartDebugging, m_debuggerThread, &Qt5DebuggerThread::tryStartDebugging); 
	
	//connect(m_debuggerThread, &Qt5DebuggerThread::setCallStack, m_callstack, &Qt5CallStack::updateCallStack); 
	
	//connect(this, &Qt5CodeEditor::tryStep, m_debuggerThread, &Qt5DebuggerThread::tryStep); 

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    QFont font("Courier New", 13);
    font.setStyleHint(QFont::Courier, QFont::NoAntialias);

    setFont(font);

    g_debugSession->addCodeEditor(this);

    m_sourceFile = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CodeEditor::~Qt5CodeEditor()
{
    g_debugSession->delCodeEditor(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update called from the debugging session

void Qt5CodeEditor::sessionUpdate()
{
	const char* filename;
	int line;

	if (g_debugSession->getFilenameLine(&filename, &line))
		setFileLine(filename, line);

	update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // 20 + to give rom for breakpoint marker

    int space = 20 + 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::updateLineNumberAreaWidth(int)
{ 
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) 
    {
        QTextEdit::ExtraSelection selection;
        QTextCursor cursor = textCursor();

        QColor lineColor = QColor(Qt::darkGray).lighter(50);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = cursor;
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    int width = m_lineNumberArea->width();
    int height = fontMetrics().height();

    while (block.isValid() && top <= event->rect().bottom()) 
    {
        if (block.isVisible() && bottom >= event->rect().top()) 
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, width, height, Qt::AlignRight, number);

            if (g_debugSession->hasLineBreakpoint(m_sourceFile, blockNumber))
           		painter.drawArc(0, top + 1, 16, height - 2, 0, 360 * 16);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::step()
{
	g_debugSession->callAction(PDAction_step);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_F8)
	{
        QTextCursor cursor = textCursor();
        int lineNum = cursor.blockNumber();

        // Check if this breakpoint was added directly on the UI thread then update the window to show it 
        
		if (g_debugSession->addBreakpointUI(m_sourceFile, lineNum))
			update();

		return;
	}

	if (event->key() == Qt::Key_F11)
		step();

	QPlainTextEdit::keyPressEvent(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::readSourceFile(const char* filename)
{
	QFile f(filename);

	if (!f.exists())
	{
		printf("Unable to open %s\n", filename);
		return;
	}

	f.open(QFile::ReadOnly | QFile::Text);

	QTextStream ts(&f);
	setPlainText(ts.readAll());

	free((void*)m_sourceFile);
	m_sourceFile = strdup(filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::setFileLine(const char* file, int line)
{
	if (strcmp(file, m_sourceFile))
		readSourceFile(file);

	const QTextBlock& block = document()->findBlockByNumber(line - 1);
	QTextCursor cursor(block);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 0);
	setTextCursor(cursor);
	centerCursor();
	setFocus();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

