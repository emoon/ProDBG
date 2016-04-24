#include "pd_backend.h"
#include "pd_host.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DummyPlugin {
	int dummy;

} DummyPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* create_instance(ServiceFunc* serviceFunc) {
	(void)serviceFunc;

	DummyPlugin* plugin = (DummyPlugin*)malloc(sizeof(DummyPlugin));
	memset(plugin, 0, sizeof(DummyPlugin));

    return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroy_instance(void* user_data) {
	free(user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void write_register(PDWriter* writer, const char* name, uint8_t size, uint16_t reg, uint8_t read_only) {
    PDWrite_array_entry_begin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "size", size);

    if (read_only) {
        PDWrite_u8(writer, "read_only", 1);
	}

    if (size == 2) {
        PDWrite_u16(writer, "register", reg);
	} else {
        PDWrite_u8(writer, "register", (uint8_t)reg);
	}

    PDWrite_entry_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void send_6502_registers(PDWriter* writer) {
	PDWrite_event_begin(writer, PDEventType_SetRegisters);
	PDWrite_array_begin(writer, "registers");

	write_register(writer, "pc", 2, 0x4444, 1);
	write_register(writer, "sp", 1, 1, 0);
	write_register(writer, "a", 1, 2, 0);
	write_register(writer, "x", 1, 3, 0);
	write_register(writer, "y", 1, 4, 0);

	PDWrite_array_end(writer);
	PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data,
						   PDAction action,
						   PDReader* reader,
						   PDWriter* writer) {
	(void)user_data;
    (void)action;
    (void)reader;
    (void)writer;

	send_6502_registers(writer);

    // printf("Update backend\n");

    return PDDebugState_NoTarget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin =
{
    "Dummy Backend",
    create_instance,
    destroy_instance,
    0,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

