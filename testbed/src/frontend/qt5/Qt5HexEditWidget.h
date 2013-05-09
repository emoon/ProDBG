#pragma once

#include <QtWidgets>

namespace prodbg
{
class Qt5HexEditInternal;
class Qt5HexEditWidget : public QScrollArea
{
	Q_OBJECT;
	Q_PROPERTY(QByteArray data READ getData WRITE setData);
    Q_PROPERTY(int addressOffset READ getAddressOffset WRITE setAddressOffset);
    Q_PROPERTY(QColor addressAreaColor READ getAddressAreaColor WRITE setAddressAreaColor);
    Q_PROPERTY(int cursorPosition READ getCursorPosition WRITE setCursorPosition);
    Q_PROPERTY(QColor highlightingColor READ getHighlightingColor WRITE setHighlightingColor);
    Q_PROPERTY(QColor selectionColor READ getSelectionColor WRITE setSelectionColor);
    Q_PROPERTY(bool overwriteMode READ getOverwriteMode WRITE setOverwriteMode);
    Q_PROPERTY(bool readOnly READ getReadOnly WRITE setReadOnly);
    Q_PROPERTY(QFont font READ getFont WRITE setFont);

public:
	Qt5HexEditWidget(QWidget* parent = nullptr);

	void insert(int index, const QByteArray& values);
    void insert(int index, char value);

    int indexOf(const QByteArray& values, int from = 0) const;
    int lastIndexOf(const QByteArray& values, int from = 0) const;

    void remove(int position, int length = 1);

    void replace(int position, int length, const QByteArray& values);

    QString getReadableString();
    QString getReadableStringFromSelection();

public:
	void setAddressOffset(int offset);
    int getAddressOffset() const;

    void setCursorPosition(int cursorPosition);
    int getCursorPosition() const;

    void setData(const QByteArray& data);
    QByteArray getData() const;

    void setAddressAreaColor(const QColor& color);
    QColor getAddressAreaColor() const;

    void setHighlightingColor(const QColor& color);
    QColor getHighlightingColor() const;

    void setSelectionColor(const QColor& color);
    QColor getSelectionColor() const;

    void setOverwriteMode(bool mode);
    bool getOverwriteMode() const;

    void setReadOnly(bool mode);
    bool getReadOnly() const;

    const QFont& getFont() const;
    void setFont(const QFont& font);

public slots:
    void setAddressWidth(int addressWidth);
    void setAddressArea(bool addressArea);
    void setAsciiArea(bool asciiArea);
    void setHighlighting(bool mode);

    void undo();
    void redo();

signals:
	void currentAddressChanged(int address);
    void currentSizeChanged(int size);
    void overwriteModeChanged(bool state);
    void dataChanged();

private:
    QHBoxLayout* m_layout;
    QScrollArea* m_scrollArea;
    Qt5HexEditInternal* m_internal;
};

}
