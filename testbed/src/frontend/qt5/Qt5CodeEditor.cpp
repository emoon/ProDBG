#include "Qt5CodeEditor.h"
#include "Qt5DebugSession.h"
#include "core/AssemblyRegister.h"
#include <QtGui>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

class AssemblyHighlighter : public QSyntaxHighlighter
{   

  public:    
	AssemblyHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) 
	{
		m_matchRegisters.clear();
	};

	~AssemblyHighlighter() {};

	void clearRegisterList()
	{
		m_matchRegisters.clear();
	}
	
	void highlightBlock(const QString& text)
	{
		int matchRegCount = m_matchRegisters.count();

		for (int i = 0; i < text.length(); ++i) 
		{
			for (int p = 0; p < matchRegCount; ++p)
			{
				const RegInfo& info = m_matchRegisters[p];

				// \todo Don't use mid. It returns a new QString so we will do malloc/free for each call

				if (text.mid(i, info.length) == info.name)
					setFormat(i, info.length, info.color);
			}
		}  
	}

	void addRegister(QString registerName)
	{
		RegInfo info;

		static QColor colors[] =
		{
			Qt::red,
			Qt::magenta,
			Qt::blue,
		};

		info.length = registerName.length();
		info.color = colors[m_matchRegisters.count() % 3];
		info.name = registerName;
		m_matchRegisters.push_back(info);
	}

  private:
  	
  	struct RegInfo
	{
		int length;
		QColor color;
		QString name;
	};

  	QVector<RegInfo> m_matchRegisters;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CodeEditor::Qt5CodeEditor(QWidget* parent) : QPlainTextEdit(parent),
	m_assemblyHighlighter(0),
	m_address(0),
	m_disassemblyStart(0),
	m_disassemblyEnd(0),
	m_lineStart(0),
	m_lineEnd(0)
{
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect&, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

#if defined(_WIN32)
    QFont font("Courier", 11);
#else
    QFont font("Courier", 13);
#endif

    setFont(font);

    g_debugSession->addCodeEditor(this);

    m_assemblyHighlighter = new AssemblyHighlighter(document());

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

	QTextEdit::ExtraSelection selection;
	QTextCursor cursor = textCursor();

	QString	text = cursor.selectedText();

	QString string = cursor.block().text();

	if (m_assemblyHighlighter)
	{
		m_assemblyHighlighter->clearRegisterList();

		bool addedReg = false;
		int registerCount = m_assemblyRegistersCount;

		QByteArray ba = string.toLatin1();
  		const char* lineText = ba.data();

		// Update the syntax highlighter if we should highlight assembly

		for (int i = 0, length = string.length() - 1; i < length; ++i)
		{
			for (int p = 0; p < registerCount; ++p)
			{
				const AssemblyRegister* reg = &m_assemblyRegisters[p];

				if (!strncmp(&lineText[i], reg->name, reg->nameLength))
				{
					m_assemblyHighlighter->addRegister(string.mid(i, 2));
					addedReg = true;
				}
			}
		}

		if (addedReg)
			m_assemblyHighlighter->rehighlight();
	}

	QColor lineColor = QColor(Qt::lightGray).lighter(100);

	selection.format.setBackground(lineColor);
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	selection.cursor = cursor;
	selection.cursor.clearSelection();
	extraSelections.append(selection);

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

void Qt5CodeEditor::setAssemblyRegisters(AssemblyRegister* registers, int count)
{
	m_assemblyRegisters = registers;
	m_assemblyRegistersCount = count;
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

