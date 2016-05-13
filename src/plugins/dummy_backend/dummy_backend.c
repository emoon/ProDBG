#include "pd_backend.h"
#include "pd_host.h"
#include "pd_menu.h"
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

static void on_menu(PDReader* reader) {
    uint32_t menuId;

    PDRead_find_u32(reader, &menuId, "menu_id", 0);

    switch (menuId) {
        case 1:
        {
        	printf("id 1 pressed!\n");
            break;
        }

        case 2:
        {
        	printf("id 2 pressed!\n");
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data,
						   PDAction action,
						   PDReader* reader,
						   PDWriter* writer) {
    uint32_t event;

	(void)user_data;
    (void)action;
    (void)reader;
    (void)writer;

    while ((event = PDRead_get_event(reader))) {
        switch (event) {
            case PDEventType_MenuEvent: 
			{
                on_menu(reader);
                break;
            }
		}
	}

	send_6502_registers(writer);

    // printf("Update backend\n");

    return PDDebugState_NoTarget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenuHandle register_menu(void* user_data, PDMenuFuncs* menu_funcs) {
	(void)user_data;

	PDMenuHandle menu = PDMenu_create_menu(menu_funcs, "Dummy Backend Menu");

	PDMenu_add_menu_item(menu_funcs, menu, "Id 1", 1, 0, 0);
	PDMenu_add_menu_item(menu_funcs, menu, "Id 2", 2, 0, 0);

	return menu;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin =
{
    "Dummy Backend",
    create_instance,
    destroy_instance,
	register_menu,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

