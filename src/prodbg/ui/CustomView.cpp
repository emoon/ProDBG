#include "CustomView.h"
#include <PDUI.h>
#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QFontMetrics>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PainterData
{
	QPainter* painter;
	int fontWidth;
	int fontHeight;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CustomView::CustomView(QWidget* parent, void* userData, PDCustomDrawCallback callback) : 
	QWidget(parent),
	m_userData(userData),
	m_callback(callback)
{
    QFont f("ProggySquare", 11, QFont::Normal, false);
    f.setStyleHint(QFont::Monospace);
    f.setFixedPitch(true);
    setFont(f);

    QFontMetrics metrics(font());
    m_fontHeight = metrics.height();
    m_fontWidth = metrics.width(QLatin1Char('0'));

    printf("fmetrics %d %d\n", m_fontWidth, m_fontHeight);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void fillRect(void* privateData, PDRect* rect, unsigned int color)
{
	QPainter* painter = ((PainterData*)privateData)->painter;
	QRect qRect(QPoint(rect->x, rect->y), QPoint(rect->width, rect->height));
	painter->fillRect(qRect, color);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setPen(void* privateData, unsigned int color)
{
	QPainter* painter = ((PainterData*)privateData)->painter;
	painter->setPen(QColor(color));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawText(void* privateData, int x, int y, const char* text) 
{
	QPainter* painter = ((PainterData*)privateData)->painter;
	//painter->setBackground(QColor((int)0));
    //painter->setBackgroundMode(Qt::OpaqueMode);
	painter->drawText(QPointF(x, y), QString::fromUtf8(text));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void fontMetrics(void* privateData, int* width, int* height)
{
	PainterData* data = (PainterData*)privateData;
	*width = data->fontWidth;
	*height = data->fontHeight;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CustomView::paintEvent(QPaintEvent* event)
{
	PDUIPainter pdPainter;
	PainterData data;

	QPainter qPainter(this); 
	QRect qRect = event->rect();
	PDRect rect = { qRect.x(), qRect.y(), qRect.width(), qRect.height() };

	data.painter = &qPainter;
	data.fontWidth = m_fontWidth;
	data.fontHeight = m_fontHeight;

	pdPainter.privateData = &data;
	pdPainter.fillRect = fillRect;
	pdPainter.fontMetrics = prodbg::fontMetrics;
	pdPainter.setPen = setPen;
	pdPainter.drawText = drawText;

	m_callback(m_userData, &rect, &pdPainter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
