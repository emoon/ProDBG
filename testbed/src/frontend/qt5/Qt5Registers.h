#pragma once

#include <QTreeView>
#include <ProDBGAPI.h>

class QStandardItemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

class KeyPressEater : public QObject
 {
     Q_OBJECT
 protected:
     bool eventFilter(QObject *obj, QEvent *event);
 };

struct Register;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5Registers : public QTreeView
{
	Q_OBJECT
public:
	Qt5Registers(QWidget* parent);
	virtual ~Qt5Registers();
	void update(PDReader* reader);
protected:
	virtual void keyPressEvent(QKeyEvent* event);

	QVector<Register*> m_registers;
	QStandardItemModel* m_model;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

