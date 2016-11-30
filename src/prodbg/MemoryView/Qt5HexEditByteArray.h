#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QtCore>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5HexEditByteArray
{
public:
    explicit Qt5HexEditByteArray();

    int getAddressOffset() const;
    void setAddressOffset(int offset);

    int getAddressWidth() const;
    void setAddressWidth(int width);

    QByteArray& getData();
    void setData(QByteArray data);

    bool getDataChanged(int index) const;
    QByteArray getDataChanged(int index, int length) const;

    void setDataChanged(int index, bool state);
    void setDataChanged(int index, const QByteArray& state);

    int getRealAddressNumbers();
    int getSize() const;

    QByteArray& insert(int index, char value);
    QByteArray& insert(int index, const QByteArray& values);

    QByteArray& remove(int index, int length);

    QByteArray& replace(int index, char value);
    QByteArray& replace(int index, const QByteArray& values);
    QByteArray& replace(int index, int length, const QByteArray& values);

    QChar getAsciiChar(int index) const;
    QString getReadableString(int start = 0, int end = -1);

private:
    QByteArray m_data;
    QByteArray m_dataChanged;

    int m_addressNumbers;
    int m_addressOffset;
    int m_addressRealNumbers;
    int m_oldSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
