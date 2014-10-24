#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DissassemblyData
{
    char* code;
    uint64_t location;
    uint8_t locationSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

static void buildAddressString(char* output, uint64_t address, uint8_t size)
{
    switch (size)
    {
        case 0:
            output[0] = 0; break;
        case 1:
            sprintf(output, "%02x", (uint8_t)address); break;
        case 2:
            sprintf(output, "%04x", (uint16_t)address); break;
        case 4:
            sprintf(output, "%08x", (uint32_t)address); break;
        case 8:
            sprintf(output, "%016llx", (uint64_t)address); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static void drawCallback(void* userData, PDRect* viewRect, PDUIPainter* painter)
{
    int fontX;
    int fontY;

    char locationAddress[128];

    DissassemblyData* data = (DissassemblyData*)userData;

    PDUIPaint_fontMetrics(painter, &fontX, &fontY);
    PDUIPaint_fillRect(painter, viewRect, 0x7f7f);
    PDUIPaint_setPen(painter, 0x1fffffff);

    buildAddressString(locationAddress, data->location, data->locationSize);

    if (data->code)
    {
        int y = 30;
        char* tempCode = strdup(data->code);
        char* pch = strtok(tempCode, "\n");

        while (pch != NULL)
        {
            if (strstr(pch, locationAddress) == pch)
                PDUIPaint_setPen(painter, 0x1f1f1fff);
            else
                PDUIPaint_setPen(painter, 0x1fffffff);

            PDUIPaint_drawText(painter, viewRect->x, y, pch);

            pch = strtok(NULL, "\n");
            y += fontX + 2;
        }

        free(tempCode);
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    DissassemblyData* userData = (DissassemblyData*)malloc(sizeof(DissassemblyData));
    memset(userData, 0, sizeof(DissassemblyData));

    (void)uiFuncs;
    (void)serviceFunc;

    //userData->view = PDUICustomView_create(uiFuncs, userData, drawCallback);

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setDisassemblyCode(DissassemblyData* data, PDReader* inEvents)
{
    const char* stringBuffer;

    PDRead_findString(inEvents, &stringBuffer, "string_buffer", 0);

    //printf("Got disassembly\n");
    //printf("disassembly %s\n", stringBuffer);

    free(data->code);

    if (!stringBuffer)
        return;

    data->code = strdup(stringBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void renderUI(DissassemblyData* data, PDUI* uiFuncs)
{
	if (!data->code)
		return;

	uiFuncs->text("");	// TODO: Temporary
	uiFuncs->text(data->code);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer)
{
    uint32_t event;

    DissassemblyData* data = (DissassemblyData*)userData;

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
                PDRead_findU64(inEvents, &data->location, "address", 0);
                PDRead_findU8(inEvents, &data->locationSize, "address_size", 0);
            }
        }
    }

    renderUI(data, uiFuncs);

    // Temporary req

	PDWrite_eventBegin(writer, PDEventType_getDisassembly);
	PDWrite_u64(writer, "address_start", 0);
	PDWrite_u32(writer, "instruction_count", (uint32_t)10);
	PDWrite_eventEnd(writer);

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

