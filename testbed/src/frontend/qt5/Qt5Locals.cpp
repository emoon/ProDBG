#include "Qt5Locals.h"
#include "ProDBGAPI.h"
#include "Qt5DebugSession.h"
#include "core/log.h"

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

void Qt5Locals::update(PDReader* reader)
{
    PDReaderIterator it;

    QList<QTreeWidgetItem*> items;

    PDRead_findArray(reader, &it, "locals", 0);

    if (!it)
    {
        log_info("Unable to find locals array from reader\n");
        return;
    }

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* name = "";
        const char* value = "";
        const char* type = "";
        uint64_t address;

        QStringList temp;

        PDRead_findString(reader, &name, "name", it);
        PDRead_findString(reader, &value, "value", it);
        PDRead_findString(reader, &type, "type", it);
        PDRead_findU64(reader, &address, "address", it);

        temp << name << value << type << QString::number(address);
        items.append(new QTreeWidgetItem((QTreeWidget*)0, temp)); 
    }
    
    insertTopLevelItems(0, items);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}

