#include "Qt5CallStack.h"
#include <QStandardItemModel>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStack::Qt5CallStack()
{
	setColumnCount(3);

	QStringList headers;
	headers << "Address" << "Module" << "Name";

	setHeaderLabels(headers);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CallStack::updateCallstack(QStringList strings)
{
	QList<QTreeWidgetItem*> items;

	clear();

	// don't really like this approach but...

	for (int i = 0; i < strings.size(); i += 3)
	{
		QStringList temp;
		temp << strings.at(i + 0) << strings.at(i + 1) << strings.at(i + 2);
		items.append(new QTreeWidgetItem((QTreeWidget*)0, temp)); 
	}

	insertTopLevelItems(0, items);
}

}

