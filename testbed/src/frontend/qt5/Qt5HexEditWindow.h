#pragma once

#include <QWindow>
#include <QObject>
#include "Qt5HexEditWidget.h"

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
class QUndoStack;
QT_END_NAMESPACE

namespace prodbg
{

class Qt5HexEditWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit Qt5HexEditWindow();//QWindow* parent = NULL);
	~Qt5HexEditWindow();

protected:
	//void closeEvent(QCloseEvent* event);

private slots:
	//onClose();

private:
	void initialize();
	void initializeStatusBar();

	void testEditor();

private:
	Qt5HexEditWidget* m_hexEdit;


	QLabel* m_addressLabel;
	QLabel* m_addressNameLabel;
    QLabel* m_overwriteModeLabel;
    QLabel* m_overwriteModeNameLabel;
    QLabel* m_sizeLabel;
    QLabel* m_sizeNameLabel;

};

}
