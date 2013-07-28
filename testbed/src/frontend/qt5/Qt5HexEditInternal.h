#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QtWidgets>
#include "Qt5HexEditByteArray.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QKeyEvent;
class QMouseEvent;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5HexEditInternal : public QWidget
{
	Q_OBJECT

public:
    Qt5HexEditInternal(QScrollArea* parent);

    Qt5HexEditByteArray& getHexData();

    void insert(int index, const QByteArray& values);
    void insert(int index, char value);

    int indexOf(const QByteArray& values, int from = 0);
    int lastIndexOf(const QByteArray& values, int from = 0);

    void remove(int index, int length = 1);

    void replace(int index, char value);
    void replace(int index, const QByteArray& values);
    void replace(int position, int length, const QByteArray& values);

    void undo();
    void redo();

    QString getReadableString();
    QString getReadableStringFromSelection();

public:
	void setAddressArea(bool addressArea);
    void setAddressWidth(int addressWidth);
    void setAsciiArea(bool asciiArea);
    void setHighlighting(bool mode);

    void setAddressOffset(int offset);
    int getAddressOffset() const;

    void setCursorPosition(int position);
    int getCursorPosition() const;

    void setData(const QByteArray& data);
    QByteArray getData();

    void setOverwriteMode(bool overwriteMode);
    bool getOverwriteMode() const;

    void setReadOnly(bool readOnly);
    bool getReadOnly() const;

    void setAddressAreaColor(const QColor& color);
    QColor getAddressAreaColor() const;

    void setHighlightingColor(const QColor& color);
    QColor getHighlightingColor() const;

    void setSelectionColor(const QColor& color);
    QColor getSelectionColor() const;

    virtual void setFont(const QFont& font);

signals:
	void currentAddressChanged(int address);
    void currentSizeChanged(int size);
    void dataChanged();
    void overwriteModeChanged(bool state);

protected:
	int calculateCursorPos(QPoint position) const;

    void resetSelection(int position);
    void resetSelection();

    void setSelection(int position);

    int getSelectionBegin() const;
    int getSelectionEnd() const;

protected:
	void keyPressEvent(QKeyEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);

private slots:
	void updateCursor();

private:
    void adjust();
    void ensureVisible();

    void handleCursorMovements(QKeyEvent* event);
	void handleSelectCommands(QKeyEvent* event);
    void handleEditCommands(QKeyEvent* event);

private:
	Qt5HexEditByteArray m_data;
	QTimer m_cursorTimer;
	QColor m_addressAreaColor;
	QColor m_highlightingColor;
	QColor m_selectionColor;
	QScrollArea* m_scrollArea;
	QUndoStack* m_undoStack;

	int m_characterWidth;
	int m_characterHeight;
	int m_cursorX;
	int m_cursorY;
	int m_cursorPosition;
	int m_addressPosition;
	int m_hexPosition;
	int m_asciiPosition;
	int m_selectionBegin;
	int m_selectionEnd;
	int m_selectionAnchor;
	int m_size;

	bool m_blinking      : 1;
	bool m_shouldRender  : 1;
	bool m_addressArea   : 1;
	bool m_asciiArea     : 1;
	bool m_highlighting  : 1;
	bool m_overwriteMode : 1;
	bool m_readOnly      : 1;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
