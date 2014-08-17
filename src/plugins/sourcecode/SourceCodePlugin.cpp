#include <PDView.h>
#include <stdlib.h>
#include <stdio.h>
#include <PDBackend.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SourceCodeData
{
	PDUICustomView view;
	char* code;
	uint64_t location;
	uint8_t locationSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Do not read whole file to memory? 

int parseFile(const char* filename)
{


}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawCallback(void* userData, PDRect* viewRect, PDUIPainter* painter)
{
	int fontX;
	int fontY;

	char locationAddress[128];

	SourceCodeData* data = (SourceCodeData*)userData; 

	PDUIPaint_fontMetrics(painter, &fontX, &fontY);
	PDUIPaint_fillRect(painter, viewRect, 0x7f7f);
	PDUIPaint_setPen(painter, 0x1fffffff);


	//PDUIPaint_setPen(painter, 0x1fffffff);
	//PDUIPaint_drawText(painter, viewRect->x, y, pch);

	//y += fontX + 2;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
	(void)serviceFunc;
	SourceCodeData* userData = (SourceCodeData*)malloc(sizeof(SourceCodeData));
	memset(userData, 0, sizeof(SourceCodeData));
	
	userData->view = PDUICustomView_create(uiFuncs, userData, drawCallback);

	return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
	free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setDisassemblyCode(SourceCodeData* data, PDReader* inEvents)
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

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer)
{
	uint32_t event;

	SourceCodeData* data = (SourceCodeData*)userData; 

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setDisassembly : 
			{
            	setDisassemblyCode(data, inEvents);
            	PDUICustomView_repaint(uiFuncs, data->view);
            	break;
			}

            case PDEventType_setExceptionLocation : 
			{
    			PDRead_findU64(inEvents, &data->location, "address", 0);
    			PDRead_findU8(inEvents, &data->locationSize, "address_size", 0);
			}
        }
    }

	// TODO: Make sure to only request data if the debugger is in break/exception state
	/*
    PDWrite_eventBegin(writer, PDEventType_getDisassembly);
    PDWrite_u64(writer, "address_start", 0);
    PDWrite_u32(writer, "instruction_count", (uint32_t)10);
    PDWrite_eventEnd(writer);
    */

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    0,    // version
    "SourceCode",
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

