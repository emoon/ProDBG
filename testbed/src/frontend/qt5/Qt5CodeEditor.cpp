#include "Qt5CodeEditor.h"
#include <QtGui>
#include <core/PluginHandler.h>
#include <ProDBGAPI.h>
#include "Qt5DebuggerThread.h"
#include "Qt5CallStack.h"
#include "Qt5Locals.h"
#include "ProDBGAPI.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CodeEditor::Qt5CodeEditor(QWidget* parent) : QPlainTextEdit(parent)
{
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect&, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	m_threadRunner = new QThread;

	m_debuggerThread = new Qt5DebuggerThread();
	m_callstack = new Qt5CallStack(parent);
	m_locals = new Qt5Locals(parent);
	
	m_debuggerThread->moveToThread(m_threadRunner);

	connect(m_threadRunner , SIGNAL(started()), m_debuggerThread, SLOT(start()));
	connect(m_debuggerThread, SIGNAL(finished()), m_threadRunner , SLOT(quit()));
	connect(m_debuggerThread, SIGNAL(callUIthread()), this, SLOT(updateUIThread()));

	connect(m_debuggerThread, &Qt5DebuggerThread::addBreakpointUI, this, &Qt5CodeEditor::addBreakpoint); 
	connect(m_debuggerThread, &Qt5DebuggerThread::setFileLine, this, &Qt5CodeEditor::setFileLine); 

	connect(this, &Qt5CodeEditor::tryAddBreakpoint, m_debuggerThread, &Qt5DebuggerThread::tryAddBreakpoint); 
	connect(this, &Qt5CodeEditor::tryStartDebugging, m_debuggerThread, &Qt5DebuggerThread::tryStartDebugging); 
	
	//connect(m_debuggerThread, &Qt5DebuggerThread::setCallStack, m_callstack, &Qt5CallStack::updateCallStack); 
	
	// TODO: Ok.. a bit ugly to have this here so need to separate this out
	
	m_callstack->resize(320, 200);
	m_callstack->show();
	
	m_locals->resize(320, 200);
	m_locals->show();

	connect(this, &Qt5CodeEditor::tryStep, m_debuggerThread, &Qt5DebuggerThread::tryStep); 

	m_threadRunner->start();

	m_breakpoints = new PDBreakpointFileLine[1024];
	m_breakpointCountMax = 1024;
	m_breakpointCount = 0;
	m_debugState = PDDebugState_default;

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
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

        //printf("%d\n", cursor.blockNumber());
        
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

			for (uint32_t i = 0, count = m_breakpointCount; i < count; ++i)
			{
            	if (m_breakpoints[i].line == blockNumber)
				{
            		painter.drawArc(0, top + 1, 16, height - 2, 0, 360 * 16);
            		break;
				}
			}
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::beginDebug(const char* executable)
{
	printf("beginDebug %s %d\n", executable, (uint32_t)(uint64_t)QThread::currentThreadId());

	emit tryStartDebugging(executable, m_breakpoints, (int)m_breakpointCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::step()
{
	emit tryStep();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::keyPressEvent(QKeyEvent* event)
{
	//int key = event->key();
	//printf("%08x %08x\n", key, Qt::Key_F8);
	if (event->key() == Qt::Key_F8)
	{
        QTextCursor cursor = textCursor();
        int lineNum = cursor.blockNumber();

		for (uint32_t i = 0, count = m_breakpointCount; i < count; ++i)
		{
			if (m_breakpoints[i].line == lineNum)
			{
				m_breakpoints[i] = m_breakpoints[count-1];
				m_breakpointCount--;
				return;
			}
		}

		printf("begin Adding breakpoint\n");

		emit tryAddBreakpoint(m_sourceFile, lineNum);

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

	m_sourceFile = filename;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::updateUIThread()
{
	PDDebugState state;
	void* data;

	state = m_debuggerThread->getDebugState(&data);

	if (state != m_debugState)
	{
		printf("updating status from worker..\n");

		switch (state)
		{
			case PDDebugState_breakpoint : 
			{
				PDDebugStateFileLine* filelineData = (PDDebugStateFileLine*)data;

				printf("Goto line %d\n", filelineData->line);

				const QTextBlock& block = document()->findBlockByNumber(filelineData->line - 1);
				QTextCursor cursor(block);
				cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 0);
				setTextCursor(cursor);
				centerCursor();
				setFocus();
				
				// Update callstack
				
				if (m_callstack)
				{
					PDCallStack stackEntries[128];
					int count = 128;
					
					m_debuggerThread->getCallStack(stackEntries, &count);
					m_callstack->updateCallStack(stackEntries, count);
				}
				
				// Update locals
			
				if (m_locals)
				{
					PDLocals locals[64];
					int count = 64;
					
					m_debuggerThread->getLocals(locals, &count);
					m_locals->updateLocals(locals, count);
				}

				break;
			}
	
			case PDDebugState_default :
			case PDDebugState_noTarget :
			case PDDebugState_breakpointFileLine :
			case PDDebugState_exception :
			case PDDebugState_custom :
				break;
		}

		m_debugState = state;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::setFileLine(const char* file, int line)
{
	// TODO: update filename
	(void)file;

	const QTextBlock& block = document()->findBlockByNumber(line - 1);
	QTextCursor cursor(block);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 0);
	setTextCursor(cursor);
	centerCursor();
	setFocus();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::addBreakpoint(const char* filename, int line, int id)
{
	int breakpoint = m_breakpointCount++;

	m_breakpoints[breakpoint].filename = strdup(filename);
	m_breakpoints[breakpoint].line = line;
	m_breakpoints[breakpoint].id = id;

	printf("Added breakpoint %s %d %d (count %d)\n", filename, line, id, m_breakpointCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

