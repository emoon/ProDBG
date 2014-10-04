#include <pd_view.h>
#include <pd_backend.h>
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
    RegistersData* userData = (RegistersData*)malloc(sizeof(RegistersData));

    (void)uiFuncs;
    (void)serviceFunc;

    // static const char* headers[] = { "Register", "Value", 0 };

    // userData->registerList = PDUIListView_create(uiFuncs, headers, 0);
    //PDUIListView_itemAdd(uiFuncs, userData->registerList, meh);

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Support floats
/*
   static void getRegisterString(char* value, PDReader* reader, PDReaderIterator it)
   {
    uint64_t regValue;
    uint32_t type = PDRead_findU64(reader, &regValue, "register", it);

    switch (type & PDReadStatus_typeMask)
    {
        case PDReadType_u8: sprintf(value, "0x%02x", (uint8_t)regValue); break;
        case PDReadType_s8: sprintf(value, "0x%02x", (int8_t)regValue); break;
        case PDReadType_u16: sprintf(value, "0x%04x", (uint16_t)regValue); break;
        case PDReadType_s16: sprintf(value, "0x%04x", (int16_t)regValue); break;
        case PDReadType_u32: sprintf(value, "0x%08x", (uint32_t)regValue); break;
        case PDReadType_s32: sprintf(value, "0x%08x", (int32_t)regValue); break;
    }
   }
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(RegistersData* data, PDReader* reader, PDUI* uiFuncs)
{
    PDReaderIterator it;

    if (PDRead_findArray(reader, &it, "registers", 0) == PDReadStatus_notFound)
        return;

    (void)data;
    (void)uiFuncs;

    /*
       PDUIListView_clear(uiFuncs, data->registerList);

       while (PDRead_getNextEntry(reader, &it))
       {
        uint64_t regValue;
        const char* name = "";
        char registerValue[128];

        PDRead_findString(reader, &name, "name", it);
        getRegisterString(registerValue, reader, it);

        const char* values[] = { name, registerValue, 0 };

        PDUIListView_itemAdd(uiFuncs, data->registerList, values);
       }
     */
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
    uint32_t event;
    RegistersData* data = (RegistersData*)userData;

    // Loop over all the in events

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setRegisters:
                showInUI(data, inEvents, uiFuncs); break;
        }
    }


    //(void)userData;
    //(void)inEvents;
    (void)outEvents;
    //(void)uiFuncs;

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

