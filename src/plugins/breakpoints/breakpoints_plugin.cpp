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
	bool markDelete;
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

static Breakpoint* createBreakpoint(BreakpointsData* userData)
{
	Breakpoint* bp = (Breakpoint*)malloc(sizeof(Breakpoint));
	memset(bp, 0, sizeof(Breakpoint));

	bp->location.address = (char*)malloc(userData->maxPath);
	bp->condition = (char*)malloc(userData->maxPath);

	memset(bp->location.address, 0, userData->maxPath);
	memset(bp->condition, 0, userData->maxPath);

	bp->enabled = false;

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
        }
    }

    uiFuncs->text("");

	if (uiFuncs->button("Add Breakpoint"))
	{
		Breakpoint* bp = createBreakpoint(data);
		data->breakpoints.push_back(bp);
	}

    uiFuncs->columns(5, "", true);
    uiFuncs->text(""); uiFuncs->nextColumn();
    uiFuncs->text("Name/Address"); uiFuncs->nextColumn();
    uiFuncs->text("Label"); uiFuncs->nextColumn();
    uiFuncs->text("Condition"); uiFuncs->nextColumn();
    uiFuncs->text(""); uiFuncs->nextColumn();

    for (auto& i : data->breakpoints)
    {
    	Breakpoint* bp = i;

		uiFuncs->pushIdPtr(bp);
		
		uiFuncs->checkbox("Enabled", &bp->enabled); uiFuncs->nextColumn();

    	if (bp->location.filename)
        	uiFuncs->inputText("##filename", bp->location.filename, (int)data->maxPath, 0, 0, 0);
		else
        	uiFuncs->inputText("##address", bp->location.address, (int)data->maxPath, PDInputTextFlags_CharsHexadecimal, 0, 0);

		uiFuncs->nextColumn();

		uiFuncs->text("");
		uiFuncs->nextColumn();

		uiFuncs->inputText("##condition", bp->condition, (int)data->maxPath, 0, 0, 0);
		uiFuncs->nextColumn();

		if (uiFuncs->button("Delete"))
			bp->markDelete = true;

		uiFuncs->nextColumn();

		uiFuncs->popId();
    }
	
	// Delete breakpoints that have been marked delete

	for (auto& i = data->breakpoints.begin(); i != data->breakpoints.end(); ++i)
	{
		if ((*i)->markDelete)
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


