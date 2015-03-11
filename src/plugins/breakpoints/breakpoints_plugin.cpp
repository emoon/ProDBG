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
    char* address;
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
	BreakpointsData() : maxPath(8192) {}

    std::list<Breakpoint*> breakpoints;
    int addressSize;
    size_t maxPath;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Breakpoint* createBreakpoint()
{
	Breakpoint* bp = (Breakpoint*)malloc(sizeof(Breakpoint));
	memset(bp, 0, sizeof(Breakpoint));
    bp->enabled = true;

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
    const char* filename;
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

    Breakpoint* breakpoint = createBreakpoint(); 

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

    Breakpoint* breakpoint = createBreakpoint(); 
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
        }
    }

    uiFuncs->text("");

    uiFuncs->columns(3, "", true);
    uiFuncs->text("Name"); uiFuncs->nextColumn();
    uiFuncs->text("Condition"); uiFuncs->nextColumn();

    for (auto& i : data->breakpoints)
    {
    	Breakpoint* bp = i;

    	if (bp->location.filename)
		{
        	uiFuncs->inputText("", bp->location.filename, (int)data->maxPath, 0, 0, 0);
		}
		else
		{
        	uiFuncs->inputText("", bp->location.address, (int)data->maxPath, 0, 0, 0);
		}

		uiFuncs->nextColumn();

        uiFuncs->text(bp->condition); uiFuncs->nextColumn();
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


