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

void Qt5CallStack::update(PDSerializeRead* reader, PDToken token)
{
	QList<QTreeWidgetItem*> items;

	clear();

	int count = reader->readInt(token);

	for (int i = 0; i < count; ++i) 
	{
		QStringList temp;

		const char* address = reader->readString(token);
		const char* moduleName = reader->readString(token);
		const char* fileLine = reader->readString(token);

		temp << address << moduleName << fileLine;
		items.append(new QTreeWidgetItem((QTreeWidget*)0, temp)); 
	}

	insertTopLevelItems(0, items);
}

}

