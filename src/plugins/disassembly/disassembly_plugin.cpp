#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Line
{
    uint64_t address;
    const char* text;
    bool breakpoint;
    uint8_t addressSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int BlockSize = 64;

struct Block
{
	uint64_t id;
	uint64_t address;
	uint32_t addressEnd;
	std::vector<Line> lines;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DissassemblyData
{
	std::vector<Block*> blocks;
    uint64_t location;
    uint64_t pc;
    uint8_t locationSize;
    bool requestDisassembly; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    DissassemblyData* userData = new DissassemblyData; 
    userData->location = 0;
    userData->pc = 0;
    userData->locationSize = 0;
    userData->requestDisassembly = false;

    (void)uiFuncs;
    (void)serviceFunc;

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    delete (DissassemblyData*)userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Block* findBlock(DissassemblyData* data, uint64_t blockId)
{
	for (Block* b : data->blocks)
	{
		if (b->id == blockId)
			return b;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Block* createBlock(DissassemblyData* data, uint64_t address, uint64_t blockId)
{
	Block* block = new Block;

	block->id = blockId;
	block->address = address & (BlockSize - 1);
	block->addressEnd = (address & (BlockSize - 1)) + BlockSize;

	data->blocks.push_back(block);

	return block;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void insertLineBlock(Block* block, uint64_t address, const char* text)
{
	// So this isn't very smart but will do for now

	for (auto i = block->lines.begin(); i != block->lines.end(); ++i)
	{
		Line& line = *i;

		if (address < line.address)
		{
			Line newLine = { 0 };
			newLine.address = address;
			newLine.text = strdup(text);
			block->lines.insert(i, newLine);
			return;
		}

		// found matching address, update the disassembly

		if (line.address == address)
		{
			free((void*)line.text);
			line.text = (const char*)strdup(text);
			return;
		}

		// TODO: Handle the case if a new line is inbetween lines, meaning the code has been modified
		// so the disasssembly is out of data
	}

	// not find, insert the line
	Line newLine = { 0 };
	newLine.address = address;
	newLine.text = strdup(text);
	block->lines.push_back(newLine);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void insertLine(DissassemblyData* data, uint64_t address, const char* text)
{
	Block* block = 0;

	// first find the block which this address should be in

	uint64_t blockId = address / BlockSize; 

	if (!(block = findBlock(data, blockId)))
		block = createBlock(data, address, blockId);

	insertLineBlock(block, address, text);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setDisassemblyCode(DissassemblyData* data, PDReader* reader)
{
    PDReaderIterator it;

    if (PDRead_findArray(reader, &it, "disassembly", 0) == PDReadStatus_notFound)
        return;

    while (PDRead_getNextEntry(reader, &it))
    {
    	uint64_t address;
        const char* text;

        PDRead_findU64(reader, &address, "address", it);
        PDRead_findString(reader, &text, "line", it);

		insertLine(data, address, text);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void renderUI(DissassemblyData* data, PDUI* uiFuncs)
{
    uiFuncs->text("");  // TODO: Temporary

    if (data->blocks.size() == 0)
    	return;

	Block* block = data->blocks[0];

    for (Line& line : block->lines)
    {
        if (line.address == data->pc)
        {
            PDRect rect;
            PDVec2 pos = uiFuncs->getCursorPos();
            rect.x = pos.x;
            rect.y = pos.y;
            rect.width = 400;
            rect.height = 14;
            uiFuncs->fillRect(rect, PD_COLOR_32(200, 0, 0, 127));
        }

        uiFuncs->text("0x%04x %s", (uint64_t)line.address, line.text);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateRegisters(DissassemblyData* data, PDReader* reader)
{
    PDReaderIterator it;

    if (PDRead_findArray(reader, &it, "registers", 0) == PDReadStatus_notFound)
        return;

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* name = "";

        PDRead_findString(reader, &name, "name", it);

        if (!strcmp(name, "pc"))
        {
            PDRead_findU64(reader, &data->pc, "register", it);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer)
{
    uint32_t event;

    DissassemblyData* data = (DissassemblyData*)userData;

	data->requestDisassembly = false;

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setDisassembly:
            {
                setDisassemblyCode(data, inEvents);
                break;
            }

            case PDEventType_setExceptionLocation:
            {
            	uint64_t location = 0;

                PDRead_findU64(inEvents, &location, "address", 0);

            	if (location != data->location)
				{
					data->location = location;
					data->requestDisassembly = true;
				}

                PDRead_findU8(inEvents, &data->locationSize, "address_size", 0);
                break;
            }

            case PDEventType_setRegisters:
			{
                updateRegisters(data, inEvents); 
                break;
			}

        }
    }

    renderUI(data, uiFuncs);

    if (data->requestDisassembly)
	{
		int pc = (int)(data->pc) & ~(BlockSize - 1);
		PDWrite_eventBegin(writer, PDEventType_getDisassembly);
		PDWrite_u64(writer, "address_start", (uint64_t)pc);
		PDWrite_u32(writer, "instruction_count", (uint32_t)BlockSize / 3);
		PDWrite_eventEnd(writer);
	}

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Disassembly",
    createInstance,
    destroyInstance,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
    {
        registerPlugin(PD_VIEW_API_VERSION, &plugin, privateData);
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

