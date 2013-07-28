#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QUndoCommand>
#include "Qt5HexEditByteArray.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5HexEditValueUndoCommand : public QUndoCommand
{
public:
    enum Operation
    {
    	InsertOperation,
    	RemoveOperation,
    	ReplaceOperation
    };

    Qt5HexEditValueUndoCommand(Qt5HexEditByteArray* data,
    	                       Operation operation,
    	                       int characterPosition,
    	                       char characterNew,
    	                       QUndoCommand* parent = 0);

    void undo();
    void redo();

    bool mergeWith(const QUndoCommand* command);

private:
    Qt5HexEditByteArray* m_data;
    Operation m_operation;
    int  m_characterPosition;
    char m_characterNew;
    char m_characterOld;
    bool m_wasChanged : 1;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5HexEditRangeUndoCommand : public QUndoCommand
{
public:
    enum Operation
    {
    	InsertOperation,
    	RemoveOperation,
    	ReplaceOperation
    };

    Qt5HexEditRangeUndoCommand(Qt5HexEditByteArray* data,
    	                       Operation operation,
    	                       int positionValues,
    	                       const QByteArray& newValues,
    	                       int length = 0,
    	                       QUndoCommand* parent = 0);

    void undo();
    void redo();

private:
    Qt5HexEditByteArray* m_data;
    Operation m_operation;
    int m_positionValues;
    int m_length;
    QByteArray m_changedValues;
    QByteArray m_newValues;
    QByteArray m_oldValues;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
