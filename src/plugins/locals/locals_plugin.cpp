#include <pd_view.h>
#include <stdlib.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RegistersData
{
    int dummy;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;
    RegistersData* userData = (RegistersData*)malloc(sizeof(RegistersData));

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

	uiFuncs->text("");

	uiFuncs->columns(4, "data", true);
	uiFuncs->text("ID"); uiFuncs->nextColumn();
	uiFuncs->text("Name"); uiFuncs->nextColumn();
	uiFuncs->text("Path"); uiFuncs->nextColumn();
	uiFuncs->text("Flags"); uiFuncs->nextColumn();

	uiFuncs->text("0000"); uiFuncs->nextColumn();
	uiFuncs->text("Robert"); uiFuncs->nextColumn();
	uiFuncs->text("/path/robert"); uiFuncs->nextColumn();
	uiFuncs->text("...."); uiFuncs->nextColumn();

	uiFuncs->text("0001"); uiFuncs->nextColumn();
	uiFuncs->text("Stephanie"); uiFuncs->nextColumn();
	uiFuncs->text("/path/stephanie"); uiFuncs->nextColumn();
	uiFuncs->text("...."); uiFuncs->nextColumn();

	uiFuncs->text("0002"); uiFuncs->nextColumn();
	uiFuncs->text("C64"); uiFuncs->nextColumn();
	uiFuncs->text("/path/computer"); uiFuncs->nextColumn();
	uiFuncs->text("...."); uiFuncs->nextColumn();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Locals",
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

}

