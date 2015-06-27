#include <pd_view.h>
#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ThreadsData
{
    int dummy;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;

    ThreadsData* userData = (ThreadsData*)malloc(sizeof(ThreadsData));

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(ThreadsData* data, PDReader* reader, PDUI* uiFuncs)
{
    PDReaderIterator it;
    (void)data;

    if (PDRead_findArray(reader, &it, "threads", 0) == PDReadStatus_notFound)
        return;

    uiFuncs->text("");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
    uint32_t event = 0;

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setThreads:
            {
                showInUI((ThreadsData*)userData, inEvents, uiFuncs);
                break;
            }
        }
    }

    // Request threads data

    PDWrite_eventBegin(outEvents, PDEventType_getThreads);
    PDWrite_eventEnd(outEvents);

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

