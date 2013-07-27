#include "Qt5Registers.h"
#include "Qt5DebugSession.h"
#include "ProDBGAPI.h"
#include "core/log.h"
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QEvent>
#include <QListView>
#include <QTableView>
#include <QSplitter>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

 bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
 {
     if (event->type() == QEvent::KeyPress) {
         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
         qDebug("Ate key press %d", keyEvent->key());
         return true;
     } else {
         // standard event processing
         return QObject::eventFilter(obj, event);
     }
 }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

union RegisterValue
{
	double d;
	uint64_t u64;
	float f;
	uint32_t u32;
	uint16_t u16;
	uint8_t u8;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum ViewMode
{
	ViewMode_Hex,
	ViewMode_Dec,
	ViewMode_Float,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum RegisterType
{
	RegisterType_u8,
	RegisterType_u16,
	RegisterType_u32,
	RegisterType_u64,
	RegisterType_float,
	RegisterType_double
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Register
{
	RegisterValue value;
	char name[64];
	int count;					// number of "internal" registers (4 x u32 for SSE for example)
	RegisterType type;		// 
	ViewMode viewMode;
	uint8_t readOnly;
	uint8_t statusFlags;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5Registers::Qt5Registers(QWidget* parent) : QTreeView(parent)
{
	QStringList horzHeaders;
    horzHeaders << "Register" << "Value"; 

	m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(horzHeaders);
	setModel(m_model);

	// temp fill

	Register* reg = new Register;
	memset(reg, 0, sizeof(Register));
	strcpy(reg->name, "d0"); 
	m_registers.push_back(reg);
	reg->type = RegisterType_u32;
	reg->value.u32 = 0x120;

	reg = new Register;
	memset(reg, 0, sizeof(Register));
	strcpy(reg->name, "d1"); 
	reg->type = RegisterType_u32;
	m_registers.push_back(reg);
	reg->value.u32 = 0x12001401;

	reg = new Register;
	memset(reg, 0, sizeof(Register));
	strcpy(reg->name, "fp0"); 
	reg->type = RegisterType_float;
	m_registers.push_back(reg);
	reg->value.f = 1.043f;

	update(0);

	//installEventFilter(new KeyPressEater());
	g_debugSession->addRegisters(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5Registers::~Qt5Registers()
{
	g_debugSession->delRegisters(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void buildRegisterList(PDReader* reader, QVector<Register*>& registers)
{
	PDReaderIterator it;

	PDRead_findArray(reader, &it, "registers", 0);

	if (!it)
	{
		log_info("Unable to find any registers array\n");
		return;
	}

	while (PDRead_getNextEntry(reader, &it))
	{
		Register* reg = 0;

		uint64_t regValue;
		const char* name = "";

		QStringList temp;

		PDRead_findString(reader, &name, "name", it);
		int type = PDRead_findU64(reader, &regValue, "register", it);

		// find entry or insert

		for (int i = 0, size = registers.size(); i < size; ++i)
		{
			if (!strcmp(name, registers[i]->name))
			{
				reg = registers[i];
				break;
			}
		}

		// insert the reg if we couldn't find it

		if (!reg)
		{
			reg = new Register;
			memset(reg, 0, sizeof(Register));
			strcpy(reg->name, name); 
			registers.push_back(reg);

			switch (type & PDReadStatus_typeMask)
			{
				case PDReadType_u8 : reg->type = RegisterType_u8; break;
				case PDReadType_u16 : reg->type = RegisterType_u16; break;
				case PDReadType_u32 : reg->type = RegisterType_u32; break;
				case PDReadType_u64 : reg->type = RegisterType_u64; break;
				case PDReadType_float : reg->type = RegisterType_float; break;
				case PDReadType_double : reg->type = RegisterType_double; break;
			}

			PDRead_findU8(reader, &reg->readOnly, "read_only", it);
			PDRead_findU8(reader, &reg->statusFlags, "flags", it);
		}

		// TODO: Handle if we have registers that are wider than 64-bit

		reg->value.u64 = regValue;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Don clear this all the time but track changes instead

static void fillList(QStandardItemModel* model, QVector<Register*>& registers)
{
	for (int i = 0, size = registers.size(); i < size; ++i)
	{
		QStringList temp;
		QString tempV;

		Register* reg = registers[i];

		switch (reg->type)
		{
			case RegisterType_u8 : temp << tempV.sprintf("0x%x", reg->value.u8); break;
			case RegisterType_u16 : temp << tempV.sprintf("0x%04x", reg->value.u16); break;
			case RegisterType_u32 : temp << tempV.sprintf("0x%08x", reg->value.u32); break;
			case RegisterType_u64 : temp << tempV.sprintf("0x%016llx", reg->value.u64); break;
			case RegisterType_float : temp << tempV.sprintf("%8.8f", reg->value.f); break;
			case RegisterType_double : temp << tempV.sprintf("%8.8f", reg->value.d); break;
		}

		QList<QStandardItem*> newIt;
		QStandardItem * item = new QStandardItem(reg->name);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		newIt.append(item);

		item = new QStandardItem(tempV);
		item->setBackground(QBrush(Qt::yellow));

		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
		newIt.append(item);
		model->appendRow(newIt);
	}

	/*
 QTreeView *tree = new QTreeView();
  tree->setModel( model );
  tree->show();
  */

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5Registers::update(PDReader* reader)
{
	if (reader)
		buildRegisterList(reader, m_registers);

	fillList(m_model, m_registers);
 /* 
	QTreeView *tree = new QTreeView();
  QListView *list = new QListView();
  QTableView *table = new QTableView();
  
  QSplitter* splitter = new QSplitter();
  splitter->addWidget( tree );
  splitter->addWidget( list );
  splitter->addWidget( table );
  
  QStandardItemModel* model = new QStandardItemModel( 5, 2 );
  for( int r=0; r<5; r++ ) 
    for( int c=0; c<2; c++) 
    {
      QStandardItem *item = new QStandardItem( QString("Row:%0, Column:%1").arg(r).arg(c) );
      
      if( c == 0 )
        for( int i=0; i<3; i++ )
        {
          QStandardItem *child = new QStandardItem( QString("Item %0").arg(i) );
          child->setEditable( false );
          item->appendRow( child );
        }
      
      model->setItem(r, c, item);
    }

  model->setHorizontalHeaderItem( 0, new QStandardItem( "Foo" ) );
  model->setHorizontalHeaderItem( 1, new QStandardItem( "Bar-Baz" ) );

  tree->setModel( model );
  list->setModel( model );
  table->setModel( model );

  list->setSelectionModel( selectionModel() );
  table->setSelectionModel( selectionModel() );

  table->setSelectionBehavior( QAbstractItemView::SelectRows );
  table->setSelectionMode( QAbstractItemView::SingleSelection );

  splitter->show();
*/
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

