#include "Qt5Locals.h"
#include "ProDBGAPI.h"
#include "Qt5DebugSession.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5Locals::Qt5Locals(QWidget* parent) : QTreeWidget(parent)
{
	setColumnCount(4);

	QStringList headers;
	headers << "Name" << "Value" << "Type" << "Address";

	setHeaderLabels(headers);

	g_debugSession->addLocals(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5Locals::~Qt5Locals() 
{
	g_debugSession->delLocals(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5Locals::updateLocals(PDSerializeRead* reader, PDToken token)
{
	QList<QTreeWidgetItem*> items;
	
	int count = reader->readInt(token);

	clear();
	
	for (int i = 0; i < count; ++i) 
	{
		QStringList temp;
		
		const char* name = reader->readString(token);
		const char* value = reader->readString(token);
		const char* type = reader->readString(token);
		const char* address = reader->readString(token);

		temp << name << value << type << address;
		items.append(new QTreeWidgetItem((QTreeWidget*)0, temp)); 
	}

	insertTopLevelItems(0, items);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}

