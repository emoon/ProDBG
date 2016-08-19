#include "pd_view.h"
#include "pd_backend.h"
#include "pd_host.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <scintilla/include/Scintilla.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SourceCodeData {
    char filename[4096];
    int line;
    bool requestFiles;
    bool hasFiles;
    uint32_t fastOpenKey;
    uint32_t toggleBreakpointKey;
    bool update_after_load;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* readFileFromDisk(const char* file, size_t* size) {
    FILE* f = fopen(file, "rb");
    uint8_t* data = 0;
    size_t s = 0, t = 0;

    *size = 0;

    if (!f)
        return 0;

    // TODO: Use fstat here?

    fseek(f, 0, SEEK_END);
    long ts = ftell(f);

    if (ts < 0)
        goto end;

    s = (size_t)ts;

    data = (uint8_t*)malloc(s + 16);

    if (!data)
        goto end;

    fseek(f, 0, SEEK_SET);

    t = fread(data, s, 1, f);
    (void)t;

    data[s] = 0;

    *size = s;
    end:

    fclose(f);

    return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc) {
    (void)serviceFunc;
    (void)uiFuncs;
    SourceCodeData* user_data = (SourceCodeData*)malloc(sizeof(SourceCodeData));
    memset(user_data, 0, sizeof(SourceCodeData));

	/*
    s_settings = (PDSettingsFuncs*)serviceFunc(PDSETTINGS_GLOBAL);

    user_data->fastOpenKey = s_settings->get_shortcut(s_pluginName, "fast_open");
    user_data->toggleBreakpointKey = s_settings->get_shortcut(s_pluginName, "toggle_breakpoint");

    printf("fastOpenKey 0x%x\n", user_data->fastOpenKey);
    printf("toggleBreakpointKey  0x%x\n", user_data->toggleBreakpointKey);

	*/
    user_data->requestFiles = false;
    user_data->hasFiles = false;

    return user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* user_data) {
    free(user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void update_file_data(PDUI* uiFuncs, PDUISCInterface* sourceFuncs, SourceCodeData* data) {
	size_t size = 0;
	void* fileData = readFileFromDisk(data->filename, &size);

	printf("reading file to buffer %s\n", data->filename);

	PDUI_set_title(uiFuncs, data->filename);

	if (fileData) {
		PDUI_sc_send_command(sourceFuncs, SCI_CLEARALL, 0, 0);
		PDUI_sc_send_command(sourceFuncs, SCI_ADDTEXT, size, (intptr_t)fileData);
	}else {
		printf("Sourcecode_plugin: Unable to load %s\n", data->filename);
	}

	free(fileData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setSourceCodeFile(PDUI* uiFuncs, PDUISCInterface* sourceFuncs, SourceCodeData* data, const char* filename, uint32_t line) {
    if (strcmp(filename, data->filename)) {
        strncpy(data->filename, filename, sizeof(data->filename));
        data->filename[sizeof(data->filename) - 1] = 0;
		update_file_data(uiFuncs, sourceFuncs, data);
    }

    PDUI_sc_send_command(sourceFuncs, SCI_GOTOLINE, (uintptr_t)line, 0);

    data->line = (int)line;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExceptionLocation(PDUI* uiFuncs, PDUISCInterface* sourceFuncs, SourceCodeData* data, PDReader* inEvents) {
    const char* filename;
    uint32_t line;

    // TODO: How to show this? Tell user to switch to disassembly view?

    if (PDRead_find_string(inEvents, &filename, "filename", 0) == PDReadStatus_NotFound)
        return;

    if (PDRead_find_u32(inEvents, &line, "line", 0) == PDReadStatus_NotFound)
        return;

    setSourceCodeFile(uiFuncs, sourceFuncs, data, filename, line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateKeyboard(SourceCodeData* data, PDUISCInterface* sourceFuncs, PDUI* uiFuncs) {
    (void)data;
    (void)uiFuncs;
    (void)sourceFuncs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void toggleBreakpointCurrentLine(PDUISCInterface* sourceFuncs, SourceCodeData* data, PDWriter* writer) {
    (void)data;
    (void)writer;

    PDUI_sc_send_command(sourceFuncs, SCN_TOGGLE_BREAKPOINT, 0, 0);

    uint32_t currentLine = (uint32_t)PDUI_sc_send_command(sourceFuncs, SCN_GETCURRENT_LINE, 0, 0);

    printf("currentLine %d\n", currentLine);

    // TODO: Currenty we don't handly if we set breakpoints on a line we can't

    PDWrite_event_begin(writer, PDEventType_SetBreakpoint);
    PDWrite_string(writer, "filename", data->filename);
    PDWrite_u32(writer, "line", currentLine);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* user_data, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer) {
    uint32_t event;

    SourceCodeData* data = (SourceCodeData*)user_data;
    PDUISCInterface* sourceFuncs = uiFuncs->sc_input_text("test", 800, 700);

    if (data->update_after_load) {
		update_file_data(uiFuncs, sourceFuncs, data);
		data->update_after_load = false;
	}

    while ((event = PDRead_get_event(inEvents)) != 0) {
        switch (event) {
            case PDEventType_SetExceptionLocation:
            {
                setExceptionLocation(uiFuncs, sourceFuncs, data, inEvents);
                data->requestFiles = true;
                break;
            }

            case PDEventType_SetSourceCodeFile:
            {
                const char* filename;

                if (PDRead_find_string(inEvents, &filename, "filename", 0) == PDReadStatus_NotFound)
                    break;

                setSourceCodeFile(uiFuncs, sourceFuncs, data, filename, 0);

                break;
            }

            case PDEventType_ToggleBreakpointCurrentLine:
            {
                toggleBreakpointCurrentLine(sourceFuncs, data, writer);
                break;
            }

            case PDEventType_SetSourceFiles:
            {
                // TODO: Store the files

                data->hasFiles = true;
                break;
            }
        }
    }

    updateKeyboard(data, sourceFuncs, uiFuncs);

    PDUI_sc_update(sourceFuncs);
    PDUI_sc_draw(sourceFuncs);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int save_state(void* user_data, struct PDSaveState* save_state) {
    SourceCodeData* data = (SourceCodeData*)user_data;
    printf("writing string to file %s\n", data->filename);
	PDIO_write_string(save_state, data->filename);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int load_state(void* user_data, struct PDLoadState* load_state) {
    SourceCodeData* data = (SourceCodeData*)user_data;
	int status = PDIO_read_string(load_state, (char*)&data->filename, sizeof(data->filename));
	data->update_after_load = true;
    data->requestFiles = false;

	printf("load state %s (%d)\n", data->filename, status);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Source Code View",
    createInstance,
    destroyInstance,
    update,
    save_state,
    load_state,
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

