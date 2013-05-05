#pragma once

#include <QMainWindow>
#include <QObject>

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

namespace prodbg
{

class CodeEditor;

class Qt5MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	Qt5MainWindow();
	void readSourceFile(const char* filename);

private slots:
	void newFile();

private:

	CodeEditor* m_codeEditor;
	QMenu* m_fileMenu;
	QMenu* m_helpMenu;
};

}

