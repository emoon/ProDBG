#include <QtGui>

#include "Qt5CodeEditor.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent)
{
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect&, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CodeEditor::lineNumberAreaWidth()
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

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{ 
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) 
    {
        QTextEdit::ExtraSelection selection;
        QTextCursor cursor = textCursor();

        printf("%d\n", cursor.blockNumber());
        
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

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
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
            //painter.drawArc(0, top + 1, 16, height - 2, 0, 360 * 16);

        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeEditor::keyPressEvent(QKeyEvent* event)
{
	//int key = event->key();
	//printf("%08x %08x\n", key, Qt::Key_F8);
	if (event->key() == Qt::Key_F8)
	{
		//printf("toggle breakpoint\n");
		return;
	}

	QPlainTextEdit::keyPressEvent(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeEditor::readSourceFile(const char* filename)
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

