#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Location
{
    char* filename;
    char address[64];
    int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Breakpoint
{
    Location location;
    char* condition;
    bool enabled;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BreakpointsData
{
    std::list<Breakpoint*> breakpoints;
    int addressSize;
};

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
    const char* filename;
    uint32_t line;

    PDRead_findString(reader, &filename, "filename", 0);
    PDRead_findU32(reader, &line, "line", 0);

    if (!filename)
        return;

    for (auto i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i)
    {
        if ((*i)->location.line == (int)line && !strcmp((*i)->location.filename, filename))
        {
            free((*i)->location.filename);
            free((*i)->condition);
            free(*i);
            data->breakpoints.erase(i);
            return;
        }
    }

    Breakpoint* breakpoint = (Breakpoint*)malloc(sizeof(Breakpoint));
    memset(breakpoint, 0, sizeof(Breakpoint));

    breakpoint->location.filename = strdup(filename);
    breakpoint->location.line = (int)line;
    breakpoint->enabled = true;

    updateCondition(breakpoint, reader);

    data->breakpoints.push_back(breakpoint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*

void toggleBreakpointAddress(BreakpointsData* data, PDReader* reader)
{
    uint64_t address;

    if (PDRead_findU64(reader, &address, "address", 0) == PDReadStatus_notFound)
        return;

    for (auto i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i)
    {
        if ((*i)->location.address == address)
        {
            free(*i);
            data->breakpoints.erase(i);
            return;
        }
    }

    Breakpoint* breakpoint = (Breakpoint*)malloc(sizeof(Breakpoint));
    memset(breakpoint, 0, sizeof(Breakpoint));

	breakpoint->location.address = address;

    breakpoint->enabled = true;

    updateCondition(breakpoint, reader);

    data->breakpoints.push_back(breakpoint);
}
*/

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
        }
    }

    uiFuncs->text("");

    uiFuncs->columns(3, "", true);
    uiFuncs->text("Name"); uiFuncs->nextColumn();
    uiFuncs->text("Condition"); uiFuncs->nextColumn();

    for (auto& i : data->breakpoints)
    {
    	Breakpoint* bp = i;

    	//if (bp->location.filename)
		{
        	uiFuncs->text("%s:%d", bp->location.filename, bp->location.line); uiFuncs->nextColumn();
		}
		//else
		//{
	//	}

        //uiFuncs->text(->locationfilename); uiFuncs->nextColumn();
        uiFuncs->text("");
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


