#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CallStackData
{
    int dummy;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    CallStackData* userData = (CallStackData*)malloc(sizeof(CallStackData));

    (void)uiFuncs;
    (void)serviceFunc;

    /*
       static const char* headers[] = { "Address", "Module", "Name", "line", 0 };
       userData->callStackList = PDUIListView_create(uiFuncs, headers, 0);
     */

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
		case PDReadType_u64: sprintf(value, "0x%014llx", (uint64_t)regValue); break;
		case PDReadType_s64: sprintf(value, "0x%014llx", (int64_t)regValue); break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int findSeparator(const char* str)
{
	size_t len = strlen(str);

	for (size_t i = len; i != 0; --i)
	{
		if (str[i] == '/')
			return (int)i + 1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(CallStackData* data, PDReader* reader, PDUI* uiFuncs)
{
    PDReaderIterator it;
    (void)data;

    if (PDRead_findArray(reader, &it, "callstack", 0) == PDReadStatus_notFound)
        return;

	uiFuncs->text("");

	// TODO: Have a "spec" for the callstack to be used

	uiFuncs->columns(4, "callstack", true);
	uiFuncs->text("Address"); uiFuncs->nextColumn();
	uiFuncs->text("Module"); uiFuncs->nextColumn();
	uiFuncs->text("Name"); uiFuncs->nextColumn();
	uiFuncs->text("Line"); uiFuncs->nextColumn();

	while (PDRead_getNextEntry(reader, &it))
	{
		const char* filename = "";
		const char* module = "";
		char address[64] = { 0 };
		uint32_t line = 0;

		getAddressString(address, reader, it);

		PDRead_findString(reader, &filename, "filename", it);
		PDRead_findString(reader, &module, "module_name", it);
		PDRead_findU32(reader, &line, "line", it);

		int fSep = findSeparator(filename);
		int mSep = findSeparator(module);

		uiFuncs->text(address); uiFuncs->nextColumn();
		uiFuncs->text(&module[mSep]); uiFuncs->nextColumn();
		uiFuncs->text(&filename[fSep]); uiFuncs->nextColumn();
		uiFuncs->text("%d", line); uiFuncs->nextColumn();
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
            case PDEventType_setCallstack:
                showInUI(data, inEvents, uiFuncs); break;
        }
    }

    // Request callstack data

    PDWrite_eventBegin(outEvents, PDEventType_getCallstack);
    PDWrite_u8(outEvents, "dummy_remove", 0);	// TODO: Remove me
    PDWrite_eventEnd(outEvents);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "CallStack",
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

