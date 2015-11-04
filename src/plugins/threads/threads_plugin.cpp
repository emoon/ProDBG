#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ThreadsData {
    int selectedThread;
    int threadId;
    bool requestData;
    bool setSelectedThread;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc) {
    (void)serviceFunc;
    (void)uiFuncs;

    ThreadsData* user_data = (ThreadsData*)malloc(sizeof(ThreadsData));

    user_data->selectedThread = 0;
    user_data->threadId = 0;

    return user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* user_data) {
    free(user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(ThreadsData* user_data, PDReader* reader, PDUI* uiFuncs) {
    PDReaderIterator it;
    ThreadsData* data = (ThreadsData*)user_data;

    if (PDRead_find_array(reader, &it, "threads", 0) == PDReadStatus_NotFound)
        return;

    uiFuncs->text("");

    uiFuncs->columns(3, "threads", true);
    uiFuncs->text("Id"); uiFuncs->next_column();
    uiFuncs->text("Name"); uiFuncs->next_column();
    uiFuncs->text("Function"); uiFuncs->next_column();

    int i = 0;

    PDVec2 size = { 0.0f, 0.0f };

    data->setSelectedThread = false;

    int oldSelectedThread = data->selectedThread;

    while (PDRead_get_next_entry(reader, &it)) {
        uint64_t id;
        const char* name = "";
        const char* function = "";

        PDRead_find_u64(reader, &id, "id", it);
        PDRead_find_string(reader, &name, "name", it);
        PDRead_find_string(reader, &function, "function", it);

        char label[32];
        sprintf(label, "%llx", id);

        if (uiFuncs->selectable(label, data->selectedThread == i, 1 << 1, size)) {
            data->selectedThread = i;
            data->threadId = (int)id;
        }

        uiFuncs->next_column();
        uiFuncs->text(name); uiFuncs->next_column();
        uiFuncs->text(function); uiFuncs->next_column();

        i++;
    }

    if (oldSelectedThread != data->selectedThread) {
        data->setSelectedThread = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* user_data, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents) {
    uint32_t event = 0;

    ThreadsData* data = (ThreadsData*)user_data;

    data->requestData = false;
    data->setSelectedThread = false;

    while ((event = PDRead_get_event(inEvents)) != 0) {
        switch (event) {
            case PDEventType_SetThreads:
            {
                showInUI((ThreadsData*)user_data, inEvents, uiFuncs);
                break;
            }

            case PDEventType_SetExceptionLocation:
            {
                data->requestData = true;
                break;
            }
        }
    }

    // Request threads data

    if (data->setSelectedThread) {
        PDWrite_event_begin(outEvents, PDEventType_SelectThread);
        printf("writing thread id %d\n", data->threadId);
        PDWrite_u32(outEvents, "thread_id", (uint32_t)data->threadId);
        PDWrite_event_end(outEvents);
    }

    if (data->requestData) {
        PDWrite_event_begin(outEvents, PDEventType_GetThreads);
        PDWrite_event_end(outEvents);
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

    PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
        registerPlugin(PD_VIEW_API_VERSION, &plugin, private_data);
    }

}

