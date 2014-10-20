#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Breakpoint
{
	char* filename;
	int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BreakpointsData
{
	std::list<Breakpoint*> breakpoints;
	int temp;
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

void toogleBreakpoint(BreakpointsData* data, PDReader* reader)
{
    const char* filename;
    uint32_t line;

    PDRead_findString(reader, &filename, "filename", 0);
    PDRead_findU32(reader, &line, "line", 0);

    for (auto i = data->breakpoints.begin(), end = data->breakpoints.end(); i != end; ++i)
	{
		if ((*i)->line == (int)line && !strcmp((*i)->filename, filename))
		{
			free((*i)->filename);
			data->breakpoints.erase(i);
			return;
		}
	}

	Breakpoint* breakpoint = (Breakpoint*)malloc(sizeof(Breakpoint));
	breakpoint->filename = strdup(filename);
	breakpoint->line = (int)line; 

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
                toogleBreakpoint(data, inEvents);
                break;
            }
        }
    }

    uiFuncs->text("");

    uiFuncs->columns(2, "callstack", true);
    uiFuncs->text("File"); uiFuncs->nextColumn();
    uiFuncs->text("Line"); uiFuncs->nextColumn();

    for (auto& i : data->breakpoints)
	{
		uiFuncs->text(i->filename); uiFuncs->nextColumn();
		uiFuncs->text("%d", i->line); uiFuncs->nextColumn();
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


