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

void Qt5Locals::update(PDSerializeRead* reader)
{
	QList<QTreeWidgetItem*> items;

	int count = PDREAD_INT(reader);

	clear();
	
	for (int i = 0; i < count; ++i) 
	{
		QStringList temp;
		
		const char* name = PDREAD_STRING(reader);
		const char* value = PDREAD_STRING(reader);
		const char* type = PDREAD_STRING(reader);
		const char* address = PDREAD_STRING(reader);

		temp << name << value << type << address;
		items.append(new QTreeWidgetItem((QTreeWidget*)0, temp)); 
	}

	insertTopLevelItems(0, items);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}

