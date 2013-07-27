#include "Qt5CodeEditor.h"
#include "Qt5DebugSession.h"
#include <QtGui>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CodeEditor::Qt5CodeEditor(QWidget* parent) : QPlainTextEdit(parent),
	m_address(0),
	m_disassemblyStart(0),
	m_disassemblyEnd(0),
	m_lineStart(0),
	m_lineEnd(0)
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

    QFont font("Courier", 11);
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

	m_lineStart = blockNumber;

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

	m_lineEnd = blockNumber;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::step()
{
	g_debugSession->callAction(PDAction_step);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::setMode(Mode mode)
{
	m_mode = mode;

	switch (mode)
	{
		case Sourcefile : 
		case Mixed : 
			break;

		case Disassembly : 
		{

			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::setAddress(uint64_t address)
{
	m_address = address;

	// Check if disassembly is with in the range of what we have

	if (address >= m_disassemblyStart && m_address < m_disassemblyEnd)
	{
		// We have the address within the range so now we need to find the current line
		// \todo: We should really just keep a faster way to look this up using a struct
		// with address + text line for each line so we don't need to performe this operation
		// which is just a waste of time

		QString text = toPlainText();
		QStringList textList = text.split("\n");

		for (int i = 0, lineCount = textList.size(); i < lineCount; ++i)
		{
			QByteArray ba = textList[i].toLocal8Bit();
			char* temp = ba.data() + 4;
			uint64_t ta = strtoul(ba.data(), &temp, 16);

			//printf("address 0x%x 0x%x\n", (uint32_t)address, (uint32_t)ta);

			if (ta == address)
			{
				setLine(i + m_lineStart + 1);
				return;
			}
		}
	}
	else
	{
		// Right now we request from only from the PC because we know that that location
		// is valid (from the disassembly point of view) If we had some more contex it would
		// be nice to be able to request code around a PC.

		printf("request address 0x%x\n", (uint32_t)m_address);

		if (m_address < m_disassemblyStart)
			m_disassemblyStart = m_address;

		g_debugSession->requestDisassembly(m_disassemblyStart, (m_lineEnd - m_lineStart));

		m_disassemblyEnd = 0x40;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::setDisassembly(const char* text, int64_t startAddress, int32_t instructionCount)
{
	(void)instructionCount;

	// \todo: We should be a bit smarter than clearing the whole buffer when we set this
	// and keep track on sub regions that we can request instead by keeping track of valid start points
	// the disassembly meroy

	if (startAddress != -1LL)
		m_disassemblyStart = (uint64_t)startAddress; 

	printf("setDisassembly %s\n", text);

	QString string = QString::fromLocal8Bit(text);
	setPlainText(string);
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

void Qt5CodeEditor::setLine(int line)
{
	const QTextBlock& block = document()->findBlockByNumber(line - 1);
	QTextCursor cursor(block);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 0);
	setTextCursor(cursor);
	centerCursor();
	setFocus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CodeEditor::setFileLine(const char* file, int line)
{
	if (strcmp(file, m_sourceFile))
		readSourceFile(file);

	setLine(line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

