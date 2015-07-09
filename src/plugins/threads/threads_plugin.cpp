#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ThreadsData
{
    int selectedThread;
    int threadId;
    bool requestData;
    bool setSelectedThread;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;

    ThreadsData* userData = (ThreadsData*)malloc(sizeof(ThreadsData));

	userData->selectedThread = 0;
	userData->threadId = 0;

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(ThreadsData* userData, PDReader* reader, PDUI* uiFuncs)
{
    PDReaderIterator it;
    ThreadsData* data = (ThreadsData*)userData;

    if (PDRead_findArray(reader, &it, "threads", 0) == PDReadStatus_notFound)
        return;

    uiFuncs->text("");

    uiFuncs->columns(3, "threads", true);
    uiFuncs->text("Id"); uiFuncs->nextColumn();
    uiFuncs->text("Name"); uiFuncs->nextColumn();
    uiFuncs->text("Function"); uiFuncs->nextColumn();

    int i = 0;

    PDVec2 size = { 0.0f, 0.0f };

    data->setSelectedThread = false;

    int oldSelectedThread = data->selectedThread;

    while (PDRead_getNextEntry(reader, &it))
    {
    	uint64_t id;
        const char* name = "";
        const char* function = "";

        PDRead_findU64(reader, &id, "id", it);
        PDRead_findString(reader, &name, "name", it);
        PDRead_findString(reader, &function, "function", it);

		char label[32];
		sprintf(label, "%llx", id);

		if (uiFuncs->selectable(label, data->selectedThread == i, 1 << 1, size))
		{
			data->selectedThread = i;
			data->threadId = (int)id;
		}

        uiFuncs->nextColumn();
        uiFuncs->text(name); uiFuncs->nextColumn();
        uiFuncs->text(function); uiFuncs->nextColumn();

        i++;
    }

    if (oldSelectedThread != data->selectedThread)
	{
    	data->setSelectedThread = true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
    uint32_t event = 0;

    ThreadsData* data = (ThreadsData*)userData;

    data->requestData = false;
    data->setSelectedThread = false;

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setThreads:
            {
                showInUI((ThreadsData*)userData, inEvents, uiFuncs);
                break;
            }

            case PDEventType_setExceptionLocation:
            {
            	data->requestData = true;
				break;
			}
        }
    }

    // Request threads data

	if (data->setSelectedThread)
	{
		PDWrite_eventBegin(outEvents, PDEventType_selectThread);
		printf("writing thread id %d\n", data->threadId);
		PDWrite_u32(outEvents, "thread_id", (uint32_t)data->threadId);
		PDWrite_eventEnd(outEvents);
	}

	if (data->requestData)
	{
		PDWrite_eventBegin(outEvents, PDEventType_getThreads);
		PDWrite_eventEnd(outEvents);
	}

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Threads",
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

}

