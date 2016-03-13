#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    ValueSize = 1024,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Register {
    char name[ValueSize];
    char value[ValueSize];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RegistersData {
    int dummy;
    Register* registers;
    int registerCount;
    int maxRegisters;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc) {
    (void)serviceFunc;
    RegistersData* user_data = (RegistersData*)malloc(sizeof(RegistersData));

    user_data->maxRegisters = 256;
    user_data->registers = (Register*)malloc(sizeof(Register) * (size_t)user_data->maxRegisters);
    user_data->registerCount = 0;

    (void)uiFuncs;
    (void)serviceFunc;

    return user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* user_data) {
    free(user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Support floats

static void getRegisterString(char* value, PDReader* reader, PDReaderIterator it) {
    uint64_t regValue = 0;
    const char* regString = 0;

    // Support that backend can write down value in custom format

    if (PDRead_find_string(reader, &regString, "register_string", it) & PDReadStatus_Ok) {
        strncpy(value, regString, ValueSize);
        return;
    }

    uint32_t type = PDRead_find_u64(reader, &regValue, "register", it);

    switch (type & PDReadStatus_TypeMask) {
        case PDReadType_U8:
            sprintf(value, "0x%02x", (uint8_t)regValue); break;
        case PDReadType_S8:
            sprintf(value, "0x%02x", (int8_t)regValue); break;
        case PDReadType_U16:
            sprintf(value, "0x%04x", (uint16_t)regValue); break;
        case PDReadType_S16:
            sprintf(value, "0x%04x", (int16_t)regValue); break;
        case PDReadType_U32:
            sprintf(value, "0x%08x", (uint32_t)regValue); break;
        case PDReadType_S32:
            sprintf(value, "0x%08x", (int32_t)regValue); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addOrUpdate(RegistersData* data, const char* name, const char* value) {
    int count = data->registerCount;

    for (int i = 0; i < count; ++i) {
        if (!strcmp(data->registers[i].name, name)) {
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

static void updateRegisters(RegistersData* data, PDReader* reader) {
    PDReaderIterator it;

    if (PDRead_find_array(reader, &it, "registers", 0) == PDReadStatus_NotFound)
        return;

    while (PDRead_get_next_entry(reader, &it)) {
        const char* name = "";
        char registerValue[ValueSize];

        PDRead_find_string(reader, &name, "name", it);
        getRegisterString(registerValue, reader, it);

        addOrUpdate(data, name, registerValue);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showUI(RegistersData* data, PDUI* uiFuncs) {
    uiFuncs->text("");
    uiFuncs->columns(2, "registers", true);
    uiFuncs->text("Name"); uiFuncs->next_column();
    uiFuncs->text("Value"); uiFuncs->next_column();

    for (int i = 0; i < data->registerCount; ++i) {
        uiFuncs->text(data->registers[i].name); uiFuncs->next_column();
        uiFuncs->text(data->registers[i].value); uiFuncs->next_column();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* user_data, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents) {
    uint32_t event;
    RegistersData* data = (RegistersData*)user_data;

    (void)outEvents;

    // Loop over all the in events

    while ((event = PDRead_get_event(inEvents)) != 0) {
        switch (event) {
            case PDEventType_SetRegisters:
                updateRegisters(data, inEvents); break;
        }
    }

    showUI(data, uiFuncs);

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

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
	registerPlugin(PD_VIEW_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

