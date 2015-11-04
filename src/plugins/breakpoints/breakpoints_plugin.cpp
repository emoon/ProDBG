#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Location {
    char* filename;
    char* address;
    int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Breakpoint {
    Location location;
    char* condition;
    int32_t id;
    bool enabled;
    bool markDelete;
    int pendingCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BreakpointsData {
    BreakpointsData() : addressSize(2), maxPath(8192) {}

    std::list<Breakpoint*> breakpoints;
    int addressSize;
    size_t maxPath;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Breakpoint* createBreakpoint(BreakpointsData* user_data) {
    Breakpoint* bp = (Breakpoint*)malloc(sizeof(Breakpoint));
    memset(bp, 0, sizeof(Breakpoint));

    bp->location.address = (char*)malloc(user_data->maxPath);
    bp->condition = (char*)malloc(user_data->maxPath);

    memset(bp->location.address, 0, user_data->maxPath);
    memset(bp->condition, 0, user_data->maxPath);

    bp->enabled = false;
    bp->id = -1;
    bp->pendingCount = 1;   // this is used as the breakpoint isn't valid until we got a reply back that it is

    return bp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyBreakpoint(Breakpoint* bp) {
    free(bp->location.filename);
    free(bp->location.address);
    free(bp->condition);
    free(bp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc) {
    (void)serviceFunc;
    BreakpointsData* user_data = new BreakpointsData;

    (void)uiFuncs;
    (void)serviceFunc;

    return user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* user_data) {
    BreakpointsData* data = (BreakpointsData*)user_data;
    delete data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateCondition(Breakpoint* bp, PDReader* reader) {
    const char* condition;

    bp->condition = 0;

    PDRead_find_string(reader, &condition, "condition", 0);

    if (condition)
        bp->condition = strdup(condition);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void toogleBreakpointFileLine(BreakpointsData* data, PDReader* reader) {
    const char* filename = 0;
    uint32_t line;

    char fileLine[8192];

    PDRead_find_string(reader, &filename, "filename", 0);
    PDRead_find_u32(reader, &line, "line", 0);

    if (!filename) {
        return;
    }

    for (auto i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i) {
        if ((*i)->location.line == (int)line && !strcmp((*i)->location.filename, filename)) {
            destroyBreakpoint(*i);
            data->breakpoints.erase(i);
            return;
        }
    }

    Breakpoint* breakpoint = createBreakpoint(data);

    sprintf(fileLine, "%s:%d\n", filename, line);

    breakpoint->location.filename = (char*)malloc(data->maxPath);
    breakpoint->location.line = (int)line;

    strcpy(breakpoint->location.filename, fileLine);

    updateCondition(breakpoint, reader);

    data->breakpoints.push_back(breakpoint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void toggleBreakpointAddress(BreakpointsData* data, PDReader* reader) {
    const char* address;

    if (PDRead_find_string(reader, &address, "address", 0) == PDReadStatus_NotFound)
        return;

    for (auto i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i) {
        if (!strcmp((*i)->location.address, address)) {
            destroyBreakpoint(*i);
            data->breakpoints.erase(i);
            return;
        }
    }

    Breakpoint* breakpoint = createBreakpoint(data);
    breakpoint->location.address = (char*)malloc(data->maxPath);

    strcpy(breakpoint->location.address, address);

    updateCondition(breakpoint, reader);

    data->breakpoints.push_back(breakpoint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* user_data, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer) {
    uint32_t event;
    (void)uiFuncs;
    (void)writer;

    BreakpointsData* data = (BreakpointsData*)user_data;

    while ((event = PDRead_get_event(inEvents)) != 0) {
        switch (event) {
            case PDEventType_SetBreakpoint:
            {
                toogleBreakpointFileLine(data, inEvents);
                break;
            }

            case PDEventType_ReplyBreakpoint:
            {
                uint64_t address = 0;
                uint32_t id = (uint32_t) ~0;

                PDRead_find_u64(inEvents, &address, "address", 0);
                PDRead_find_u32(inEvents, &id, "id", 0);

                for (Breakpoint* bp : data->breakpoints) {
                    if ((uint64_t)strtol(bp->location.address, 0, 16) == address) {
                        bp->pendingCount = 0;       // breakpoint accepted
                        printf("bp view: updated breakpoint with id %d (was %d)\n", id, bp->id);
                        bp->id = (int)id;
                        break;
                    }
                }

                break;
            }
        }
    }

    uiFuncs->text("");

    if (uiFuncs->button("Add Breakpoint", { 0.0f, 0.0f } )) {
        Breakpoint* bp = createBreakpoint(data);
        data->breakpoints.push_back(bp);
    }

    uiFuncs->columns(4, "", true);
    //uiFuncs->text(""); uiFuncs->next_column();
    uiFuncs->text("Name/Address"); uiFuncs->next_column();
    uiFuncs->text("Label"); uiFuncs->next_column();
    uiFuncs->text("Condition"); uiFuncs->next_column();
    uiFuncs->text(""); uiFuncs->next_column();

    for (auto& i : data->breakpoints) {
        Breakpoint* bp = i;
        bool needUpdate = false;

        uiFuncs->push_id_ptr(bp);

        //if (uiFuncs->checkbox("Enabled", &bp->enabled))
        //	needUpdate = true;

        if (bp->location.filename) {
            uiFuncs->input_text("##filename", bp->location.filename, (int)data->maxPath, 0, 0, 0);
        }else {
            if (uiFuncs->input_text("##address", bp->location.address, (int)data->maxPath,
                                    PDUIInputTextFlags_CharsHexadecimal | PDUIInputTextFlags_EnterReturnsTrue, 0, 0))
                needUpdate = true;
        }

        uiFuncs->next_column();

        uiFuncs->text("");
        uiFuncs->next_column();

        uiFuncs->text(""); // no condition for now

        //if (uiFuncs->input_text("##condition", bp->condition, (int)data->maxPath, PDInputTextFlags_EnterReturnsTrue, 0, 0))
        //	needUpdate = true;

        uiFuncs->next_column();

        if (needUpdate) {
            // TODO: Add support for file/line

            PDWrite_event_begin(writer, PDEventType_SetBreakpoint);
            PDWrite_u64(writer, "address", (uint64_t)strtol(bp->location.address, 0, 16));

            //if (bp->condition[0] != 0)
            //	PDWrite_string(writer, "condition", bp->condition);

            if (bp->id != -1)
                PDWrite_u32(writer, "id", (uint32_t)bp->id);

            PDWrite_event_end(writer);

            printf("Sending breakpint\n");
        }

        if (uiFuncs->button("Delete", {0.0f, 0.0f})) {
            PDWrite_event_begin(writer, PDEventType_DeleteBreakpoint);
            PDWrite_u32(writer, "id", (uint32_t)bp->id);
            PDWrite_event_end(writer);
            bp->markDelete = true;
        }

        uiFuncs->next_column();

        uiFuncs->pop_id();
    }

    // Delete breakpoints that have been marked delete

    for (auto i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i) {
        Breakpoint* bp = *i;

        if (bp->pendingCount > 1)
            bp->pendingCount++;

        if (bp->markDelete || bp->pendingCount >= 10)
            i = data->breakpoints.erase(i);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Breakpoint View",
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


