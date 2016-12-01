#include "MemoryViewByteArray.h"

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryViewByteArray::MemoryViewByteArray()
    : m_addressNumbers(4)
    , m_addressOffset(0)
    , m_oldSize(-99)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MemoryViewByteArray::getAddressOffset() const
{
    return m_addressOffset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewByteArray::setAddressOffset(int offset)
{
    m_addressOffset = offset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MemoryViewByteArray::getAddressWidth() const
{
    return m_addressNumbers;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewByteArray::setAddressWidth(int width)
{
    if ((width >= 0) && (width <= 6))
    {
        m_addressNumbers = width;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& MemoryViewByteArray::getData()
{
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewByteArray::setData(QByteArray data)
{
    m_data = data;
    m_dataChanged = QByteArray(data.length(), char(0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MemoryViewByteArray::getDataChanged(int index) const
{
    return bool(m_dataChanged[index]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray MemoryViewByteArray::getDataChanged(int index, int length) const
{
    return m_dataChanged.mid(index, length);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewByteArray::setDataChanged(int index, bool state)
{
    m_dataChanged[index] = char(state);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryViewByteArray::setDataChanged(int index, const QByteArray& state)
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

int MemoryViewByteArray::getRealAddressNumbers()
{
    if (m_oldSize != m_data.size())
    {
        QString convert = QString("%1").arg(m_data.size() + m_addressOffset, m_addressNumbers, 16, QChar('0'));
        m_addressRealNumbers = convert.size();
    }

    return m_addressRealNumbers;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MemoryViewByteArray::getSize() const
{
    return m_data.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& MemoryViewByteArray::insert(int index, char value)
{
    m_data.insert(index, value);
    m_dataChanged.insert(index, char(1));
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& MemoryViewByteArray::insert(int index, const QByteArray& values)
{
    m_data.insert(index, values);
    m_dataChanged.insert(index, QByteArray(values.length(), char(1)));
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& MemoryViewByteArray::remove(int index, int length)
{
    m_data.remove(index, length);
    m_dataChanged.remove(index, length);
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& MemoryViewByteArray::replace(int index, char value)
{
    m_data[index] = value;
    m_dataChanged[index] = char(1);
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& MemoryViewByteArray::replace(int index, const QByteArray& values)
{
    int len = values.length();
    return replace(index, len, values);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray& MemoryViewByteArray::replace(int index, int length, const QByteArray& values)
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

QChar MemoryViewByteArray::getAsciiChar(int index) const
{
    char value = m_data[index];

    if ((value < 0x20) || (value > 0x7e))
    {
        value = '.';
    }

    return QChar(value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QString MemoryViewByteArray::getReadableString(int start, int end)
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
