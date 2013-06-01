#pragma once

#include <QPlainTextEdit>

namespace prodbg
{

class Qt5DebugOutput : public QPlainTextEdit
{
    Q_OBJECT

public:
    Qt5DebugOutput(QWidget* parent = 0);
    virtual ~Qt5DebugOutput();

	void appendLine(const char* line);
};

}