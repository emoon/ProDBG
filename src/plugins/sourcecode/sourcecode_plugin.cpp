#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <scintilla/include/Scintilla.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SourceCodeData
{
    char filename[4096];
    int line;
    bool requestFiles;
    bool hasFiles;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* readFileFromDisk(const char* file, size_t* size)
{
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

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;
    SourceCodeData* userData = (SourceCodeData*)malloc(sizeof(SourceCodeData));
    memset(userData, 0, sizeof(SourceCodeData));

    userData->requestFiles = false;
    userData->hasFiles = false;

    // TODO: Temp testing code

    //parseFile(&userData->file, "examples/crashing_native/crash2.c");

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExceptionLocation(PDUI* uiFuncs, PDSCInterface* sourceFuncs, SourceCodeData* data, PDReader* inEvents)
{
    const char* filename;
    uint32_t line;

    // TODO: How to show this? Tell user to switch to disassembly view?

    if (PDRead_findString(inEvents, &filename, "filename", 0) == PDReadStatus_notFound)
        return;

    if (PDRead_findU32(inEvents, &line, "line", 0) == PDReadStatus_notFound)
        return;

    if (strcmp(filename, data->filename))
    {
        size_t size = 0;
        void* fileData = readFileFromDisk(filename, &size);

        printf("reading file to buffer %s\n", filename);

        (void)uiFuncs;

        PDUI_setTitle(uiFuncs, filename);

        if (fileData)
		{
            PDUI_SCSendCommand(sourceFuncs, SCI_CLEARALL, 0, 0);
            PDUI_SCSendCommand(sourceFuncs, SCI_ADDTEXT, size, (intptr_t)fileData);
		}
        else
		{
            printf("Sourcecode_plugin: Unable to load %s\n", filename);
		}

        free(fileData);

        strncpy(data->filename, filename, sizeof(data->filename));
        data->filename[sizeof(data->filename) - 1] = 0;
    }

    PDUI_SCSendCommand(sourceFuncs, SCI_GOTOLINE, (uintptr_t)line, 0);

    data->line = (int)line;

    //if (strcmp(filename, data->filename))
    // PDUI_setTitle(uiFuncs, filename); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateKeyboard(SourceCodeData* data, PDUI* uiFuncs)
{
    (void)data;
    (void)uiFuncs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void toggleBreakpointCurrentLine(SourceCodeData* data, PDWriter* writer)
{
    (void)data;
    (void)writer;
    /*
       // TODO: Currenty we don't handly if we set breakpoints on a line we can't

       PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
       PDWrite_string(writer, "filename", data->file.filename);
       PDWrite_u32(writer, "line", (unsigned int)data->cursorPos + 1);
       PDWrite_eventEnd(writer);

       data->file.lines[data->cursorPos].breakpoint = !data->file.lines[data->cursorPos].breakpoint;
     */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer)
{
    uint32_t event;

    (void)uiFuncs;

    SourceCodeData* data = (SourceCodeData*)userData;
    PDSCInterface* sourceFuncs = uiFuncs->scInputText("test", 800, 700, 0, 0);

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setExceptionLocation:
            {
                setExceptionLocation(uiFuncs, sourceFuncs, data, inEvents);
                data->requestFiles = true;
                break;
            }

            case PDEventType_toggleBreakpointCurrentLine:
            {
                toggleBreakpointCurrentLine(data, writer);
                break;
            }

			case PDEventType_setSourceFiles:
			{
				// TODO: Store the files

                data->hasFiles = true;
				break;
			}
        }
    }

    updateKeyboard(data, uiFuncs);

    PDUI_SCUpdate(sourceFuncs);
    PDUI_SCDraw(sourceFuncs);

    //showInUI(data, uiFuncs);

    PDWrite_eventBegin(writer, PDEventType_getExceptionLocation);
    PDWrite_eventEnd(writer);

    if (!data->hasFiles && data->requestFiles)
	{
		printf("requesting files\n");
		PDWrite_eventBegin(writer, PDEventType_getSourceFiles);
		PDWrite_eventEnd(writer);
	}

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Source Code View",
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

