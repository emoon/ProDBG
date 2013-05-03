#pragma once

#include <QMainWindow>
#include <QObject>

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

namespace prodbg
{

class Qt5MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	Qt5MainWindow();

private slots:
	void newFile();

private:

	QMenu* m_fileMenu;
	QMenu* m_helpMenu;
};

}

