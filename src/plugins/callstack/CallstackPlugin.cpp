#include <PDView.h>
#include <stdlib.h>
#include <stdio.h>
#include <PDBackend.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CallStackData
{
	PDUIListView callStackList;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
	(void)serviceFunc;
	CallStackData* userData = (CallStackData*)malloc(sizeof(CallStackData));
	
	static const char* headers[] = { "Address", "Module", "Name", "line", 0 };

	userData->callStackList = PDUIListView_create(uiFuncs, headers, 0);

	return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
	free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Support floats

static void getAddressString(char* value, PDReader* reader, PDReaderIterator it)
{
    uint64_t regValue;
	uint32_t type = PDRead_findU64(reader, &regValue, "address", it);

	switch (type & PDReadStatus_typeMask)
	{
		case PDReadType_u8: sprintf(value, "0x%02x", (uint8_t)regValue); break;
		case PDReadType_s8: sprintf(value, "0x%02x", (int8_t)regValue); break;
		case PDReadType_u16: sprintf(value, "0x%04x", (uint16_t)regValue); break;
		case PDReadType_s16: sprintf(value, "0x%04x", (int16_t)regValue); break;
		case PDReadType_u32: sprintf(value, "0x%08x", (uint32_t)regValue); break;
		case PDReadType_s32: sprintf(value, "0x%08x", (int32_t)regValue); break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(CallStackData* data, PDReader* reader, PDUI* uiFuncs)
{
    PDReaderIterator it;

    if (PDRead_findArray(reader, &it, "callstack", 0) == PDReadStatus_notFound)
    	return;

	PDUIListView_clear(uiFuncs, data->callStackList);

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* filename = "";
        const char* module = "";
        char address[64];
        char lineText[64];
        uint32_t line = 0;

        getAddressString(address, reader, it);

        PDRead_findString(reader, &filename, "filename", it);
        PDRead_findString(reader, &module, "module_name", it);
        PDRead_findU32(reader, &line, "line", it);

        sprintf(lineText, "%d", line);

		const char* values[] = { address, module, filename, lineText, 0 };

		PDUIListView_itemAdd(uiFuncs, data->callStackList, values);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
	CallStackData* data = (CallStackData*)userData;
	uint32_t event;

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setCallstack : showInUI(data, inEvents, uiFuncs); break; 
        }
    }

    // Request callstack data

    PDWrite_eventBegin(outEvents, PDEventType_getDisassembly);
    PDWrite_eventEnd(outEvents);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    0,    // version
    "CallStack",
    createInstance,
    destroyInstance,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

PD_EXPORT void InitPlugin(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin)
{
	(void)version;
	(void)serviceFunc;
    registerPlugin(PD_VIEW_API_VERSION, &plugin);
}

}

