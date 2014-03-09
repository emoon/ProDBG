#include "Qt5HexEditCommands.h"

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditValueUndoCommand::Qt5HexEditValueUndoCommand(Qt5HexEditByteArray* data,
                                                       Operation operation,
                                                       int characterPosition,
                                                       char characterNew,
                                                       QUndoCommand* parent)
: QUndoCommand(parent)
, m_data(data)
, m_operation(operation)
, m_characterPosition(characterPosition)
, m_characterNew(characterNew)
, m_characterOld(0)
, m_wasChanged(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditValueUndoCommand::undo()
{
    switch (m_operation)
    {
        case InsertOperation:
            m_data->remove(m_characterPosition, 1);
            break;

        case ReplaceOperation:
            m_data->replace(m_characterPosition, m_characterOld);
            m_data->setDataChanged(m_characterPosition, m_wasChanged);
            break;

        case RemoveOperation:
            m_data->insert(m_characterPosition, m_characterOld);
            m_data->setDataChanged(m_characterPosition, m_wasChanged);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditValueUndoCommand::redo()
{
    switch (m_operation)
    {
        case InsertOperation:
            m_data->insert(m_characterPosition, m_characterNew);
            break;

        case ReplaceOperation:
            m_characterOld = m_data->getData()[m_characterPosition];
            m_wasChanged = m_data->getDataChanged(m_characterPosition);
            m_data->replace(m_characterPosition, m_characterNew);
            break;

        case RemoveOperation:
            m_characterOld = m_data->getData()[m_characterPosition];
            m_wasChanged = m_data->getDataChanged(m_characterPosition);
            m_data->remove(m_characterPosition, 1);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5HexEditValueUndoCommand::mergeWith(const QUndoCommand* command)
{
    bool result = false;

    if (m_operation != RemoveOperation)
    {
        const Qt5HexEditValueUndoCommand* nextCommand = static_cast<const Qt5HexEditValueUndoCommand*>(command);
        
        if (nextCommand->m_operation == ReplaceOperation)
        {
            if (nextCommand->m_characterPosition == m_characterPosition)
            {
                m_characterNew = nextCommand->m_characterNew;
                result = true;
            }
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditRangeUndoCommand::Qt5HexEditRangeUndoCommand(Qt5HexEditByteArray* data,
                                                       Operation operation,
                                                       int positionValues,
                                                       const QByteArray& newValues,
                                                       int length,
                                                       QUndoCommand* parent)
: QUndoCommand(parent)
, m_data(data)
, m_operation(operation)
, m_positionValues(positionValues)
, m_length(length)
, m_newValues(newValues)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditRangeUndoCommand::undo()
{
    switch (m_operation)
    {
        case InsertOperation:
            m_data->remove(m_positionValues, m_newValues.length());
            break;

        case ReplaceOperation:
            m_data->replace(m_positionValues, m_oldValues);
            m_data->setDataChanged(m_positionValues, m_changedValues);
            break;

        case RemoveOperation:
            m_data->insert(m_positionValues, m_oldValues);
            m_data->setDataChanged(m_positionValues, m_changedValues);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditRangeUndoCommand::redo()
{
    switch (m_operation)
    {
        case InsertOperation:
            m_data->insert(m_positionValues, m_newValues);
            break;

        case ReplaceOperation:
            m_oldValues = m_data->getData().mid(m_positionValues, m_length);
            m_changedValues = m_data->getDataChanged(m_positionValues, m_length);
            m_data->replace(m_positionValues, m_newValues);
            break;

        case RemoveOperation:
            m_oldValues = m_data->getData().mid(m_positionValues, m_length);
            m_changedValues = m_data->getDataChanged(m_positionValues, m_length);
            m_data->remove(m_positionValues, m_length);
            break;
    }
}

}
