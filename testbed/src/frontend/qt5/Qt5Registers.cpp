#include "Qt5Registers.h"
#include "Qt5DebugSession.h"
#include "ProDBGAPI.h"
#include "core/log.h"
#include "core/AssemblyRegister.h"
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QEvent>
#include <QListView>
#include <QTableView>
#include <QSplitter>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5Registers::Qt5Registers(QWidget* parent) : QTreeView(parent)
{
    QStringList horzHeaders;
    horzHeaders << "Register" << "Value"; 

    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(horzHeaders);
    setModel(m_model);

#if defined(_WIN32)
    QFont font("Courier", 11);
#else
    QFont font("Courier", 13);
#endif

    setFont(font);

    g_debugSession->addRegisters(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5Registers::~Qt5Registers()
{
    g_debugSession->delRegisters(this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Don clear this all the time but track changes instead

static void fillList(QStandardItemModel* model, AssemblyRegister* registers, int count) 
{
    for (int i = 0; i < count; ++i)
    {
        QStringList temp;
        QString tempV;

        AssemblyRegister* reg = &registers[i];

        switch (reg->type)
        {
            case AssemblyRegisterType_u8 : temp << tempV.sprintf("0x%x", reg->value.u8); break;
            case AssemblyRegisterType_u16 : temp << tempV.sprintf("0x%04x", reg->value.u16); break;
            case AssemblyRegisterType_u32 : temp << tempV.sprintf("0x%08x", reg->value.u32); break;
            case AssemblyRegisterType_u64 : temp << tempV.sprintf("0x%016llx", reg->value.u64); break;
            case AssemblyRegisterType_float : temp << tempV.sprintf("%8.8f", reg->value.f); break;
            case AssemblyRegisterType_double : temp << tempV.sprintf("%8.8f", reg->value.d); break;
        }

        QList<QStandardItem*> newIt;
        QStandardItem * item = new QStandardItem(reg->name);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        newIt.append(item);

        item = new QStandardItem(tempV);
        //item->setBackground(QBrush(Qt::yellow));

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
        newIt.append(item);
        model->appendRow(newIt);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5Registers::update(AssemblyRegister* registers, int count)
{
    m_model->clear();

    fillList(m_model, registers, count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
void Qt5Registers::keyPressEvent(QKeyEvent* event)
{
    auto m = selectedIndexes();

    if (!m.empty())
    {
        auto t = m[0];
        (void)t;
    }

    QTreeView::keyPressEvent(event);
}

}

