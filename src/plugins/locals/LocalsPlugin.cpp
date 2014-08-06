#include <PDView.h>
#include <stdlib.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RegistersData
{
	PDUIListView registerList;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
	(void)serviceFunc;
	RegistersData* userData = (RegistersData*)malloc(sizeof(RegistersData));
	
	static const char* foo[] = { "Name", "Value", 0 };
	static const char* meh[] = { "Foobar", "0", 0 };

	userData->registerList = PDUIListView_create(uiFuncs, foo, 0);

	PDUIListView_itemAdd(uiFuncs, userData->registerList, meh);

	return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
	free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
	(void)userData;
	(void)inEvents;
	(void)outEvents;
	(void)uiFuncs;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    0,    // version
    "Locals",
    createInstance,
    destroyInstance,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

PD_EXPORT void InitPlugin(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin)
{
	(void)version;
	(void)serviceFunc;
    registerPlugin(PD_VIEW_API_VERSION, &plugin);
}

}

