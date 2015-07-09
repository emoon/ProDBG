#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Location
{
    char* filename;
    char* address;
    int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Breakpoint
{
    Location location;
    char* condition;
    int32_t id;
    bool enabled;
    bool markDelete;
    int pendingCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BreakpointsData
{
    BreakpointsData() : addressSize(2), maxPath(8192) {}

    std::list<Breakpoint*> breakpoints;
    int addressSize;
    size_t maxPath;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Breakpoint* createBreakpoint(BreakpointsData* userData)
{
    Breakpoint* bp = (Breakpoint*)malloc(sizeof(Breakpoint));
    memset(bp, 0, sizeof(Breakpoint));

    bp->location.address = (char*)malloc(userData->maxPath);
    bp->condition = (char*)malloc(userData->maxPath);

    memset(bp->location.address, 0, userData->maxPath);
    memset(bp->condition, 0, userData->maxPath);

    bp->enabled = false;
    bp->id = -1;
    bp->pendingCount = 1;   // this is used as the breakpoint isn't valid until we got a reply back that it is

    return bp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyBreakpoint(Breakpoint* bp)
{
    free(bp->location.filename);
    free(bp->location.address);
    free(bp->condition);
    free(bp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    BreakpointsData* userData = new BreakpointsData;

    (void)uiFuncs;
    (void)serviceFunc;

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    BreakpointsData* data = (BreakpointsData*)userData;
    delete data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateCondition(Breakpoint* bp, PDReader* reader)
{
    const char* condition;

    bp->condition = 0;

    PDRead_findString(reader, &condition, "condition", 0);

    if (condition)
        bp->condition = strdup(condition);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void toogleBreakpointFileLine(BreakpointsData* data, PDReader* reader)
{
    const char* filename = 0;
    uint32_t line;

    char fileLine[8192];

    PDRead_findString(reader, &filename, "filename", 0);
    PDRead_findU32(reader, &line, "line", 0);

    if (!filename)
        return;

    for (auto i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i)
    {
        if ((*i)->location.line == (int)line && !strcmp((*i)->location.filename, filename))
        {
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

void toggleBreakpointAddress(BreakpointsData* data, PDReader* reader)
{
    const char* address;

    if (PDRead_findString(reader, &address, "address", 0) == PDReadStatus_notFound)
        return;

    for (auto i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i)
    {
        if (!strcmp((*i)->location.address, address))
        {
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

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer)
{
    uint32_t event;
    (void)uiFuncs;
    (void)writer;

    BreakpointsData* data = (BreakpointsData*)userData;

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setBreakpoint:
            {
                toogleBreakpointFileLine(data, inEvents);
                break;
            }

            case PDEventType_replyBreakpoint:
            {
                uint64_t address = 0;
                uint32_t id = (uint32_t) ~0;

                PDRead_findU64(inEvents, &address, "address", 0);
                PDRead_findU32(inEvents, &id, "id", 0);

                for (Breakpoint* bp : data->breakpoints)
                {
                    if ((uint64_t)strtol(bp->location.address, 0, 16) == address)
                    {
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

    if (uiFuncs->button("Add Breakpoint", { 0.0f, 0.0f } ))
    {
        Breakpoint* bp = createBreakpoint(data);
        data->breakpoints.push_back(bp);
    }

    uiFuncs->columns(4, "", true);
    //uiFuncs->text(""); uiFuncs->nextColumn();
    uiFuncs->text("Name/Address"); uiFuncs->nextColumn();
    uiFuncs->text("Label"); uiFuncs->nextColumn();
    uiFuncs->text("Condition"); uiFuncs->nextColumn();
    uiFuncs->text(""); uiFuncs->nextColumn();

    for (auto& i : data->breakpoints)
    {
        Breakpoint* bp = i;
        bool needUpdate = false;

        uiFuncs->pushIdPtr(bp);

        //if (uiFuncs->checkbox("Enabled", &bp->enabled))
        //	needUpdate = true;

        if (bp->location.filename)
        {
            uiFuncs->inputText("##filename", bp->location.filename, (int)data->maxPath, 0, 0, 0);
        }
        else
        {
            if (uiFuncs->inputText("##address", bp->location.address, (int)data->maxPath,
                                   PDUIInputTextFlags_CharsHexadecimal | PDUIInputTextFlags_EnterReturnsTrue, 0, 0))
                needUpdate = true;
        }

        uiFuncs->nextColumn();

        uiFuncs->text("");
        uiFuncs->nextColumn();

        uiFuncs->text(""); // no condition for now

        //if (uiFuncs->inputText("##condition", bp->condition, (int)data->maxPath, PDInputTextFlags_EnterReturnsTrue, 0, 0))
        //	needUpdate = true;

        uiFuncs->nextColumn();

        if (needUpdate)
        {
            // TODO: Add support for file/line

            PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
            PDWrite_u64(writer, "address", (uint64_t)strtol(bp->location.address, 0, 16));

            //if (bp->condition[0] != 0)
            //	PDWrite_string(writer, "condition", bp->condition);

            if (bp->id != -1)
                PDWrite_u32(writer, "id", (uint32_t)bp->id);

            PDWrite_eventEnd(writer);

            printf("Sending breakpint\n");
        }

        if (uiFuncs->button("Delete", {0.0f, 0.0f}))
        {
            PDWrite_eventBegin(writer, PDEventType_deleteBreakpoint);
            PDWrite_u32(writer, "id", (uint32_t)bp->id);
            PDWrite_eventEnd(writer);
            bp->markDelete = true;
        }

        uiFuncs->nextColumn();

        uiFuncs->popId();
    }

    // Delete breakpoints that have been marked delete

    for (auto i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i)
    {
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

    PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
    {
        registerPlugin(PD_VIEW_API_VERSION, &plugin, privateData);
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}


