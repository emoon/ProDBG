#include "Qt5HexEditByteArray.h"

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditByteArray::Qt5HexEditByteArray()
: m_addressNumbers(4)
, m_addressOffset(0)
, m_oldSize(-99)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditByteArray::getAddressOffset() const
{
    return m_addressOffset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditByteArray::setAddressOffset(int offset)
{
    m_addressOffset = offset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditByteArray::getAddressWidth() const
{
    return m_addressNumbers;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditByteArray::setAddressWidth(int width)
{
    if ((width >= 0) && (width <= 6))
    {
        m_addressNumbers = width;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& Qt5HexEditByteArray::getData()
{
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditByteArray::setData(QByteArray data)
{
    m_data = data;
    m_dataChanged = QByteArray(data.length(), char(0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5HexEditByteArray::getDataChanged(int index) const
{
    return bool(m_dataChanged[index]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray Qt5HexEditByteArray::getDataChanged(int index, int length) const
{
    return m_dataChanged.mid(index, length);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditByteArray::setDataChanged(int index, bool state)
{
    m_dataChanged[index] = char(state);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditByteArray::setDataChanged(int index, const QByteArray& state)
{
    int length = state.length();
    int len;

    if ((index + length) > m_dataChanged.length())
    {
        len = m_dataChanged.length() - index;
    }
    else
    {
        len = length;
    }

    m_dataChanged.replace(index, len, state);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditByteArray::getRealAddressNumbers()
{
    if (m_oldSize != m_data.size())
    {
        QString convert = QString("%1").arg(m_data.size() + m_addressOffset, m_addressNumbers, 16, QChar('0'));
        m_addressRealNumbers = convert.size();
    }

    return m_addressRealNumbers;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5HexEditByteArray::getSize() const
{
    return m_data.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& Qt5HexEditByteArray::insert(int index, char value)
{
    m_data.insert(index, value);
    m_dataChanged.insert(index, char(1));
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& Qt5HexEditByteArray::insert(int index, const QByteArray& values)
{
    m_data.insert(index, values);
    m_dataChanged.insert(index, QByteArray(values.length(), char(1)));
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& Qt5HexEditByteArray::remove(int index, int length)
{
    m_data.remove(index, length);
    m_dataChanged.remove(index, length);
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& Qt5HexEditByteArray::replace(int index, char value)
{
    m_data[index] = value;
    m_dataChanged[index] = char(1);
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& Qt5HexEditByteArray::replace(int index, const QByteArray& values)
{
    int len = values.length();
    return replace(index, len, values);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& Qt5HexEditByteArray::replace(int index, int length, const QByteArray& values)
{
    int len;

    if ((index + length) > m_data.length())
        len = m_data.length() - index;
    else
        len = length;

    m_data.replace(index, len, values.mid(0, len));
    m_dataChanged.replace(index, len, QByteArray(len, char(1)));

    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QChar Qt5HexEditByteArray::getAsciiChar(int index) const
{
    char value = m_data[index];

    if ((value < 0x20) || (value > 0x7e))
    {
        value = '.';
    }

    return QChar(value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QString Qt5HexEditByteArray::getReadableString(int start, int end)
{
    int addressWidth = getRealAddressNumbers();

    if (m_addressNumbers > addressWidth)
    {
        addressWidth = m_addressNumbers;
    }

    if (end < 0)
    {
        end = m_data.size();
    }

    QString result;

    for (int index1 = start; index1 < end; index1 += 16)
    {
        QString addressStr = QString("%1").arg(m_addressOffset + index1, addressWidth, 16, QChar('0'));
        QString hexStr;
        QString ascStr;

        for (int index2 = 0; index2 < 16; ++index2)
        {
            if ((index1 + index2) < m_data.size())
            {
                hexStr.append(" ").append(m_data.mid(index1 + index2, 1).toHex());
                ascStr.append(getAsciiChar(index1 + index2));
            }
        }

        result += addressStr + " " + QString("%1").arg(hexStr, -48) + "  " + QString("%1").arg(ascStr, -17) + "\n";
    }

    return result;
}

}
