#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    ValueSize = 1024,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Register
{
    char name[ValueSize];
    char value[ValueSize];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RegistersData
{
    int dummy;
    Register* registers;
    int registerCount;
    int maxRegisters;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    RegistersData* userData = (RegistersData*)malloc(sizeof(RegistersData));

    userData->maxRegisters = 256;
    userData->registers = (Register*)malloc(sizeof(Register) * (size_t)userData->maxRegisters);
    userData->registerCount = 0;

    (void)uiFuncs;
    (void)serviceFunc;

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Support floats

static void getRegisterString(char* value, PDReader* reader, PDReaderIterator it)
{
    uint64_t regValue = 0;
    const char* regString = 0;

    // Support that backend can write down value in custom format

    if (PDRead_findString(reader, &regString, "register_string", it) & PDReadStatus_ok)
    {
        strncpy(value, regString, ValueSize);
        return;
    }

    uint32_t type = PDRead_findU64(reader, &regValue, "register", it);

    switch (type & PDReadStatus_typeMask)
    {
        case PDReadType_u8:
            sprintf(value, "0x%02x", (uint8_t)regValue); break;
        case PDReadType_s8:
            sprintf(value, "0x%02x", (int8_t)regValue); break;
        case PDReadType_u16:
            sprintf(value, "0x%04x", (uint16_t)regValue); break;
        case PDReadType_s16:
            sprintf(value, "0x%04x", (int16_t)regValue); break;
        case PDReadType_u32:
            sprintf(value, "0x%08x", (uint32_t)regValue); break;
        case PDReadType_s32:
            sprintf(value, "0x%08x", (int32_t)regValue); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addOrUpdate(RegistersData* data, const char* name, const char* value)
{
    int count = data->registerCount;

    for (int i = 0; i < count; ++i)
    {
        if (!strcmp(data->registers[i].name, name))
        {
            strncpy(data->registers[i].value, value, ValueSize);
            data->registers[i].value[ValueSize - 1] = 0;

            return;
        }
    }

    strncpy(data->registers[count].name, name, ValueSize);
    strncpy(data->registers[count].value, value, ValueSize);

    data->registers[count].name[ValueSize - 1] = 0;
    data->registers[count].value[ValueSize - 1] = 0;

    data->registerCount++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateRegisters(RegistersData* data, PDReader* reader)
{
    PDReaderIterator it;

    if (PDRead_findArray(reader, &it, "registers", 0) == PDReadStatus_notFound)
        return;

    while (PDRead_getNextEntry(reader, &it))
    {
        const char* name = "";
        char registerValue[ValueSize];

        PDRead_findString(reader, &name, "name", it);
        getRegisterString(registerValue, reader, it);

        addOrUpdate(data, name, registerValue);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showUI(RegistersData* data, PDUI* uiFuncs)
{
    uiFuncs->text("");
    uiFuncs->columns(2, "registers", true);
    uiFuncs->text("Name"); uiFuncs->nextColumn();
    uiFuncs->text("Value"); uiFuncs->nextColumn();

    for (int i = 0; i < data->registerCount; ++i)
    {
        uiFuncs->text(data->registers[i].name); uiFuncs->nextColumn();
        uiFuncs->text(data->registers[i].value); uiFuncs->nextColumn();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
    uint32_t event;
    RegistersData* data = (RegistersData*)userData;

    (void)outEvents;

    // Loop over all the in events

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setRegisters:
                updateRegisters(data, inEvents); break;
        }
    }

    showUI(data, uiFuncs);

    //PDWrite_eventBegin(outEvents, PDEventType_getRegisters);
    //PDWrite_eventEnd(outEvents);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Registers View",
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

