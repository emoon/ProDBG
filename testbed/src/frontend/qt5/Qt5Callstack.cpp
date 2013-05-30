#include "Qt5CallStack.h"
#include "Qt5DebugSession.h"
#include "ProDBGAPI.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStack::Qt5CallStack(QWidget* parent) : QTreeWidget(parent)
{
	setColumnCount(3);

	QStringList headers;
	headers << "Address" << "Module" << "Name";

	setHeaderLabels(headers);

	g_debugSession->addCallStack(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStack::~Qt5CallStack()
{
	g_debugSession->delCallStack(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CallStack::updateCallStack(PDCallStack* callStack, int count)
{
	QList<QTreeWidgetItem*> items;

	clear();

	for (int i = 0; i < count; ++i) 
	{
		char addressString[32];
		QStringList temp;

		const PDCallStack* callStackEntry = &callStack[i];
		sprintf(addressString, "%016llx", callStackEntry->address);
		
		temp << addressString << callStackEntry->moduleName << callStackEntry->fileLine;
		items.append(new QTreeWidgetItem((QTreeWidget*)0, temp)); 
	}

	insertTopLevelItems(0, items);
}

}

