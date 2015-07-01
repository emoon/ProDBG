#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct LocalsData
{
    int dummy;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;
    LocalsData* userData = (LocalsData*)malloc(sizeof(LocalsData));

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(LocalsData* data, PDReader* reader, PDUI* uiFuncs)
{
    PDReaderIterator it;
    (void)data;

    if (PDRead_findArray(reader, &it, "locals", 0) == PDReadStatus_notFound)
        return;

    uiFuncs->text("");

    uiFuncs->columns(3, "callstack", true);
    uiFuncs->text("Name"); uiFuncs->nextColumn();
    uiFuncs->text("Value"); uiFuncs->nextColumn();
    uiFuncs->text("Type"); uiFuncs->nextColumn();

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* name = "";
        const char* value = "";
        const char* type = "";

        PDRead_findString(reader, &name, "name", it);
        PDRead_findString(reader, &value, "value", it);
        PDRead_findString(reader, &type, "type", it);

        uiFuncs->text(name); uiFuncs->nextColumn();
        uiFuncs->text(value); uiFuncs->nextColumn();
        uiFuncs->text(type); uiFuncs->nextColumn();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
    uint32_t event = 0;

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setLocals:
            {
                showInUI((LocalsData*)userData, inEvents, uiFuncs);
                break;
            }
        }
    }

    // Request callstack data

    PDWrite_eventBegin(outEvents, PDEventType_getLocals);
    PDWrite_u8(outEvents, "dummy_remove", 0);   // TODO: Remove me
    PDWrite_eventEnd(outEvents);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Locals",
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

