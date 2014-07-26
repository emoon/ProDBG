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

	userData->registerList = PDUIListView_create(uiFuncs, "Test", 0);

	PDUIListView_itemAdd(uiFuncs, userData->registerList, "Item 0");
	PDUIListView_itemAdd(uiFuncs, userData->registerList, "Item 1");
	PDUIListView_itemAdd(uiFuncs, userData->registerList, "Item 2");

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
    "Registers",
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

