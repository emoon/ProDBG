#pragma once

#include <QPlainTextEdit>

struct PDDebugOutput;

namespace prodbg
{

class Qt5DebugOutput : public QPlainTextEdit
{
    Q_OBJECT

public:
    Qt5DebugOutput(QWidget* parent = 0);
    virtual ~Qt5DebugOutput();

	void updateDebugOutput(PDDebugOutput* debugOutput);
};

}