#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Line
{
    uint64_t address;
    const char* text;
    bool breakpoint;
    uint8_t addressSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DisassemblyRange
{
    Line* lines;
    int lineCount;
    uint64_t rangeStart;
    uint64_t rangeEnd;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DissassemblyData
{
    DisassemblyRange range;
    uint64_t location;
    uint64_t pc;
    uint8_t locationSize;
    bool requestDisassembly; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    DissassemblyData* userData = (DissassemblyData*)malloc(sizeof(DissassemblyData));
    memset(userData, 0, sizeof(DissassemblyData));

    (void)uiFuncs;
    (void)serviceFunc;

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void freeRange(DisassemblyRange* range)
{
    for (int i = 0, end = range->lineCount; i < end; ++i)
        free((void*)range->lines[i].text);

    free(range->lines);

    range->lineCount = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setDisassemblyCode(DissassemblyData* data, PDReader* reader)
{
    Line* line;

    PDReaderIterator it;

    if (PDRead_findArray(reader, &it, "disassembly", 0) == PDReadStatus_notFound)
        return;

    freeRange(&data->range);

    int linesCount = 0;

    PDReaderIterator t = it;

    while (PDRead_getNextEntry(reader, &t))
    {
        linesCount++;
    }

    data->range.lines = line = (Line*)malloc((size_t)linesCount * sizeof(Line));
    data->range.lineCount = linesCount;

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* text;
        line->breakpoint = false;

        line->addressSize = (uint8_t)PDRead_findU64(reader, &line->address, "address", it);

        PDRead_findString(reader, &text, "line", it);

        line->text = strdup(text);

        line++;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void renderUI(DissassemblyData* data, PDUI* uiFuncs)
{
    uiFuncs->text("");  // TODO: Temporary

    for (int i = 0; i < data->range.lineCount; ++i)
    {
        if (data->range.lines[i].address == data->pc)
        {
            PDRect rect;
            PDVec2 pos = uiFuncs->getCursorPos();
            rect.x = pos.x;
            rect.y = pos.y;
            rect.width = 400;
            rect.height = 14;
            uiFuncs->fillRect(rect, PD_COLOR_32(200, 0, 0, 127));
        }

        uiFuncs->text("0x%04x %s", (uint64_t)data->range.lines[i].address, data->range.lines[i].text);
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
		// Temporary req

		int pc = (int)(data->pc) - 0x40;

		if (pc < 0)
			pc = 0;

		PDWrite_eventBegin(writer, PDEventType_getDisassembly);
		PDWrite_u64(writer, "address_start", (uint64_t)pc);
		PDWrite_u32(writer, "instruction_count", (uint32_t)40);
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

