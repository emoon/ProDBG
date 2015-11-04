#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CallstackEntry {
    const char* address;
    const char* module;
    const char* filename;
    int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CallstackData {
    std::vector<CallstackEntry> callstack;
    uint64_t location;
    char filename[4096];
    int line;
    uint32_t selectedFrame;
    bool request;
    bool setSelectedFrame;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc) {
    (void)serviceFunc;
    CallstackData* user_data = new CallstackData;

    memset(user_data->filename, 0, sizeof(user_data->filename));
    user_data->line = -1;

    user_data->location = 0;
    user_data->request = false;
    user_data->selectedFrame = 0;

    (void)uiFuncs;
    (void)serviceFunc;

    return user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* user_data) {
    delete (CallstackData*)user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Support floats

static void getAddressString(char* value, PDReader* reader, PDReaderIterator it) {
    uint64_t regValue;
    uint32_t type = PDRead_find_u64(reader, &regValue, "address", it);

    switch (type & PDReadStatus_TypeMask) {
        case PDReadType_U8:
            sprintf(value, "0x%02x", (uint8_t)regValue); break;
        case PDReadType_S8:
            sprintf(value, "0x%02x", (int8_t)regValue); break;
        case PDReadType_U16:
            sprintf(value, "0x%04x", (uint16_t)regValue); break;
        case PDReadType_S16:
            sprintf(value, "0x%04x", (int16_t)regValue); break;
        case PDReadType_U32:
            sprintf(value, "0x%08x", (uint32_t)regValue); break;
        case PDReadType_S32:
            sprintf(value, "0x%08x", (int32_t)regValue); break;
        case PDReadType_U64:
            sprintf(value, "0x%014llx", (uint64_t)regValue); break;
        case PDReadType_S64:
            sprintf(value, "0x%014llx", (int64_t)regValue); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int findSeparator(const char* str) {
    size_t len = strlen(str);

    for (size_t i = len; i != 0; --i) {
        if (str[i] == '/')
            return (int)i + 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateCallstack(CallstackData* data, PDReader* reader) {
    PDReaderIterator it;

    if (PDRead_find_array(reader, &it, "callstack", 0) == PDReadStatus_NotFound)
        return;

    for (CallstackEntry& entry : data->callstack) {
        free((void*)entry.address);
        free((void*)entry.module);
        free((void*)entry.filename);
    }

    data->callstack.clear();

    // TODO: Have a "spec" for the callstack to be used

    while (PDRead_get_next_entry(reader, &it)) {
        const char* filename = "";
        const char* module = "";
        char address[64] = { 0 };
        uint32_t line = (uint32_t) ~0;

        CallstackEntry entry = { 0 };

        getAddressString(address, reader, it);

        PDRead_find_string(reader, &filename, "filename", it);
        PDRead_find_string(reader, &module, "module_name", it);
        PDRead_find_u32(reader, &line, "line", it);

        entry.address = strdup(address);
        entry.line = -1;

        if (filename) {
            int fSep = findSeparator(filename);
            entry.filename = strdup(&filename[fSep]);
        }

        if (module) {
            int mSep = findSeparator(module);
            entry.module = strdup(&module[mSep]);
        }

        if (line != (uint32_t) ~0)
            entry.line = (int)line;

        data->callstack.push_back(entry);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawText(PDUI* uiFuncs, const char* text) {
    if (text)
        uiFuncs->text(text);
    else
        uiFuncs->text("");

    uiFuncs->next_column();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawTextInt(PDUI* uiFuncs, int v) {
    if (v >= 0)
        uiFuncs->text("%d", v);
    else
        uiFuncs->text("");

    uiFuncs->next_column();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showUI(PDUI* uiFuncs, CallstackData* data) {
    uiFuncs->columns(4, "callstack", true);
    uiFuncs->text("Address"); uiFuncs->next_column();
    uiFuncs->text("Module"); uiFuncs->next_column();
    uiFuncs->text("Name"); uiFuncs->next_column();
    uiFuncs->text("Line"); uiFuncs->next_column();

    uint32_t i = 0;
    PDVec2 size = { 0.0f, 0.0f };

    const uint32_t oldSelectedFrame = data->selectedFrame;

    for (CallstackEntry& entry : data->callstack) {
        if (uiFuncs->selectable(entry.address, data->selectedFrame == i, PDUISelectableFlags_SpanAllColumns, size))
            data->selectedFrame = i;

        uiFuncs->next_column();
        drawText(uiFuncs, entry.module);
        drawText(uiFuncs, entry.filename);
        drawTextInt(uiFuncs, entry.line);

        i++;
    }

    if (oldSelectedFrame != data->selectedFrame)
        data->setSelectedFrame = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* user_data, PDUI* uiFuncs, PDReader* reader, PDWriter* writer) {
    uint32_t event;

    CallstackData* data = (CallstackData*)user_data;

    data->request = false;
    data->setSelectedFrame = false;

    while ((event = PDRead_get_event(reader)) != 0) {
        switch (event) {
            case PDEventType_SetCallstack:
            {
                updateCallstack(data, reader);
                break;
            }

            case PDEventType_SelectFrame:
            {
                PDRead_find_u32(reader, &data->selectedFrame, "frame", 0);
                break;
            }

            case PDEventType_SetExceptionLocation:
            {
                const char* filename = 0;
                uint32_t line = 0;
                uint64_t location = 0;

                PDRead_find_u64(reader, &location, "address", 0);

                if (location != data->location) {
                    data->location = location;
                    data->request = true;
                }

                PDRead_find_string(reader, &filename, "filename", 0);
                PDRead_find_u32(reader, &line, "line", 0);

                if (!filename || line == 0)
                    break;

                if (strcmp(data->filename, filename)) {
                    strcpy(data->filename, filename);
                    data->line = (int)line;
                    data->request = true;
                }
            }
        }
    }

    showUI(uiFuncs, data);

    if (data->setSelectedFrame) {
        PDWrite_event_begin(writer, PDEventType_SelectFrame);
        PDWrite_u32(writer, "frame", (uint32_t)data->selectedFrame);
        PDWrite_event_end(writer);
    }

    if (data->request) {
        PDWrite_event_begin(writer, PDEventType_GetCallstack);
        PDWrite_event_end(writer);
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

    PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
        registerPlugin(PD_VIEW_API_VERSION, &plugin, private_data);
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

