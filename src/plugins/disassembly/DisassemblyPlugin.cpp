#include <PDView.h>
#include <stdlib.h>
#include <stdio.h>
#include <PDBackend.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DissassemblyData
{
	PDUICustomView view;
	char* code;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawCallback(void* userData, PDRect* viewRect, PDUIPainter* painter)
{
	int fontX;
	int fontY;

	DissassemblyData* data = (DissassemblyData*)userData; 

	PDUIPaint_fontMetrics(painter, &fontX, &fontY);
	PDUIPaint_fillRect(painter, viewRect, 0x7f7f);
	PDUIPaint_setPen(painter, 0x1fffffff);

	printf("beging render\n");

	if (data->code)
	{
		int y = 30;
		char* pch = strtok(data->code,"\n");

		printf("strtok %p\n", pch);

		while (pch != NULL)
		{
			printf("tok text %d : %s\n", y, pch);
			PDUIPaint_drawText(painter, viewRect->x, y, pch);

			pch = strtok(NULL, "\n");
			y += fontX;
		}
	}

	printf("end render\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
	(void)serviceFunc;
	DissassemblyData* userData = (DissassemblyData*)malloc(sizeof(DissassemblyData));
	memset(userData, 0, sizeof(DissassemblyData));
	
	userData->view = PDUICustomView_create(uiFuncs, userData, drawCallback);

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

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer)
{
	uint32_t event;

	DissassemblyData* data = (DissassemblyData*)userData; 

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setDisassembly : 
			{
            	setDisassemblyCode(data, inEvents); break; 
            	PDUICustomView_repaint(uiFuncs, data->view);
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
    "Disassembly",
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

