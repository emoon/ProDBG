#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QUndoCommand>
#include "MemoryViewByteArray.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MemoryViewValueUndoCommand : public QUndoCommand
{
public:
    enum Operation
    {
        InsertOperation,
        RemoveOperation,
        ReplaceOperation
    };

    MemoryViewValueUndoCommand(MemoryViewByteArray* data,
                               Operation            operation,
                               int                  characterPosition,
                               char                 characterNew,
                               QUndoCommand*        parent = 0);

    void undo();
    void redo();

    bool mergeWith(const QUndoCommand* command);

private:
    MemoryViewByteArray* m_data;
    Operation m_operation;
    int m_characterPosition;
    char m_characterNew;
    char m_characterOld;
    bool m_wasChanged : 1;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MemoryViewRangeUndoCommand : public QUndoCommand
{
public:
    enum Operation
    {
        InsertOperation,
        RemoveOperation,
        ReplaceOperation
    };

    MemoryViewRangeUndoCommand(MemoryViewByteArray* data,
                               Operation            operation,
                               int                  positionValues,
                               const QByteArray&    newValues,
                               int                  length = 0,
                               QUndoCommand*        parent = 0);

    void undo();
    void redo();

private:
    MemoryViewByteArray* m_data;
    Operation m_operation;
    int m_positionValues;
    int m_length;
    QByteArray m_changedValues;
    QByteArray m_newValues;
    QByteArray m_oldValues;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
