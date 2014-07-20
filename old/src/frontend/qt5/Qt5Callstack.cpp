#include "Qt5CallStack.h"
#include "Qt5DebugSession.h"
#include "ProDBGAPI.h"
#include "core/log.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStack::Qt5CallStack(QWidget* parent) : QTreeWidget(parent)
{
    setColumnCount(3);

    QStringList headers;
    headers << "Address" << "Module" << "Name" << "Line";

    setHeaderLabels(headers);

    g_debugSession->addCallStack(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStack::~Qt5CallStack()
{
    g_debugSession->delCallStack(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5CallStack::update(PDReader* reader)
{
    PDReaderIterator it;

    QList<QTreeWidgetItem*> items;

    clear();

    PDRead_findArray(reader, &it, "callstack", 0);

    if (!it)
    {
        log_info("Unable to find any callstack array\n");
        return;
    }

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* filename = "";
        const char* module = "";
        uint64_t address = 0;
        uint32_t line = 0;

        QStringList temp;

        PDRead_findString(reader, &filename, "filename", it);
        PDRead_findString(reader, &module, "module_name", it);
        PDRead_findU32(reader, &line, "line", it);
        PDRead_findU64(reader, &address, "address", it);

        temp << QString::number(address) << module << filename << QString::number(line);
        items.append(new QTreeWidgetItem((QTreeWidget*)0, temp));
    }

    insertTopLevelItems(0, items);
}

}

