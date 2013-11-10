#include "AssemblyRegister.h"
#include "core/log.h"
#include "ProDBGAPI.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

enum
{
	MaxRegisterCount = 256,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AssemblyRegister* AssemblyRegister_buildFromReader(PDReader* reader, AssemblyRegister* registers, int* countIn)
{
	PDReaderIterator it;
	int count = *countIn;

	PDRead_findArray(reader, &it, "registers", 0);

	if (!it)
	{
		log_info("Unable to find any registers array\n");
		*countIn = 0;
		return 0;
	}

	if (!registers)
	{
		registers = (AssemblyRegister*)malloc(sizeof(AssemblyRegister) * MaxRegisterCount);
		memset(registers, 0, sizeof(AssemblyRegister) * MaxRegisterCount);
	}

	while (PDRead_getNextEntry(reader, &it))
	{
		AssemblyRegister* reg = 0;

		uint64_t regValue;
		const char* name = "";

		PDRead_findString(reader, &name, "name", it);
		uint32_t type = PDRead_findU64(reader, &regValue, "register", it);

		// find entry or insert

		for (int i = 0; i < count; ++i)
		{
			if (!strcmp(name, registers[i].name))
			{
				reg = &registers[i];
				break;
			}
		}

		// insert the reg if we couldn't find it

		if (!reg)
		{
			if (count >= MaxRegisterCount)
			{
				log_error("More than %d registers! Unable to handle this without bumping the limit\n", count);
				*countIn = count;
				return registers;
			}

			reg = &registers[count++];
			strcpy(reg->name, name); 
			reg->nameLength = (int)strlen(name);

			switch (type & PDReadStatus_typeMask)
			{
				case PDReadType_u8 : reg->type = AssemblyRegisterType_u8; break;
				case PDReadType_u16 : reg->type = AssemblyRegisterType_u16; break;
				case PDReadType_u32 : reg->type = AssemblyRegisterType_u32; break;
				case PDReadType_u64 : reg->type = AssemblyRegisterType_u64; break;
				case PDReadType_float : 
				{
					PDRead_findFloat(reader, &reg->value.f, "register", it);
					reg->type = AssemblyRegisterType_float; 
					break;
				}
				
				case PDReadType_double : 
				{
					PDRead_findDouble(reader, &reg->value.d, "register", it);
					reg->type = AssemblyRegisterType_double; 
					break;
				}
			}
		}

		reg->value.u64 = regValue;

		switch (type & PDReadStatus_typeMask)
		{
			case PDReadType_float : 
			{
				PDRead_findFloat(reader, &reg->value.f, "register", it);
				break;
			}
			
			case PDReadType_double : 
			{
				PDRead_findDouble(reader, &reg->value.d, "register", it);
				break;
			}
		}

		PDRead_findU16(reader, &reg->readOnly, "read_only", it);
		PDRead_findU16(reader, &reg->statusFlags, "flags", it);
	}

	*countIn = count;
	return registers;
}

}

