#include "Qt5Registers.h"
#include "Qt5DebugSession.h"
#include "ProDBGAPI.h"
#include "core/log.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

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

Qt5Registers::Qt5Registers(QWidget* parent) : QTreeWidget(parent)
{
	setColumnCount(3);

	QStringList headers;
	headers << "Name" << "Value";

	setHeaderLabels(headers);

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

static void fillList(QList<QTreeWidgetItem*>&items, QVector<Register*>& registers)
{
	for (int i = 0, size = registers.size(); i < size; ++i)
	{
		QStringList temp;
		QString tempV;

		Register* reg = registers[i];

		temp << reg->name;

		switch (reg->type)
		{
			case RegisterType_u8 : temp << tempV.sprintf("0x%x", reg->value.u8); break;
			case RegisterType_u16 : temp << tempV.sprintf("0x%04x", reg->value.u16); break;
			case RegisterType_u32 : temp << tempV.sprintf("0x%08x", reg->value.u32); break;
			case RegisterType_u64 : temp << tempV.sprintf("0x%016llx", reg->value.u64); break;
			case RegisterType_float : temp << tempV.sprintf("%8.8f", reg->value.f); break;
			case RegisterType_double : temp << tempV.sprintf("%8.8f", reg->value.d); break;
		}

		items.append(new QTreeWidgetItem((QTreeWidget*)0, temp)); 
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5Registers::update(PDReader* reader)
{
	QList<QTreeWidgetItem*> items;

	clear();

	buildRegisterList(reader, m_registers);
	fillList(items, m_registers);

	insertTopLevelItems(0, items);
}

}

