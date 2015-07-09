#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CallstackEntry
{
    const char* address;
    const char* module;
    const char* filename;
    int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CallstackData
{
    std::vector<CallstackEntry> callstack;
    uint64_t location;
    char filename[4096];
    int line;
    uint32_t selectedFrame;
    bool request;
    bool setSelectedFrame;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    CallstackData* userData = new CallstackData;

    memset(userData->filename, 0, sizeof(userData->filename));
    userData->line = -1;

    userData->location = 0;
    userData->request = false;
    userData->selectedFrame = 0;

    (void)uiFuncs;
    (void)serviceFunc;

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    delete (CallstackData*)userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Support floats

static void getAddressString(char* value, PDReader* reader, PDReaderIterator it)
{
    uint64_t regValue;
    uint32_t type = PDRead_findU64(reader, &regValue, "address", it);

    switch (type & PDReadStatus_typeMask)
    {
        case PDReadType_u8:
            sprintf(value, "0x%02x", (uint8_t)regValue); break;
        case PDReadType_s8:
            sprintf(value, "0x%02x", (int8_t)regValue); break;
        case PDReadType_u16:
            sprintf(value, "0x%04x", (uint16_t)regValue); break;
        case PDReadType_s16:
            sprintf(value, "0x%04x", (int16_t)regValue); break;
        case PDReadType_u32:
            sprintf(value, "0x%08x", (uint32_t)regValue); break;
        case PDReadType_s32:
            sprintf(value, "0x%08x", (int32_t)regValue); break;
        case PDReadType_u64:
            sprintf(value, "0x%014llx", (uint64_t)regValue); break;
        case PDReadType_s64:
            sprintf(value, "0x%014llx", (int64_t)regValue); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int findSeparator(const char* str)
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

static void updateCallstack(CallstackData* data, PDReader* reader)
{
    PDReaderIterator it;

    if (PDRead_findArray(reader, &it, "callstack", 0) == PDReadStatus_notFound)
        return;

    for (CallstackEntry& entry : data->callstack)
    {
        free((void*)entry.address);
        free((void*)entry.module);
        free((void*)entry.filename);
    }

    data->callstack.clear();

    // TODO: Have a "spec" for the callstack to be used

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* filename = "";
        const char* module = "";
        char address[64] = { 0 };
        uint32_t line = (uint32_t) ~0;

        CallstackEntry entry = { 0 };

        getAddressString(address, reader, it);

        PDRead_findString(reader, &filename, "filename", it);
        PDRead_findString(reader, &module, "module_name", it);
        PDRead_findU32(reader, &line, "line", it);

        entry.address = strdup(address);
        entry.line = -1;

        if (filename)
        {
            int fSep = findSeparator(filename);
            entry.filename = strdup(&filename[fSep]);
        }

        if (module)
        {
            int mSep = findSeparator(module);
            entry.module = strdup(&module[mSep]);
        }

        if (line != (uint32_t) ~0)
            entry.line = (int)line;

        data->callstack.push_back(entry);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawText(PDUI* uiFuncs, const char* text)
{
    if (text)
        uiFuncs->text(text);
    else
        uiFuncs->text("");

    uiFuncs->nextColumn();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawTextInt(PDUI* uiFuncs, int v)
{
    if (v >= 0)
        uiFuncs->text("%d", v);
    else
        uiFuncs->text("");

    uiFuncs->nextColumn();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showUI(PDUI* uiFuncs, CallstackData* data)
{
    uiFuncs->columns(4, "callstack", true);
    uiFuncs->text("Address"); uiFuncs->nextColumn();
    uiFuncs->text("Module"); uiFuncs->nextColumn();
    uiFuncs->text("Name"); uiFuncs->nextColumn();
    uiFuncs->text("Line"); uiFuncs->nextColumn();

    uint32_t i = 0;
    PDVec2 size = { 0.0f, 0.0f };

    const uint32_t oldSelectedFrame = data->selectedFrame;

    for (CallstackEntry& entry : data->callstack)
    {
		if (uiFuncs->selectable(entry.address, data->selectedFrame == i, PDUISelectableFlags_SpanAllColumns, size))
			data->selectedFrame = i;

    	uiFuncs->nextColumn();
        drawText(uiFuncs, entry.module);
        drawText(uiFuncs, entry.filename);
        drawTextInt(uiFuncs, entry.line);

        i++;
    }

    if (oldSelectedFrame != data->selectedFrame)
    	data->setSelectedFrame = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* reader, PDWriter* writer)
{
    uint32_t event;

    CallstackData* data = (CallstackData*)userData;

    data->request = false;
    data->setSelectedFrame = false;

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_setCallstack:
            {
                updateCallstack(data, reader);
                break;
            }

            case PDEventType_selectFrame : 
			{
    			PDRead_findU32(reader, &data->selectedFrame, "frame", 0);
				break;
			}

            case PDEventType_setExceptionLocation:
            {
                const char* filename = 0;
                uint32_t line = 0;
                uint64_t location = 0;

                PDRead_findU64(reader, &location, "address", 0);

                if (location != data->location)
                {
                    data->location = location;
                    data->request = true;
                }

                PDRead_findString(reader, &filename, "filename", 0);
                PDRead_findU32(reader, &line, "line", 0);

                if (!filename || line == 0)
                    break;

                if (strcmp(data->filename, filename))
                {
                    strcpy(data->filename, filename);
                    data->line = (int)line;
                    data->request = true;
                }
            }
        }
    }

    showUI(uiFuncs, data);

    if (data->setSelectedFrame)
	{
		PDWrite_eventBegin(writer, PDEventType_selectFrame);
		PDWrite_u32(writer, "frame", (uint32_t)data->selectedFrame);
		PDWrite_eventEnd(writer);
	}

    if (data->request)
    {
        PDWrite_eventBegin(writer, PDEventType_getCallstack);
        PDWrite_eventEnd(writer);
    }

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

